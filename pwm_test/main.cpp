#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <vla/pwm.hpp>
#include <vla/serial.hpp>

int main(void) {
    vla::timer0_pcpwm timer0;
    vla::timer2_pcpwm timer2;
    timer0.set_clock(vla::clock_source::PRESCALE_1);
    timer2.set_clock(vla::clock_source::PRESCALE_1);
    vla::pcpwm_portd6 pwm0a(timer0);
    vla::pcpwm_portd5 pwm0b(timer0);
    vla::pcpwm_portd3 pwm2a(timer2);
    vla::pcpwm_portb3 pwm2b(timer2);
    uint8_t duty = 0;
    while (1) {
        do {
            pwm0a.set_level(duty);
            pwm0b.set_level(UINT8_MAX - duty);
            pwm2a.set_level(duty - UINT8_MAX / 2);
            pwm2b.set_level(UINT8_MAX / 2 - duty);
            ++duty;
            _delay_ms(100);
        } while (duty != UINT8_MAX);
        do {
            pwm0a.set_level(duty);
            pwm0b.set_level(UINT8_MAX - duty);
            pwm2a.set_level(duty - UINT8_MAX / 2);
            pwm2b.set_level(UINT8_MAX / 2 - duty);
            --duty;
            _delay_ms(100);
        } while (duty != 0);
    }
}
