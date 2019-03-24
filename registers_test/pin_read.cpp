#include <vla/registers.hpp>
#include <vla/serial.hpp>
#include <stdint.h>
#include <util/delay.h>

using namespace vla;

int main(void) {
    auto& ser = get_serial_debug();
    // PORTD0 and PORTD1 are the serial port. They are reserved on the
    // arduino so ignore them here.
    PORTD2_t::set_mode_input();
    PORTD3_t::set_mode_input();
    PORTD4_t::set_mode_input();
    PORTD5_t::set_mode_input();
    PORTD6_t::set_mode_input();
    PORTD7_t::set_mode_input();
    PORTD0_t::switch_on_pull_up();
    PORTD1_t::switch_on_pull_up();
    PORTD2_t::switch_on_pull_up();
    PORTD3_t::switch_on_pull_up();
    PORTD4_t::switch_on_pull_up();
    PORTD5_t::switch_on_pull_up();
    PORTD6_t::switch_on_pull_up();
    PORTD7_t::switch_on_pull_up();
    while(1) {
        if (PORTD2_t::get()) {
            write(ser, '1');
        } else {
            write(ser, '0');
        }
        if (PORTD3_t::get()) {
            write(ser, '1');
        } else {
            write(ser, '0');
        }
        if (PORTD4_t::get()) {
            write(ser, '1');
        } else {
            write(ser, '0');
        }
        if (PORTD5_t::get()) {
            write(ser, '1');
        } else {
            write(ser, '0');
        }
        if (PORTD6_t::get()) {
            write(ser, '1');
        } else {
            write(ser, '0');
        }
        if (PORTD7_t::get()) {
            write(ser, '1');
        } else {
            write(ser, '0');
        }
        write_line(ser, "");
        write(ser, "-->");
        write_line(ser, "");
        _delay_ms(1000);
    }
}
