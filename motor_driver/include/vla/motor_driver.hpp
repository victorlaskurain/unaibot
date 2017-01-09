#ifndef VLA_MOTOR_MOTOR_DRIVER_HPP
#define VLA_MOTOR_MOTOR_DRIVER_HPP

#include <vla/shift_register.hpp>
#include <vla/pwm.hpp>

namespace vla {

    typedef shift_register<PORTD4_t/*4*/,
                           PORTB4_t/*12*/,
                           PORTD7_t/*7*/,
                           PORTB0_t/*8*/,
                           uint8_t> control_register_t;

    enum pole_t : uint8_t
    {
        MOTOR1_A = 2,
        MOTOR1_B = 3,
        MOTOR2_A = 1,
        MOTOR2_B = 4,
        MOTOR3_A = 5,
        MOTOR3_B = 7,
        MOTOR4_A = 0,
        MOTOR4_B = 6
    };

    enum class direction_t : uint8_t
    {
        IDLE      = 0x00,
        FORWARD   = 0x01,
        BACKWARD  = 0x02,
        STOP      = 0x11
    };

    template <typename pwm, pole_t pole1, pole_t pole2>
    class motor
    {
    public:
        motor(control_register_t &c, pwm &p)
            :control(c), power(p)
        {
            set_speed(0);
            set_direction(direction_t::IDLE);
        }
        void set_direction(direction_t d)
        {
            const auto n = static_cast<uint8_t>(d);
            control.data &= ~(1 << pole1);
            control.data &= ~(1 << pole2);
            control.data |= (n >> 1) << pole1;
            control.data |= (n &  1) << pole2;
            control.commit();
        }
        void set_speed(uint8_t s)
        {
            power.set_level(s);
        }
    private:
        control_register_t& control;
        pwm&                power;
    };

    // :TODO: ikusi pwm-ak poloekin ondo parekatuta dauden.
    typedef motor<pcpwm_portb3, pole_t::MOTOR1_A, pole_t::MOTOR1_B> motor0_t;
    typedef motor<pcpwm_portd3, pole_t::MOTOR2_A, pole_t::MOTOR2_B> motor1_t;
    typedef motor<pcpwm_portd6, pole_t::MOTOR3_A, pole_t::MOTOR3_B> motor2_t;
    typedef motor<pcpwm_portd5, pole_t::MOTOR4_A, pole_t::MOTOR4_B> motor3_t;

    class motor_driver
    {
    public:
        motor_driver()
            :pwm0(timer0),
             pwm1(timer0),
             pwm2(timer1),
             pwm3(timer1),
             motor0(control, pwm3),
             motor1(control, pwm2),
             motor2(control, pwm0),
             motor3(control, pwm1)
        {
            timer0.set_clock(vla::clock_source::PRESCALE_1024);
            timer1.set_clock(vla::clock_source::PRESCALE_1024);
        }
        ~motor_driver()
        {
            timer0.set_clock(vla::clock_source::STOP);
            timer1.set_clock(vla::clock_source::STOP);
        }

    private:
        control_register_t control;
        timer0_pcpwm timer0;
        timer2_pcpwm timer1;
        pcpwm_portd6 pwm0;
        pcpwm_portd5 pwm1;
        pcpwm_portd3 pwm2;
        pcpwm_portb3 pwm3;
    public:
        motor0_t motor0;
        motor1_t motor1;
        motor2_t motor2;
        motor3_t motor3;
    };
}

#endif // VLA_MOTOR_MOTOR_DRIVER_HPP
