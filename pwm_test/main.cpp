#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <pwm.hpp>

int main(void) {
    vla::pwm0pc pwm0;
    vla::pwm2pc pwm2;
    pwm0.set_clock(vla::clock_source::PRESCALE_1);
    pwm2.set_clock(vla::clock_source::PRESCALE_1);
    uint8_t duty = 0;
    while (1) {
        do {
            pwm0.set_level_a(duty);
            pwm0.set_level_b(UINT8_MAX - duty);
            pwm2.set_level_a(duty - UINT8_MAX / 2);
            pwm2.set_level_b(UINT8_MAX / 2 - duty);
            ++duty;
            _delay_ms(100);
        } while (duty != UINT8_MAX);
        do {
            pwm0.set_level_a(duty);
            pwm0.set_level_b(UINT8_MAX - duty);
            pwm2.set_level_a(duty - UINT8_MAX / 2);
            pwm2.set_level_b(UINT8_MAX / 2 - duty);
            --duty;
            _delay_ms(100);
        } while (duty != 0);
    }
}
