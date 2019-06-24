#ifndef VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON
#define VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON

#include "counters_daemon_queue.hpp"
#include <vla/protothreads.hpp>
#include <vla/rt_clock.hpp>

namespace vla {

    class counters_daemon : public ptxx_thread
    {
        rt_clock         &clock;
        counters_queue_t &in_q;
        // protothread state
        uint8_t i;
        const counters_set_enabled_msg_t *cb_data;
    public:
        counters_daemon(rt_clock &clock, counters_queue_t &in_q);
        void operator()();
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_COUNTERS_DAEMON
