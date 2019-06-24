#ifndef VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON_QUEUE
#define VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON_QUEUE

#include <vla/tagged_union_queue.hpp>
#include "counters_set_value_msg.hpp"
#include <stdint.h>

namespace vla {

    constexpr uint32_t to_bit_mask(counter_id_t id, bool v = true)
    {
        return uint32_t(v)<<uint8_t(id);
    }

    typedef bool(*counters_callback_t)(void *data, counter_id_t id, uint16_t value);
    struct counters_set_enabled_msg_t
    {
        counter_id_t        id;
        counters_callback_t cb;
        void*               data;
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
        template<typename ReplyQueue>
        counters_daemon_msg_t(counter_id_t id, ReplyQueue &q):
            parent_t(counters_set_enabled_msg_t{
                    id,
                    [](void *q, counter_id_t id, uint16_t v) {
                        ReplyQueue *queue = static_cast<ReplyQueue*>(q);
                        if (queue->full()) {
                            return false;
                        }
                        queue->push(counters_set_value_msg_t{id, v});
                        return true;
                    },
                    static_cast<ReplyQueue*>(&q)
                }){}
        static counters_daemon_msg_t msg_disable(counter_id_t id)
        {
            return counters_set_enabled_msg_t{id, nullptr, nullptr};
        }
    };
    // this queue never gets called from interruptions so no locking
    // is needed.
    using counters_queue_t = basic_tagged_union_queue<
        counters_daemon_msg_t,
        8, NopLock, NopLock>;
}

#endif
