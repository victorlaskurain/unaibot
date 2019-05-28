#ifndef VLA_MODBUS_RTU_SLAVE_ADC_DAEMON_QUEUE
#define VLA_MODBUS_RTU_SLAVE_ADC_DAEMON_QUEUE

#include <vla/msg_queue.hpp>
#include "adc_set_value_msg.hpp"
#include <stdint.h>

namespace vla {

    constexpr uint8_t to_bit_mask(adc_id_t id)
    {
        return 1<<uint8_t(id);
    }
    constexpr adc_id_t next(adc_id_t adc_id)
    {
        return adc_id_t((uint8_t(adc_id) + 1) % uint8_t(adc_id_t::ADC_MAX));
    }

    typedef bool(*adc_callback_t)(void *data, adc_id_t id, uint16_t value);
    struct adc_set_enabled_msg_t
    {
        adc_id_t       id;
        adc_callback_t cb;
        void*          data;
    };

    // implements a poor man's variant in order to express different
    // message types in a safe and convenient way.
    class adc_msg_t
    {
    public:
        adc_id_t       id   = adc_id_t::ADC_MAX;
        void*          data = 0;
        adc_callback_t cb   = 0;
        adc_msg_t() = default;
        template<typename ReplyQueue>
        adc_msg_t(adc_id_t id, ReplyQueue &q):
            id(id),
            data(&q),
            cb([](void *q, adc_id_t id, uint16_t v) {
                ReplyQueue *queue = static_cast<ReplyQueue*>(q);
                if (queue->full()) {
                    return false;
                }
                queue->push(adc_set_value_msg_t{id, v});
                return true;
            })
        {}
        static adc_msg_t msg_disable(adc_id_t id)
        {
            adc_msg_t msg;
            msg.id = id;
            return msg;
        }
    };

    // this queue never gets called from interruptions so no locking
    // is needed.
    using adc_queue_t = basic_msg_queue<adc_msg_t, 8, NopLock, NopLock>;
}

#endif
