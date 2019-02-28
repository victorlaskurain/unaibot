#include <vla/serial.hpp>

namespace vla {
    // see serial_debug.cpp
    serial_9600_async serial_port;
    serial_9600_async& get_serial_async_debug()
    {
        return serial_port;
    }
}
