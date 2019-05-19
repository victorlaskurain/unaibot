#ifndef VLA_MODBUS_RTU_SLAVE_ADC_SET_VALUE_MSG
#define VLA_MODBUS_RTU_SLAVE_ADC_SET_VALUE_MSG

#include <stdint.h>

namespace vla {
    enum class adc_id_t : uint8_t
    {
        ADC0,
        ADC1,
        ADC2,
        ADC3,
        ADC4,
        ADC5,
        ADC6,
        ADC7,
        ADC_MAX
    };
    // this is a reply message
    struct adc_set_value_msg_t
    {
        adc_id_t id;
        uint16_t value;
    };
}


#endif
