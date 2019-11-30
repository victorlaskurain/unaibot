#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON_QUEUE
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON_QUEUE

#include <vla/tagged_union_queue.hpp>
#include <vla/buffer_msg.hpp>
#include <vla/adc_set_value_msg.hpp>
#include <vla/counters_set_value_msg.hpp>

namespace vla {

    using pdu_handler_msg_t   = tagged_union<
        buffer_msg_t,
        adc_set_value_msg_t,
        counters_set_value_msg_t>;
    using pdu_handler_queue_t = basic_tagged_union_queue<pdu_handler_msg_t, 50, Cli, NopLock>;

}

#endif
