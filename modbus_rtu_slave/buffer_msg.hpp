#ifndef VLA_MODBUS_RTU_SLAVE_BUFFER_MSG
#define VLA_MODBUS_RTU_SLAVE_BUFFER_MSG

#include <stdint.h>

namespace vla {
    struct buffer_msg_t
    {
        uint8_t *data;
        uint8_t size;
    };
}


#endif
