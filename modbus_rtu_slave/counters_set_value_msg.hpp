#ifndef VLA_MODBUS_RTU_SLAVE_COUNTERS_SET_VALUE_MSG
#define VLA_MODBUS_RTU_SLAVE_COUNTERS_SET_VALUE_MSG

#include <stdint.h>

namespace vla {
    enum class counter_id_t : uint8_t
    {
        COUNTERS0,
        COUNTERS1,
        COUNTERS2,
        COUNTERS3,
        COUNTERS4,
        COUNTERS5,
        COUNTERS6,
        COUNTERS7,
        COUNTERS8,
        COUNTERS9,
        COUNTERS10,
        COUNTERS11,
        COUNTERS12,
        COUNTERS13,
        COUNTERS14,
        COUNTERS15,
        COUNTERS16,
        COUNTERS17,
        COUNTERS18,
        COUNTERS19,
        COUNTERS20,
        COUNTERS21,
        COUNTERS22,
        COUNTERS23,
        COUNTER_MAX
    };
    // this is a reply message
    struct counters_set_value_msg_t
    {
        counter_id_t id;
        uint16_t value;
    };
}

#endif
