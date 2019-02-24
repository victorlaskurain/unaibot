#include <vla/adc.hpp>
#include <vla/serial.hpp>
#include <stdint.h>
#include <util/delay.h>
#include <stdlib.h>

using namespace vla;

static const char hex_chr[] = {'0', '1', '2', '3',
                               '4', '5', '6', '7',
                               '8', '9', 'a', 'b',
                               'c', 'd', 'e', 'f'};
static const char* hex(uint8_t n) {
    static char hex_buffer[] = "0x00";
    hex_buffer[2] = hex_chr[n>>4];
    hex_buffer[3] = hex_chr[n & 0xf];
    return hex_buffer;
}


int main(void) {
    const int MIN_CHANGE = 2;
    auto &ser = get_serial_debug();
    write_line(ser, "BEGIN");
    auto lastv = -1;
    while (true) {
        auto v = analog_read<PORTC0_t>();
        if (abs(v - lastv) >= MIN_CHANGE) {
            lastv = v;
            write_line(ser, hex(v));
        }
    }
}
