#include <vla/hex_number.hpp>

using namespace vla;

static const uint8_t hex_chr[] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f'};

void hex_number::to_hex(char *buff, uint8_t n)
{
    buff[0] = hex_chr[n>>4];
    buff[1] = hex_chr[n & 0x0f];
}
