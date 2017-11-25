#ifndef VLA_MOTOR_MOTOR_DRIVER_HPP
#define VLA_MOTOR_MOTOR_DRIVER_HPP

#include <vla/shift_register.hpp>
#include <vla/pwm.hpp>

namespace vla {

    enum class direction_t : uint8_t
    {
        IDLE      = 0x00,
        FORWARD   = 0x01,
        BACKWARD  = 0x02,
        STOP      = 0x11,
        FIRST     = IDLE,
        LAST      = STOP
    };

    template <typename pwm>
    class motor
    {
    public:
        motor(pwm &p)
            :power(p)
        {
            set_speed(direction_t::FORWARD, 0);
        }
        /**
         * for compatibility with the other driver
         */
        void set_direction(direction_t d)
        {
            set_speed(d, speed);
        }
        void set_speed(uint8_t s)
        {
            set_speed(direction, s);
        }
        void set_speed(direction_t d, uint8_t s)
        {
            direction = d;
            speed     = s;
            switch (d) {
            case direction_t::FORWARD:
                s = 127 - (s >> 1);
                break;
            case direction_t::BACKWARD:
                s = 127 + (s >> 1);
                break;
            case direction_t::IDLE:
            case direction_t::STOP:
            default:
                s = 127;
                break;
            }
            power.set_level(s);
        }
    private:
        uint8_t     speed;
        direction_t direction;
        pwm&        power;
    };

    typedef motor<pcpwm_portd5> motor0_t;
    typedef motor<pcpwm_portd6> motor1_t;

    class motor_driver
    {
    public:
        motor_driver()
            :pwm0(timer0),
             pwm1(timer0),
             motor0(pwm0),
             motor1(pwm1)
        {
            timer0.set_clock(vla::clock_source::PRESCALE_1);
        }
        ~motor_driver()
        {
            timer0.set_clock(vla::clock_source::STOP);
        }

    private:
        timer0_pcpwm timer0;
        pcpwm_portd5 pwm0;
        pcpwm_portd6 pwm1;
    public:
        motor0_t motor0;
        motor1_t motor1;
    };
}

#endif // VLA_MOTOR_MOTOR_DRIVER_HPP
