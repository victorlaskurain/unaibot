#ifndef VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON_QUEUE
#define VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON_QUEUE

#include <vla/tagged_union_queue.hpp>
#include <vla/counters_set_value_msg.hpp>
#include <stdint.h>

namespace vla {

    constexpr uint32_t to_bit_mask(counter_id_t id, bool v = true)
    {
        return uint32_t(v)<<uint8_t(id);
    }

    struct counters_set_enabled_msg_t:
        public message_with_reply<counters_set_value_msg_t>
    {
        counter_id_t id      = counter_id_t::COUNTER_MAX;
        bool         enabled = false;
        counters_set_enabled_msg_t() = default;
        counters_set_enabled_msg_t(counter_id_t id, bool enabled):
            id(id), enabled(enabled){}
        template<typename ReplyQueue>
        counters_set_enabled_msg_t(counter_id_t id, bool enabled, ReplyQueue &q):
            message_with_reply<counters_set_value_msg_t>(q),
            id(id), enabled(enabled){}
    };

    class counters_daemon_msg_t: public tagged_union<
        counters_set_enabled_msg_t,
        counters_set_value_msg_t>
    {
        using parent_t = tagged_union<
            counters_set_enabled_msg_t,
            counters_set_value_msg_t>;
    public:
        counters_daemon_msg_t() = default;
        template<typename T>
        counters_daemon_msg_t(T&&v):parent_t(v)
        {}
        counters_daemon_msg_t(counter_id_t id, uint16_t v):
            parent_t(counters_set_value_msg_t{id, v})
        {}
    };
    // this queue never gets called from interruptions so no locking
    // is needed.
    using counters_queue_t = basic_tagged_union_queue<
        counters_daemon_msg_t,
        8, NopLock, NopLock>;
}

#endif
