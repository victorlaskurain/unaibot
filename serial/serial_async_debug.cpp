#include <vla/serial.hpp>

namespace vla {
    // see serial_debug.cpp
    serial_async serial_port;
    serial_async& get_serial_async_debug()
    {
        return serial_port;
    }
}
