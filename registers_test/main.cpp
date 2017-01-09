#include <vla/registers.hpp>
#include <stdint.h>

using namespace vla;

/**
 * f1 y f2 generan exactamente el mismo ensamblador si se usa -O3
 */
uint8_t f1()
{
    PORTB |= _BV(PORTB0) | _BV(PORTB1);
    return -1;
}

uint8_t f2()
{
    PORTB_t::ref() |= _BV(PORTB0_t::offset()) | _BV(PORTB1_t::offset());
    return -1;
}

/**
 * f3 y f4 generan exactamente el mismo ensamblador si se usa -O3
 */
uint8_t f3()
{
    PORTB |= _BV(PORTB0);
    PORTB &= ~_BV(PORTB0);
    return -1;
}

uint8_t f4()
{
    PORTB0_t::set();
    PORTB0_t::clear();
    return -1;
}

int main(void) {
    f1();
    f2();
    f3();
    f4();
}
