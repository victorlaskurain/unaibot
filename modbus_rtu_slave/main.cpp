#include <vla/pdu_handler_daemon.hpp>
#include <vla/transmission_daemon.hpp>
#include <vla/adc_daemon.hpp>
#include <vla/counters_daemon.hpp>
#include <vla/main_custom.hpp>

#include <vla/registers.hpp>
#include <vla/serial.hpp>
#include <vla/timers.hpp>
#include <vla/rt_clock.hpp>
#include <util/delay.h>

using namespace vla;

static uint32_t max(uint32_t a, uint32_t b)
{
    if (b < a) {
        return a;
    }
    return b;
}

int main(int argc, char **argv)
{
    PORTB5_t::set_mode_output();
    PORTB5_t::set();
    _delay_ms(1000);
    PORTB5_t::clear();
    PORTB5_t::set_mode_input();
    _delay_ms(1000);
    tick_0a_20khz tick;
    rt_clock clock{tick};
    transmission_queue_t tr_q;
    pdu_handler_queue_t  pdu_q;
    adc_queue_t          adc_q;
    counters_queue_t     counter_daemon_q;
    transmission    tr{clock, tr_q, pdu_q};
    pdu_handler     pduh{rtu_address{118}, pdu_q, tr_q, adc_q, counter_daemon_q};
    adc_daemon      adc{adc_q};
    counters_daemon counters{clock, counter_daemon_q};
    struct user_data_t
    {
        timestamp_us ts1{0};
        uint32_t     all_max{0};
        uint32_t     loop_max{0};
        uint32_t     tr_max{0};
        uint32_t     pdu_max{0};
        uint32_t     adc_max{0};
        uint32_t     counters_max{0};
    } user_data;
    static_assert(sizeof(user_data_t) == 28, "Unexpected user_data_t size");
    timestamp_us &ts1 = user_data.ts1,
                  ts2{0},
                  ts3{0},
                  ts4{0},
                  ts5{0},
                  ts6{0};
    uint32_t &all_max = user_data.all_max;
    uint32_t &loop_max = user_data.loop_max;
    uint32_t &tr_max = user_data.tr_max;
    uint32_t &pdu_max = user_data.pdu_max;
    uint32_t &adc_max = user_data.adc_max;
    uint32_t &counters_max = user_data.counters_max;
    register_values rv{&user_data};
    pduh.set_user_data(&rv);
    // need a call to the threads in order to proto_init to work
    // properly
    tr(); pduh(); adc(); counters();
    init(pduh);
    while(true) {
        ts1 = clock.get_current_time();
        tr();
        ts2 = clock.get_current_time();
        pduh();
        ts3 = clock.get_current_time();
        adc();
        ts4 = clock.get_current_time();
        counters();
        ts5 = clock.get_current_time();
        if (pduh.is_program_running()) {
            loop(pduh, clock);
        }
        ts6 = clock.get_current_time();
        tr_max       = max(tr_max      , (ts2 - ts1).us);
        pdu_max      = max(pdu_max     , (ts3 - ts2).us);
        adc_max      = max(adc_max     , (ts4 - ts3).us);
        counters_max = max(counters_max, (ts5 - ts4).us);
        loop_max     = max(loop_max    , (ts6 - ts5).us);
        all_max      = max(all_max     , (ts6 - ts1).us);
    }
}
