#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON

#include "daemon_queues.hpp"
#include <vla/protothreads.hpp>

namespace vla {

    class pdu_handler : public ptxx_thread
    {
        pdu_handler_queue_t &in_q;
        transmission_queue_t &out_q;
        buffer_msg_t msg = {};
    public:
        pdu_handler(
            pdu_handler_queue_t &in_q,
            transmission_queue_t &out_q):in_q(in_q), out_q(out_q) {}
        void operator()();
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
