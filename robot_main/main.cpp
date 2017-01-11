#include <vla/motor_driver.hpp>
#include <vla/serial.hpp>
#include <vla/command.hpp>
#include <stdlib.h>

using namespace vla;

static const bool skip_prefix(const char **strptr, const char *prefix)
{
    const char *str = *strptr;
    while (*prefix == *str) {
        ++prefix;
        ++str;
    }
    if (!*prefix) {
        *strptr = str;
        return true;
    }
    return false;
}

static bool read_uint8(const char **strptr, uint8_t *v)
{
    char *end;
    long number = strtol(*strptr, &end, 0);
    if (*strptr == end) {
        return false;
    }
    if (number < 0 || number > UINT8_MAX) {
        return false;
    }
    *v = number;
    *strptr = end;
    return true;
}

static bool read_direction(const char **strptr, direction_t *v)
{
    char *end;
    long number = strtol(*strptr, &end, 0);
    if (*strptr == end) {
        return false;
    }
    if (number < uint8_t(direction_t::FIRST) ||
        number > uint8_t(direction_t::LAST)) {
        return false;
    }
    *v = direction_t(number);
    *strptr = end;
    return true;
}

enum {
    SUCCESS = 0,
    ERROR   = 1
};
/**
 * Examples:
 * Motor 2 and motor 3 at about halve speed forward
 * SPEEDS2 1 128 1 128
 * Motor 2 halve backward, motor 3 full forward
 * SPEEDS2 2 128 1 255
 * Stop both motors
 * SPEEDS2 0 0 0 0
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
            read_uint8    (&str, &m2_speed)     &&
            read_direction(&str, &m3_direction) &&
            read_uint8    (&str, &m3_speed)) {
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

static const char hex_chr[] = {'0', '1', '2', '3',
                               '4', '5', '6', '7',
                               '8', '9', 'A', 'B',
                               'C', 'D', 'E', 'F'};
static const char* hex(uint8_t n) {
    static char hex_buffer[] = "0x00";
    hex_buffer[2] = hex_chr[n>>4];
    hex_buffer[3] = hex_chr[n & 0xf];
    return hex_buffer;
}

int main()
{
    const auto CMD_BUFFER_SIZE = 64;
    uint8_t cmd_buffer[CMD_BUFFER_SIZE], err_code;
    serial_9600 ser;
    motor_driver motors;
    auto dispatch = make_command_dispatcher(set_speeds_cmd(motors));
    write_line(ser, "READY");
    while(true) {
        read_line(ser, cmd_buffer, CMD_BUFFER_SIZE, '\r');
        err_code = dispatch(reinterpret_cast<const char*>(&cmd_buffer[0]));
        if (SUCCESS == err_code) {
            write_line(ser, "OK");
        } else {
            write(ser, "ERROR ");
            write_line(ser, hex(err_code));
        }
    }
}
