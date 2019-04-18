#ifndef VLA_SERIAL_HEX_NUMBER_HPP
#define VLA_SERIAL_HEX_NUMBER_HPP

#include <stdint.h>

namespace vla {

    class hex_number
    {
        void to_hex(char *buff, uint8_t v);
        char buffer[1 + 2 * sizeof(uint16_t)];
    public:
        explicit hex_number(uint8_t v)
        {
            to_hex(buffer, v);
            buffer[2] = '\0';
        }
        explicit hex_number(uint16_t v)
        {
            to_hex(buffer    , v>>8);
            to_hex(buffer + 2, v&0xff);
            buffer[4] = '\0';
        }
        const char* data() const
        {
            return buffer;
        }
    };
}

#endif // VLA_SERIAL_HEX_NUMBER_HPP
