#include <vla/main_custom.hpp>

namespace vla {
    static uint16_t oven1_power;
    static uint16_t oven2_power;
    static uint16_t *user_data[] = {
        &oven1_power,
        &oven2_power
    };
    register_values rv{user_data, sizeof user_data / sizeof user_data[0]};
    void init(pdu_handler &pduh)
    {
        // oven outputs
        const uint16_t PIN_B0_IOM_ADDR = 0x48,
                       PIN_B5_IOM_ADDR = PIN_B0_IOM_ADDR + 5;
        for (auto i = PIN_B0_IOM_ADDR; i <= PIN_B5_IOM_ADDR; ++i) {
            pduh.execute_write_single_coil(i, true);
        }
        // push button counter
        const uint16_t PIN_D7_IOM_ADDR = 0x5f,
                       PIN_D7_EC_ADDR  = 0xa7,
                       PIN_D7_EIP_ADDR = 0x8f;
        pduh.execute_write_single_coil(PIN_D7_IOM_ADDR, false);
        pduh.execute_write_single_coil(PIN_D7_EC_ADDR , true);
        pduh.execute_write_single_coil(PIN_D7_EIP_ADDR, true);
        // silo
        const uint16_t PIN_C0_IOM_ADDR       = 0x50,
                       PIN_C0_ANALOG_IN_ADDR = 0x68;
        pduh.execute_write_single_coil(PIN_C0_IOM_ADDR      , false);
        pduh.execute_write_single_coil(PIN_C0_ANALOG_IN_ADDR, true);
        // user data
        pduh.prepend_user_data(&rv);
    }
    void loop(pdu_handler &pduh)
    {}
}
