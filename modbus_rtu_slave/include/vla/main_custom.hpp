#ifndef VLA_MODBUS_RTU_SLAVE_MAIN
#define VLA_MODBUS_RTU_SLAVE_MAIN

#include <vla/pdu_handler_daemon.hpp>
#include <vla/rt_clock.hpp>

namespace vla {
    void init(pdu_handler &pdu);
    void loop(pdu_handler &pdu, rt_clock &clock);
}

#endif
