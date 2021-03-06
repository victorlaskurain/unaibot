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
        uint8_t  line1_on         = 0;
        uint8_t  padding3         = 0;
        uint8_t  line2_on         = 0;
        uint8_t  padding4         = 0;
        uint16_t counter1_top     = 0;
        uint16_t counter2_top     = 0;
        uint16_t counter1_kg_ref  = 0;
        uint16_t counter2_kg_ref  = 0;
        uint16_t counter1_kg      = 0;
        uint16_t counter2_kg      = 0;
    } user_data;
    static_assert(sizeof (user_data) == 24, "Unexpected user data size");
    static register_values rv{&user_data};
    static uint16_t &oven1_power      = user_data.oven1_power;
    static uint16_t &oven2_power      = user_data.oven2_power;
    static uint8_t  &grinder1_program = user_data.grinder1_program;
    static uint8_t  &grinder2_program = user_data.grinder2_program;
    static uint8_t  &line1_on         = user_data.line1_on;
    static uint8_t  &line2_on         = user_data.line2_on;
    static uint16_t &counter1_top     = user_data.counter1_top;
    static uint16_t &counter2_top     = user_data.counter2_top;
    static uint16_t &counter1_kg_ref  = user_data.counter1_kg_ref;
    static uint16_t &counter2_kg_ref  = user_data.counter2_kg_ref;
    static uint16_t &counter1_kg      = user_data.counter1_kg;
    static uint16_t &counter2_kg      = user_data.counter2_kg;
    static uint16_t counter1_last     = 0;
    static uint16_t counter2_last     = 0;

    uint16_t read_weight_1(const timestamp_us ts)
    {
        return counter1_kg_ref + (ts.us / 50 % 200) - 100;
    }

    uint16_t read_weight_2(const timestamp_us ts)
    {
        return counter2_kg_ref + (ts.us / 50 % 200) - 100;
    }

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
        constexpr uint16_t COUNTER1_ADDR          = 0x21,
                           COUNTER2_ADDR          = 0x15,
                           LINE1_MAIN_SWITCH_ADDR = 0x34,
                           LINE2_MAIN_SWITCH_ADDR = 0x35;
        auto now = clock.get_current_time();
        if (line1_on) {
            uint16_t counter1;
            // set oven control lines and grinder
            set_with_mask<PORTB_t>(0b00000111, to_3bits(oven1_power) << 0);
            set_grinder<PORTC1_t>(now, grinder1_program);
            pduh.execute_read_single_register(COUNTER1_ADDR, &counter1);
            if (counter1_top) {
                if (counter1 == counter1_top) {
                    pduh.execute_write_single_register(LINE1_MAIN_SWITCH_ADDR, 0);
                }
            }
            if (counter1 > counter1_last) {
                counter1_kg = read_weight_1(now);
            }
            counter1_last = counter1;
        } else {
            // oven 1 and grinder 1 off
            set_with_mask<PORTB_t>(0b00000111, 0);
            set_grinder<PORTC1_t>(now, 0);
        }
        if (line2_on) {
            uint16_t counter2;
            // set oven control lines and grinder
            set_with_mask<PORTB_t>(0b00111000, to_3bits(oven2_power) << 3);
            set_grinder<PORTC2_t>(now, grinder2_program);
            pduh.execute_read_single_register(COUNTER2_ADDR, &counter2);
            if (counter2_top) {
                if (counter2 == counter2_top) {
                    pduh.execute_write_single_register(LINE2_MAIN_SWITCH_ADDR, 0);
                }
            }
            if (counter2 > counter2_last) {
                counter2_kg = read_weight_2(now);
            }
            counter2_last = counter2;
        } else {
            // oven 1 and grinder 1 off
            set_with_mask<PORTB_t>(0b00111000, 0);
            set_grinder<PORTC2_t>(now, 0);
        }
    }
}
