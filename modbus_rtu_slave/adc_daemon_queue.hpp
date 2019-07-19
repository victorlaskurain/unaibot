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

    struct adc_msg_t: public message_with_reply<adc_set_value_msg_t>
    {
        adc_id_t id      = adc_id_t::ADC_MAX;
        bool     enabled = false;
        adc_msg_t() = default;
        adc_msg_t(adc_id_t id, bool enabled):
            id(id), enabled(enabled){}
        template<typename ReplyQueue>
        adc_msg_t(adc_id_t id, bool enabled, ReplyQueue &q):
            message_with_reply<adc_set_value_msg_t>(q),
            id(id), enabled(enabled){}
    };

    // this queue never gets called from interruptions so no locking
    // is needed.
    using adc_queue_t = basic_msg_queue<adc_msg_t, 8, NopLock, NopLock>;
}

#endif
