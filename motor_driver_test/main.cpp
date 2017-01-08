#include <motor_driver.hpp>
#include <serial.hpp>
#include <stdint.h>
#include <util/delay.h>

using namespace vla;

static void motor0(motor_driver &motors, decltype(get_serial_debug())& ser) {
    auto& motor0 = motors.motor0;
    wait(ser, "Test the outputs with a polymeter.");
    motor0.set_speed(UINT8_MAX);
    motor0.set_direction(vla::direction_t::FORWARD);
    wait(ser, "motor0 full power forward");
    motor0.set_speed(UINT8_MAX / 4);
    wait(ser, "motor0 quarter power forward");
    motor0.set_speed(UINT8_MAX);
    motor0.set_direction(vla::direction_t::BACKWARD);
    wait(ser, "motor0 full power backward");
    motor0.set_speed(UINT8_MAX / 4);
    wait(ser, "motor0 quarter power backward");
    motor0.set_speed(0);
    wait(ser, "motor0 idle");
}

static void motor1(motor_driver &motors, decltype(get_serial_debug())& ser) {
    auto& motor1 = motors.motor1;
    wait(ser, "Test the outputs with a polymeter.");
    motor1.set_speed(UINT8_MAX);
    motor1.set_direction(vla::direction_t::FORWARD);
    wait(ser, "motor1 full power forward");
    motor1.set_speed(UINT8_MAX / 4);
    wait(ser, "motor1 quarter power forward");
    motor1.set_speed(UINT8_MAX);
    motor1.set_direction(vla::direction_t::BACKWARD);
    wait(ser, "motor1 full power backward");
    motor1.set_speed(UINT8_MAX / 4);
    wait(ser, "motor1 quarter power backward");
    motor1.set_speed(0);
    wait(ser, "motor1 idle");
}

static void motor2(motor_driver &motors, decltype(get_serial_debug())& ser) {
    auto& motor2 = motors.motor2;
    wait(ser, "Test the outputs with a polymeter.");
    motor2.set_speed(UINT8_MAX);
    motor2.set_direction(vla::direction_t::FORWARD);
    wait(ser, "motor2 full power forward");
    motor2.set_speed(UINT8_MAX / 4);
    wait(ser, "motor2 quarter power forward");
    motor2.set_speed(UINT8_MAX);
    motor2.set_direction(vla::direction_t::BACKWARD);
    wait(ser, "motor2 full power backward");
    motor2.set_speed(UINT8_MAX / 4);
    wait(ser, "motor2 quarter power backward");
    motor2.set_speed(0);
    wait(ser, "motor2 idle");
}

static void motor3(motor_driver &motors, decltype(get_serial_debug())& ser) {
    auto& motor3 = motors.motor3;
    wait(ser, "Test the outputs with a polymeter.");
    motor3.set_speed(UINT8_MAX);
    motor3.set_direction(vla::direction_t::FORWARD);
    wait(ser, "motor3 full power forward");
    motor3.set_speed(UINT8_MAX / 4);
    wait(ser, "motor3 quarter power forward");
    motor3.set_speed(UINT8_MAX);
    motor3.set_direction(vla::direction_t::BACKWARD);
    wait(ser, "motor3 full power backward");
    motor3.set_speed(UINT8_MAX / 4);
    wait(ser, "motor3 quarter power backward");
    motor3.set_speed(0);
    wait(ser, "motor3 idle");
}

int main()
{
    auto &ser = get_serial_debug();
    motor_driver motors;
    motor1(motors, ser);
    motor2(motors, ser);
    motor3(motors, ser);
    write(ser, "done");
    while(true);
}
