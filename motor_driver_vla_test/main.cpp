#include <vla/motor_driver_vla.hpp>
#include <vla/serial.hpp>
#include <stdint.h>
#include <util/delay.h>

using namespace vla;

static void motor0(motor_driver &motors, decltype(get_serial_debug())& ser) {
    auto& motor0 = motors.motor0;
    wait(ser, "Test the outputs with a polymeter.");
    motor0.set_speed(direction_t::FORWARD, UINT8_MAX);
    wait(ser, "motor0 full power forward");
    motor0.set_speed(direction_t::FORWARD, UINT8_MAX / 4);
    wait(ser, "motor0 quarter power forward");
    motor0.set_speed(vla::direction_t::BACKWARD, UINT8_MAX);
    wait(ser, "motor0 full power backward");
    motor0.set_speed(vla::direction_t::BACKWARD, UINT8_MAX / 4);
    wait(ser, "motor0 quarter power backward");
    motor0.set_speed(vla::direction_t::FORWARD, 0);
    wait(ser, "motor0 stop");
}

static void motor1(motor_driver &motors, decltype(get_serial_debug())& ser) {
    auto& motor1 = motors.motor1;
    wait(ser, "Test the outputs with a polymeter.");
    motor1.set_speed(direction_t::FORWARD, UINT8_MAX);
    wait(ser, "motor1 full power forward");
    motor1.set_speed(direction_t::FORWARD, UINT8_MAX / 4);
    wait(ser, "motor1 quarter power forward");
    motor1.set_speed(vla::direction_t::BACKWARD, UINT8_MAX);
    wait(ser, "motor1 full power backward");
    motor1.set_speed(vla::direction_t::BACKWARD, UINT8_MAX / 4);
    wait(ser, "motor1 quarter power backward");
    motor1.set_speed(vla::direction_t::FORWARD, 0);
    wait(ser, "motor1 stop");
}

int main__()
{
    auto &ser = get_serial_debug();
    while (true) {
        PORTD5_t::set();
        wait(ser, "D5 = 1");
        PORTD5_t::clear();
        wait(ser, "D5 = 0");
    }
}

int main()
{
    auto &ser = get_serial_debug();
    PORTD7_t::set_mode_output();
    PORTD7_t::set();
    motor_driver motors;
    motor0(motors, ser);
    motor1(motors, ser);
    write(ser, "done");
    while(true);
}
