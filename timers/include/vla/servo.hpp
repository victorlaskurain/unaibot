#ifndef VLA_SERVO_HPP
#define VLA_SERVO_HPP

#include <vla/registers.hpp>

namespace vla {

    class servo
    {
        volatile uint8_t &port;
        uint8_t bit_number:3;
        uint16_t pulse_length_us;
    public:
        static constexpr uint16_t INSTANT_MAX = 20000;
        template<typename port_t>
        servo(port_t &p):port(p.ref()), bit_number(p.bit_number())
        {
        }
        void set_pulse_length(uint16_t us)
        {
            pulse_length_us = us;
        }
        void update(uint16_t instant_us)
        {
            const bool v = (instant_us < pulse_length_us);
            port = (port & ~(1<<bit_number)) | (v<<bit_number);
        }
    };

}

#endif // VLA_SERVO_HPP
