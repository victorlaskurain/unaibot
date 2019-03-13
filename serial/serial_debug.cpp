#include <vla/serial.hpp>

namespace vla {
    // serial_async_debug.cpp defines the same global variable so that
    // a link error (multiple definitions of vla::serial_port)
    // prevents including both, sync and async debug lines in the
    // final executable.
    serial_sync serial_port;
    serial_sync & get_serial_debug()
    {
        return serial_port;
    }
}
