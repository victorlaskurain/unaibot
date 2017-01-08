#ifndef VLA_PWM_HPP
#define VLA_PWM_HPP

#include <avr/io.h>
#include <util/delay.h>
#include <registers.hpp>

namespace vla {

    /*
     * Timer output	Arduino output	Chip pin	Pin name
     * OC0A                      6        12        PD6
     * OC0B                      5        11        PD5
     * OC1A                      9        15        PB1
     * OC1B                     10        16        PB2
     * OC2A                     11        17        PB3
     * OC2B                      3         5        PD3
     */

    enum class clock_source {
        STOP,
        PRESCALE_1,
        PRESCALE_8,
        PRESCALE_64,
        PRESCALE_256,
        PRESCALE_1024,
        EXTERNAL_CLOCK_T0_FALLING,
        EXTERNAL_CLOCK_T0_RISING
    };

    enum class timer_mode : uint8_t {
        PHASE_CORRECT_PWM = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00)
    };

    template<typename timer_register_a_t,
             typename timer_register_b_t,
             timer_mode mode>
    struct timer_configuration
    {
        typedef timer_register_a_t timer_register_a;
        typedef timer_register_b_t timer_register_b;
        inline timer_configuration()
        {
            timer_register_a::ref() = static_cast<uint8_t>(mode);
        }
        inline ~timer_configuration()
        {
            set_clock(clock_source::STOP);
        }
        inline void set_clock(clock_source cs)
        {
            timer_register_b::ref() = static_cast<uint8_t>(cs);
        }
    };

    template<typename timer_configuration,
             typename comparator_register,
             typename out_pin>
    struct pwm
    {
        /**
         * The parameters is never used, it's there just to force a
         * dependency thus ensuring you don't try to use pwm without
         * configuring a timer.
         */
        inline pwm(const timer_configuration&)
        {
            out_pin::set_mode_output();
        }

        inline void set_level(uint8_t v)
        {
            comparator_register::ref() = v;
        }
    private:
        pwm(const pwm&)                  = delete;
        pwm(pwm&&)                       = delete;
        pwm& operator=(const pwm &other) = delete;
        pwm& operator=(pwm &&other)      = delete;
    };

    typedef timer_configuration<TCCR0A_t,
                                TCCR0B_t,
                                timer_mode::PHASE_CORRECT_PWM> timer0_pcpwm;
    typedef timer_configuration<TCCR2A_t,
                                TCCR2B_t,
                                timer_mode::PHASE_CORRECT_PWM> timer2_pcpwm;

    typedef pwm<timer0_pcpwm, OCR0A_t, PORTD6_t> pcpwm_portd6;
    typedef pwm<timer0_pcpwm, OCR0B_t, PORTD5_t> pcpwm_portd5;
    typedef pwm<timer2_pcpwm, OCR2A_t, PORTB3_t> pcpwm_portd3;
    typedef pwm<timer2_pcpwm, OCR2B_t, PORTD3_t> pcpwm_portb3;

}

#endif // VLA_PWM_HPP
