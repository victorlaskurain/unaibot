#include <vla/serial.hpp>
#include <vla/command.hpp>
#include <stdlib.h>

using namespace vla;

static auto &ser = get_serial_debug();

enum {
    SUCCESS = 0,
    ERROR   = 1
};

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

struct c1
{
    inline uint8_t operator()(const char* str)
    {
        uint8_t v;
        if (skip_prefix(&str, "c1") &&
            read_uint8(&str, &v)) {
            write_line(ser, "c1");
            return SUCCESS;
        }
        return ERROR;
    }
};

struct c2
{
    inline uint8_t operator()(const char* str)
    {
        uint8_t v1, v2;
        if (skip_prefix(&str, "c2") &&
            read_uint8(&str, &v1)   &&
            read_uint8(&str, &v2)) {
            write_line(ser, "c2");
            return SUCCESS;
        }
        return ERROR;
    }
};

static uint8_t cf(const char*s)
{
    return ERROR;
}

int main()
{
    auto empty_dispatcher = make_command_distpatcher();
    empty_dispatcher("");
    c1 c1_instance;
    auto dispatcher = make_command_distpatcher(c1_instance, c2(), &cf);
    const static char *command_strings[] = {
        "kaixo",
        "c2 10 100", "c2 10 -100",
        "c3 10 10 -10", "c3 10 10 1000",
        "c1 10", "c1 130",
        0
    };
    for (uint8_t i = 0; command_strings[i]; ++i) {
        write_line(ser, command_strings[i]);
        auto status = dispatcher(command_strings[i]);
        if (status != SUCCESS) {
            write_line(ser, "unknown command");
        }
        write_line(ser, "");
        write_line(ser, "");
    }
    while(true);
}
