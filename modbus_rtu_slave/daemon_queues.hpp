#ifndef VLA_MODBUS_RTU_SLAVE_DAEMON_QUEUES
#define VLA_MODBUS_RTU_SLAVE_DAEMON_QUEUES

#include <vla/msg_queue.hpp>
#include <stdint.h>

namespace vla {

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

}

#endif // VLA_MODBUS_RTU_SLAVE_DAEMON_QUEUES
