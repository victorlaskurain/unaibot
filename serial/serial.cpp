#include "serial.hpp"

namespace vla {
    serial_9600 serial_port;
    serial_9600& get_serial_debug()
    {
        return serial_port;
    }
}
