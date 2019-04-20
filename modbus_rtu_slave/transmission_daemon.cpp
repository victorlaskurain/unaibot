#include "transmission_daemon.hpp"

static const vla::period_us inter_frame_delay = vla::period_us{1750};
static const vla::period_us inter_char_delay  = vla::period_us{ 750};

void vla::transmission::serial_read_async_into_queue::char_ready_cb(void *q, uint8_t c)
{
    static_cast<transmission_queue_t*>(q)->push(c);
}

void vla::transmission::serial_read_async_into_queue::set_queue(transmission_queue_t *q)
{
    vla::read_buffer = q;
}

vla::transmission::serial_read_async_into_queue::serial_read_async_into_queue()
{
    vla::read_buffer        = nullptr;
    vla::on_serial_received = char_ready_cb;
    UCSR0B |= (1<<RXEN0)|(1<<RXCIE0);
}

vla::transmission::serial_read_async_into_queue::~serial_read_async_into_queue()
{
    vla::read_buffer        = nullptr;
    vla::on_serial_received = nullptr;
    UCSR0B &= ~((1<<RXEN0)|(1<<RXCIE0));
}

static bool must_transmit(const vla::buffer_msg_t msg)
{
    return msg.size > 0;
}

// helper member functions and callback function to set, handle and cancel alarms
void vla::transmission::enqueue_timeout(void* q)
{
    static_cast<transmission_queue_t*>(q)->push(timeout_msg_t{});
}

vla::alarm_id vla::transmission::set_alarm(vla::period_us us)
{
    return c.set_alarm(us, enqueue_timeout, &q);
}

void vla::transmission::cancel_alarm(vla::alarm_id id)
{
    c.cancel_alarm(id);
}

void vla::transmission::operator()()
{
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
