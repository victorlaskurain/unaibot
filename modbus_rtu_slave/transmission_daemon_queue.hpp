#ifndef VLA_MODBUS_RTU_SLAVE_TRANSMISSION_DAEMON_QUEUE
#define VLA_MODBUS_RTU_SLAVE_TRANSMISSION_DAEMON_QUEUE

#include <vla/tagged_union_queue.hpp>
#include "timeout_msg.hpp"
#include "buffer_msg.hpp"
#include <stdint.h>

namespace vla {

    using input_msg_t          = uint8_t;
    using transmission_msg_t   = tagged_union<timeout_msg_t, input_msg_t, buffer_msg_t>;
    using transmission_queue_t = basic_tagged_union_queue<transmission_msg_t, 50, Cli, NopLock>;

}

#endif
