#include <vla/serial.hpp>
#include <vla/rt_clock.hpp>

using namespace vla;

/**
 * Use the clock classes to set up a clock tick at 20Khz. Use the tick
 * to let the led at pin B5 toggle every 0.5"
 */

static auto &ser = get_serial_async_debug();

enum {TICK_0A, TICK_0B};
const char *tags[] = {"0A", "0B", "2A", "2B"};
const char *current_tag = tags[0];

static void every_50ms(void*)
{
    const uint16_t c500ms       = 10000;
    static uint16_t up_to_500ms = 0;
    ++up_to_500ms;
    if (c500ms == up_to_500ms) {
        if (PORTB5_t::get()) {
            PORTB5_t::clear();
        } else {
            PORTB5_t::set();
        }
        write_line(ser, current_tag);
        up_to_500ms = 0;
    }
}

int main(int argc, char **argv)
{
    write_line(ser, "BEGIN");
    PORTB5_t::set_mode_output();
    // tick_0a_20khz tick(every_50ms);
    while (true) {
        {
            current_tag = tags[0];
            tick_0a_20khz tick(every_50ms);
            _delay_ms(2000);
        }
        write_line(ser, "2\" PAUSE");
        _delay_ms(2000);

        {
            current_tag = tags[1];
            tick_0b_20khz tick(every_50ms);
            _delay_ms(2000);
        }
        write_line(ser, "2\" PAUSE");
        _delay_ms(2000);

        {
            current_tag = tags[2];
            tick_2a_20khz tick(every_50ms);
            _delay_ms(2000);
        }
        write_line(ser, "2\" PAUSE");
        _delay_ms(2000);

        {
            current_tag = tags[3];
            tick_2b_20khz tick(every_50ms);
            _delay_ms(2000);
        }
        write_line(ser, "2\" PAUSE");
        _delay_ms(2000);
    }
}
