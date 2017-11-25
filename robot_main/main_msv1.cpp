#include <vla/motor_driver.hpp>
#include <vla/serial.hpp>
#include <vla/command.hpp>
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
        uint8_t     m2_speed    , m3_speed;
        direction_t m2_direction, m3_direction;
        if (skip_prefix (&str, "SPEEDS2")       &&
            read_direction(&str, &m2_direction) &&
            read_uint8_hex(&str, &m2_speed)     &&
            read_direction(&str, &m3_direction) &&
            read_uint8_hex(&str, &m3_speed)) {
            motors.motor2.set_direction(m2_direction);
            motors.motor2.set_speed    (m2_speed);
            motors.motor3.set_direction(m3_direction);
            motors.motor3.set_speed    (m3_speed);
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
