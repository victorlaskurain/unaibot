#ifndef VLA_MODBUS_RTU_SLAVE_MAIN
#define VLA_MODBUS_RTU_SLAVE_MAIN

#include <vla/pdu_handler_daemon.hpp>

namespace vla {
    void init(pdu_handler &pdu);
    void loop(pdu_handler &pdu);
}

#endif
