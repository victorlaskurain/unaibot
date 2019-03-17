#include <vla/registers.hpp>
#include <vla/serial.hpp>
#include <vla/timers.hpp>
#include <vla/rt_clock.hpp>
#include <vla/protothreads.hpp>
#include <util/delay.h>
#include <util/crc16.h>

using namespace vla;

struct timeout_msg_t
{
};

using input_msg_t = uint8_t;

struct buffer_msg_t
{
    uint8_t *data;
    uint8_t size;
};

// implements a poor man's variant in order to express different
// message types in a safe and convenient way.
class transmission_msg_t
{
    enum class transmission_msg_tag
    {
        NONE,
        TIMEOUT,
        INPUT,
        BUFFER
    };
    transmission_msg_tag tag = transmission_msg_tag::NONE;
    union {
        timeout_msg_t timeout;
        input_msg_t   input;
        buffer_msg_t  buffer;
    } msg;
public:
    transmission_msg_t()                    = default;
    transmission_msg_t(transmission_msg_t&) = default;
    template<typename T>
    transmission_msg_t(T&& v)
    {
        put(v);
    }
    bool get(timeout_msg_t &v)
    {
        if (transmission_msg_tag::TIMEOUT == tag) {
            v = msg.timeout;
            return true;
        }
        return false;
    }
    bool get(input_msg_t &v)
    {
        if (transmission_msg_tag::INPUT == tag) {
            v = msg.input;
            return true;
        }
        return false;
    }
    bool get(buffer_msg_t &v)
    {
        if (transmission_msg_tag::BUFFER == tag) {
            v = msg.buffer;
            return true;
        }
        return false;
    }
    void put(timeout_msg_t v)
    {
        msg.timeout = v;
        tag = transmission_msg_tag::TIMEOUT;
    }
    void put(input_msg_t v)
    {
        msg.input = v;
        tag = transmission_msg_tag::INPUT;
    }
    void put(buffer_msg_t v)
    {
        msg.buffer = v;
        tag = transmission_msg_tag::BUFFER;
    }
};

struct transmission_queue_t: public basic_msg_queue<transmission_msg_t, 50, Cli, NopLock>
{
    using parent_t = basic_msg_queue<transmission_msg_t, 50, Cli, NopLock>;
    template<typename Msg>
    bool push(Msg &&m)
    {
        return parent_t::push(m);
    }
    template<typename Msg>
    bool pop(Msg &&msg)
    {
        parent_t::ReadLock lock;
        if (!_empty() && _peek().get(msg)) {
            parent_t::_read_ptr_next();
            return true;
        }
        return false;
    }
    void ignore()
    {
        parent_t::_read_ptr_next();
    }
};

using pdu_handler_queue_t = msg_queue_fast<buffer_msg_t, 1>;

static constexpr uint16_t PDU_MAX = 256;

class transmission : public ptxx_thread
{
    // implement a version of the async serial reader that uses the
    // queue of this thread instead of a buffer
    class serial_read_async_into_queue
    {
        static void char_ready_cb(void *q, uint8_t c)
        {
            static_cast<transmission_queue_t*>(q)->push(c);
        }
    protected:
        // to be call from the constructor of serial
        void set_queue(transmission_queue_t *q)
        {
            vla::read_buffer = q;
        }
    public:
        serial_read_async_into_queue()
        {
            vla::read_buffer        = nullptr;
            vla::on_serial_received = char_ready_cb;
            UCSR0B |= (1<<RXEN0)|(1<<RXCIE0);
        }
        ~serial_read_async_into_queue()
        {
            vla::read_buffer        = nullptr;
            vla::on_serial_received = nullptr;
            UCSR0B &= ~((1<<RXEN0)|(1<<RXCIE0));
        }
    };
    using serial_base = vla::serial<serial_read_async_into_queue, vla::serial_async_write>;
    class serial : public serial_base
    {
    public:
        serial(transmission_queue_t &q, serial_speed speed):serial_base{speed, serial_parity::EVEN}
        {
            set_queue(&q);
        }
    };

    bool must_transmit(const buffer_msg_t msg) const
    {
        return true; // :TODO:
    }

    // helper member functions and callback function to set, handle and cancel alarms
    static void enqueue_timeout(void* q)
    {
        static_cast<transmission_queue_t*>(q)->push(timeout_msg_t{});
    }
    alarm_id set_alarm(period_us us)
    {
        return c.set_alarm(us, enqueue_timeout, &q);
    }
    void cancel_alarm(alarm_id id)
    {
        c.cancel_alarm(id);
    }

