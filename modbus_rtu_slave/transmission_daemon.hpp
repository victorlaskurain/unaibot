#ifndef VLA_MODBUS_RTU_SLAVE_TRANSMISSION_DAEMON
#define VLA_MODBUS_RTU_SLAVE_TRANSMISSION_DAEMON

#include "daemon_queues.hpp"
#include <vla/msg_queue.hpp>
#include <vla/protothreads.hpp>
#include <vla/serial.hpp>
#include <vla/rt_clock.hpp>

namespace vla {

static constexpr uint16_t PDU_MAX = 256;

class transmission : public ptxx_thread
{
    // implement a version of the async serial reader that uses the
    // queue of this thread instead of a buffer
    class serial_read_async_into_queue
    {
        static void char_ready_cb(void *q, uint8_t c);
    protected:
        // to be call from the constructor of serial
        void set_queue(transmission_queue_t *q);
    public:
        serial_read_async_into_queue();
        ~serial_read_async_into_queue();
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

    // helper member functions and callback function to set, handle and cancel alarms
    static void enqueue_timeout(void* q);
    alarm_id set_alarm(period_us us);
    void cancel_alarm(alarm_id id);

    enum class state_t : uint8_t {
        INITIAL,
        READY,
        EMISSION,
        RECEPTION,
        PROCESSING,
        UNKNOWN
    };
    const char* state_name(state_t s)
    {
        if (state_t::INITIAL    == s) return "INITIAL";
        if (state_t::READY      == s) return "READY";
        if (state_t::EMISSION   == s) return "EMISSION";
        if (state_t::RECEPTION  == s) return "RECEPTION";
        if (state_t::PROCESSING == s) return "PROCESSING";
        return "XXX";
    };

    rt_clock &c;
    transmission_queue_t &q;
    pdu_handler_queue_t &out_q;
    serial ser;

    // state for the operator()()
    state_t state = state_t::INITIAL;
    alarm_id aid;
    uint8_t buffer[PDU_MAX];
    uint8_t buffer_i = 0;
    buffer_msg_t buffer_msg;
    uint8_t buffer_msg_i;
public:
    transmission(
        rt_clock &c,
        transmission_queue_t &q,
        pdu_handler_queue_t &out_q,
        serial_speed speed = serial_speed::BAUD_19200):c(c), q(q), out_q(out_q), ser(q, speed) {
    }
    void operator()();
};

}

#endif // VLA_MODBUS_RTU_SLAVE_TRANSMISSION_DAEMON
