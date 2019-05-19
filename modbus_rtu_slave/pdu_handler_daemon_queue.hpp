#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON_QUEUE
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON_QUEUE

#include <vla/msg_queue.hpp>
#include "buffer_msg.hpp"
#include "timeout_msg.hpp"
#include "adc_set_value_msg.hpp"
#include <stdint.h>

namespace vla {

    // implements a poor man's variant in order to express different
    // message types in a safe and convenient way.
    class pdu_handler_msg_t
    {
        enum class pdu_handler_msg_tag
        {
            NONE,
            BUFFER,
            ADC_SET_VALUE
        };
        pdu_handler_msg_tag tag = pdu_handler_msg_tag::NONE;
        union {
            buffer_msg_t        buffer;
            adc_set_value_msg_t adc_set_value;
        } msg;
    public:
        pdu_handler_msg_t()                   = default;
        pdu_handler_msg_t(pdu_handler_msg_t&) = default;
        template<typename T>
        pdu_handler_msg_t(T&& v)
        {
            put(v);
        }
        bool get(buffer_msg_t &v)
        {
            if (pdu_handler_msg_tag::BUFFER == tag) {
                v = msg.buffer;
                return true;
            }
            return false;
        }
        bool get(adc_set_value_msg_t &v)
        {
            if (pdu_handler_msg_tag::ADC_SET_VALUE == tag) {
                v = msg.adc_set_value;
                return true;
            }
            return false;
        }
        void put(buffer_msg_t v)
        {
            msg.buffer = v;
            tag = pdu_handler_msg_tag::BUFFER;
        }
        void put(adc_set_value_msg_t v)
        {
            msg.adc_set_value = v;
            tag = pdu_handler_msg_tag::ADC_SET_VALUE;
        }
    };

    struct pdu_handler_queue_t: public basic_msg_queue<pdu_handler_msg_t, 50, Cli, NopLock>
    {
        using parent_t = basic_msg_queue<pdu_handler_msg_t, 50, Cli, NopLock>;
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
}

#endif
