#include <vla/motor_driver.hpp>
#include <vla/serial.hpp>
#include <vla/command.hpp>

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

static bool read_hex_char(char c, uint8_t *v) {
    if (c >= '0' && c <= '9') {
        *v += (c - '0');
        return true;
    }
    if (c >= 'a' && c <= 'f') {
        *v += (c - 'a') + 10;
        return true;
    }
    if (c >= 'A' && c <= 'F') {
        *v += (c - 'A') + 10;
        return true;
    }
    return false;
}

static bool read_uint8_hex(const char **strptr, uint8_t *v)
{
    const char *str = *strptr;
    while (*str == ' ') {
        ++str;
    }
    if (*str != '0') {
        return false;
    }
    ++str;
    if (*str != 'x') {
        return false;
    }
    ++str;
    *v = 0;
    if (!read_hex_char(*str, v)) {
        return false;
    }
    *v <<= 4;
    ++str;
    if (!read_hex_char(*str, v)) {
        return false;
    }
    ++str;
    *strptr = str;
    return true;
}

static bool read_direction(const char **strptr, direction_t *v)
{
    uint8_t number = 0;
    if (!read_uint8_hex(strptr, &number)) {
        return false;
    }
    if (number < uint8_t(direction_t::FIRST) ||
        number > uint8_t(direction_t::LAST)) {
        return false;
    }
    *v = direction_t(number);
    return true;
}

enum {
    SUCCESS = 0,
    ERROR   = 1
};
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

static const char hex_chr[] = {'0', '1', '2', '3',
                               '4', '5', '6', '7',
                               '8', '9', 'a', 'b',
                               'c', 'd', 'e', 'f'};
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
