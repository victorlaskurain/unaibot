#include <vla/shift_register.hpp>
#include <vla/serial.hpp>
#include <stdint.h>
#include <util/delay.h>

using namespace vla;

int main(void) {
    auto &ser = get_serial_debug();
    _delay_ms(1000);
    shift_register<PORTD4_t/*4*/, PORTB4_t/*12*/, PORTD7_t/*7*/, PORTB0_t/*8*/, uint8_t> r;
    r.data = 0xf0;
    r.commit();
    wait(ser, "shift register should show 0xf0");
    r.data = 0x81;
    r.commit();
    wait(ser, "shift register should show 0x81");
    while(true);
}