    rt_clock &c;
    transmission_queue_t &q;
    pdu_handler_queue_t &out_q;
    serial ser;
    alarm_id aid;
    bool inter_frame_delay_detected = false;
    bool inter_char_delay_detected = false;
    uint8_t buffer[PDU_MAX];
    uint8_t buffer_i = 0;
    const period_us inter_frame_delay = period_us{1750};
    const period_us inter_char_delay  = period_us{ 750};
    enum class state_t : uint8_t {
        INITIAL,
        READY,
        EMISSION,
        RECEPTION,
        PROCESSING,
        UNKNOWN
    } state = state_t::INITIAL;
    const char* state_name(state_t s)
    {
        if (state_t::INITIAL    == s) return "INITIAL";
        if (state_t::READY      == s) return "READY";
        if (state_t::EMISSION   == s) return "EMISSION";
        if (state_t::RECEPTION  == s) return "RECEPTION";
        if (state_t::PROCESSING == s) return "PROCESSING";
        return "XXX";
    };
    buffer_msg_t buffer_msg;
    uint8_t buffer_msg_i;
public:
    transmission(
        rt_clock &c,
        transmission_queue_t &q,
        pdu_handler_queue_t &out_q,
        serial_speed speed = serial_speed::BAUD_9600):c(c), q(q), out_q(out_q), ser(q, speed) {
    }
    void operator()() {
        input_msg_t   input_msg{};
        timeout_msg_t timeout_msg;
        uint16_t      bytes_written;
        /*
          // uncomment to debug state transitions
        static auto last_state = state_t::UNKNOWN;
        if (last_state != state) {
            write_line(ser, state_name(state));
            last_state = state;
        }
        */
        ptxx_begin();
        while (true) {
            if (state_t::INITIAL == state) {
                aid = set_alarm(inter_frame_delay);
                ptxx_wait(!q.empty());
                if (q.pop(input_msg)) {
                    cancel_alarm(aid);
                    set_alarm(inter_frame_delay);
                } else if (q.pop(timeout_msg)) {
                    state = state_t::READY;
                } else {
                    q.ignore();
                    state = state_t::INITIAL;
                }
            }
            if (state_t::READY == state) {
                PORTB5_t::clear();
                ptxx_wait(!q.empty());
                if (q.pop(input_msg)) {
                    set_alarm(inter_frame_delay);
                    buffer[buffer_i++] = input_msg;
                    PORTB5_t::set();
                    state = state_t::RECEPTION;
                } else if (q.pop(buffer_msg)) {
                    if (must_transmit(buffer_msg)) {
                        state = state_t::EMISSION;
                    }
                } else {
                    q.ignore();
                    state = state_t::INITIAL;
                }
            }
            if (state_t::RECEPTION == state) {
                ptxx_wait(!q.empty());
                if (q.pop(input_msg)) {
                    cancel_alarm(aid);
                     // discard the timeout message that might have
                     // been enqueued during pop and cancellation
                    while (q.pop(timeout_msg));
                    set_alarm(inter_frame_delay);
                    buffer[buffer_i++] = input_msg;
                } else if (q.pop(timeout_msg)) { // inter_frame_delay detected
                    buffer_msg_t m = {buffer, buffer_i};
                    out_q.push(m);
                    buffer_i = 0;
                    set_alarm(inter_frame_delay - inter_char_delay);
                    state = state_t::PROCESSING;
                } else {
                    q.ignore();
                    state = state_t::INITIAL;
                }
            }
            if (state_t::PROCESSING == state) {
                // we either get the buffer back and the timeout
                // expires or the other way around. Anything else is
                // an error.
                ptxx_wait(!q.empty());
                if (q.pop(timeout_msg)) {
                    ptxx_wait(!q.empty());
                    if (q.pop(buffer_msg)) {
                        q.push(buffer_msg); // enqueue again, just wait for the timeout
                        state = state_t::READY;
                    } else {
                        q.ignore();
                        state = state_t::INITIAL;
                    }
                }
                if (q.pop(buffer_msg)) {
                    ptxx_wait(!q.empty());
                    if (q.pop(timeout_msg)) {
                        q.push(buffer_msg); // enqueue again, just wait for the timeout
                        state = state_t::READY;
                    } else {
                        q.ignore();
                        state = state_t::INITIAL;
                    }
                }
            }
            if (state_t::EMISSION == state) {
                buffer_msg_i = 0;
                while (buffer_msg_i < buffer_msg.size) {
                    bytes_written = ser.write_some(
                        buffer_msg.data + buffer_msg_i,
                        buffer_msg.size - buffer_msg_i);
                    buffer_msg_i += bytes_written;
                    ptxx_yield();
                }
                set_alarm(inter_frame_delay);
                ptxx_wait(!q.empty());
                if (q.pop(timeout_msg)) {
                    state = state_t::READY;
                } else {
                    q.ignore();
                    state = state_t::INITIAL;
                }
            }
        }
        ptxx_end();
    }
};

class pdu_handler : public ptxx_thread
{
    pdu_handler_queue_t &in_q;
    transmission_queue_t &out_q;
    buffer_msg_t msg = {};
public:
    pdu_handler(
        pdu_handler_queue_t &in_q,
        transmission_queue_t &out_q):in_q(in_q), out_q(out_q) {}
    void operator()() {
        ptxx_begin();
        while (true) {
            ptxx_wait(in_q.pop(msg));
            msg.data[0] |= 0x80;
            msg.data[1]  = 0x02;
            uint16_t crc = 0xffff;
            crc = _crc16_update(crc, msg.data[0]);
            crc = _crc16_update(crc, msg.data[1]);
            msg.data[2] = crc;
            msg.data[3] = crc>>8;
            msg.size = 4;
            out_q.push(msg);
        }
        ptxx_end();
    }
};

int main(int argc, char **argv)
{
    PORTB5_t::set_mode_output();
    PORTB5_t::set();
    _delay_ms(1000);
    PORTB5_t::clear();
    _delay_ms(1000);
    tick_0a_20khz tick;
    rt_clock clock{tick};
    transmission_queue_t tr_q;
    pdu_handler_queue_t  pdu_q;
    transmission tr{clock, tr_q, pdu_q};
    pdu_handler pduh{pdu_q, tr_q};
    while(true) {
        tr();
        pduh();
    }
}
