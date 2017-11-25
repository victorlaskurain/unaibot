#include <vla/motor_driver_vla.hpp>
#include <vla/serial.hpp>
#include "main_common.hpp"

using namespace vla;

/**
 * Examples:
 * Motor 2 and motor 3 at about halve speed forward
 * SPEEDS2 0x01 0x80 1 0x80
 * Motor 2 halve backward, motor 3 full forward
 * SPEEDS2 0x02 0x80 0x01 0xff
 * Stop both motors
 * SPEEDS2 0x00 0x00 0x00 0x00
 */
struct set_speeds_cmd
{
    set_speeds_cmd(motor_driver &motors)
        :motors(motors)
    {}
    inline uint8_t operator()(const char* str)
    {
        uint8_t     m0_speed    , m1_speed;
        direction_t m0_direction, m1_direction;
        if (skip_prefix (&str, "SPEEDS2")       &&
            read_direction(&str, &m0_direction) &&
            read_uint8_hex(&str, &m0_speed)     &&
            read_direction(&str, &m1_direction) &&
            read_uint8_hex(&str, &m1_speed)) {
            motors.motor0.set_direction(m0_direction);
            motors.motor0.set_speed    (m0_speed);
            motors.motor1.set_direction(m1_direction);
            motors.motor1.set_speed    (m1_speed);
            return SUCCESS;
        }
        return ERROR;
    }
    motor_driver &motors;
};

int main()
{
    motor_main<set_speeds_cmd>();
}
