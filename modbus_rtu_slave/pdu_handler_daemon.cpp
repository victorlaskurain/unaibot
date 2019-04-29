#include "pdu_handler_daemon.hpp"
#include <util/crc16.h>

void vla::pdu_handler::operator()()
{
    ptxx_begin();
    while (true) {
        ptxx_wait(in_q.pop(msg));
        rtu_message rtu_msg{msg.data, msg.size};
        handle_indication(rtu_msg, rtu_msg);
        msg.size = rtu_msg.length;
        out_q.push(msg);
    }
    ptxx_end();
}
