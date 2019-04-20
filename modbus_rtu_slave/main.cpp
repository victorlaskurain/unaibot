#include "pdu_handler_daemon.hpp"
#include "daemon_queues.hpp"
#include "transmission_daemon.hpp"

#include <vla/registers.hpp>
#include <vla/serial.hpp>
#include <vla/timers.hpp>
#include <vla/rt_clock.hpp>
#include <util/delay.h>

using namespace vla;

int main(int argc, char **argv)
{
    PORTB5_t::set_mode_output();
    PORTB5_t::set();
    _delay_ms(1000);
    PORTB5_t::clear();
    _delay_ms(1000);
    tick_0a_20khz tick;
    rt_clock clock{tick};
    transmission_queue_t tr_q;
    pdu_handler_queue_t  pdu_q;
    transmission tr{clock, tr_q, pdu_q};
    pdu_handler pduh{pdu_q, tr_q};
    while(true) {
        tr();
        pduh();
    }
}
