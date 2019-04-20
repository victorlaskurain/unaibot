#include "pdu_handler_daemon.hpp"
#include <util/crc16.h>

void vla::pdu_handler::operator()()
{
    ptxx_begin();
    while (true) {
        ptxx_wait(in_q.pop(msg));
        msg.data[0] |= 0x80;
        msg.data[1]  = 0x02;
        uint16_t crc = 0xffff;
        crc = _crc16_update(crc, msg.data[0]);
        crc = _crc16_update(crc, msg.data[1]);
        msg.data[2] = crc;
        msg.data[3] = crc>>8;
        msg.size = 4;
        out_q.push(msg);
    }
    ptxx_end();
}
