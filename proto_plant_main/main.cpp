#include <vla/main_custom.hpp>
#include <vla/rt_clock.hpp>

namespace vla {

    uint8_t to_3bits(uint8_t v)
    {
        uint8_t r = 0;
        for (; v > 0; --v) {
            r = (r << 1) | 1;
        }
        return r;
    }

    template<typename PIN, const uint32_t period = 2000000> // 2"
    void set_grinder(const timestamp_us now, uint8_t program)
    {
        const uint32_t on_time_slice_us = program * (period / 255);
        if (now.us % period <= on_time_slice_us) {
            PIN::set();
        } else {
            PIN::clear();
        }
    }

    static struct {
        uint16_t oven1_power      = 0;
        uint16_t oven2_power      = 0;
        uint8_t  grinder1_program = 0;
        uint8_t  padding1         = 0;
        uint8_t  grinder2_program = 0;
        uint8_t  padding2         = 0;
    } user_data;
    static_assert(sizeof (user_data) == 8, "Unexpected user data size");
    static register_values rv{&user_data};
    static uint16_t &oven1_power      = user_data.oven1_power;
    static uint16_t &oven2_power      = user_data.oven2_power;
    static uint8_t  &grinder1_program = user_data.grinder1_program;
    static uint8_t  &grinder2_program = user_data.grinder2_program;
    void init(pdu_handler &pduh)
    {
        // oven outputs
        const uint16_t PIN_B0_IOD_ADDR = 0x48,
                       PIN_B5_IOD_ADDR = PIN_B0_IOD_ADDR + 5;
        for (auto i = PIN_B0_IOD_ADDR; i <= PIN_B5_IOD_ADDR; ++i) {
            pduh.execute_write_single_coil(i, true);
        }
        // push button counter
        const uint16_t PIN_D7_IOD_ADDR = 0x5f,
                       PIN_D7_EC_ADDR  = 0xa7,
                       PIN_D7_EIP_ADDR = 0x8f;
        pduh.execute_write_single_coil(PIN_D7_IOD_ADDR, false);
        pduh.execute_write_single_coil(PIN_D7_EC_ADDR , true);
        pduh.execute_write_single_coil(PIN_D7_EIP_ADDR, true);
        // photoresistor counter
        const uint16_t PIN_C3_IOD_ADDR = 0x53,
                       PIN_C3_EC_ADDR  = 0x9b,
                       PIN_C3_EIP_ADDR = 0x83;
        pduh.execute_write_single_coil(PIN_C3_IOD_ADDR, false);
        pduh.execute_write_single_coil(PIN_C3_EC_ADDR , true);
        pduh.execute_write_single_coil(PIN_C3_EIP_ADDR, true);
        // silo
        const uint16_t PIN_C0_IOD_ADDR       = 0x50,
                       PIN_C0_ANALOG_IN_ADDR = 0x68;
        pduh.execute_write_single_coil(PIN_C0_IOD_ADDR      , false);
        pduh.execute_write_single_coil(PIN_C0_ANALOG_IN_ADDR, true);
        // line 1 grinder
        const uint16_t PIN_C1_IOD_ADDR       = 0x51,
                       PIN_C1_OUT_ADDR       = 0xc9;
        pduh.execute_write_single_coil(PIN_C1_IOD_ADDR, true);
        pduh.execute_write_single_coil(PIN_C1_OUT_ADDR, false);
        // line 2 grinder
        const uint16_t PIN_C2_IOD_ADDR       = 0x52,
                       PIN_C2_OUT_ADDR       = 0xca;
        pduh.execute_write_single_coil(PIN_C2_IOD_ADDR, true);
        pduh.execute_write_single_coil(PIN_C2_OUT_ADDR, false);
        // user data
        pduh.append_user_data(&rv);
    }
    void loop(pdu_handler &pduh, rt_clock &clock)
    {
        // set oven control lines
        set_with_mask<PORTB_t>(
            0b00111111,
            (to_3bits(oven1_power) << 0) | (to_3bits(oven2_power) << 3));

        // set grinders
        auto now = clock.get_current_time();
        set_grinder<PORTC1_t>(now, grinder1_program);
        set_grinder<PORTC2_t>(now, grinder2_program);
    }
}
