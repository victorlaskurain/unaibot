#include <vla/registers.hpp>
#include <vla/timers.hpp>
#include <vla/notes.hpp>
#include <util/delay.h>

using namespace vla;

namespace vla {
    void test_function_f1();
};

int main()
{
    PORTB3_t::set_mode_output();
    timer_counter<TC2> t2;
    t2.set_mode(tc_mode::TOGGLE_ON_COMPARE_MATCH);
    t2.set_clock(clock_source_2::PRESCALE_1024);

    // t2.set_clock(clock_source_2::STOP);
    cm_unit<TC2, A> o2a(t2, cm_mode::TOGGLE_ON_COMPARE_MATCH);
    o2a.set_cm_handler(nullptr);
    play_note(o2a, note_a4, 0, 0);
    _delay_ms(1000);
    play_note(o2a, note_c4, 0, 0);
    _delay_ms(1000);
    play_note(o2a, note_0, 0, 0);
    _delay_ms(1000);
    play_note(o2a, note_a4, 0, 0);
    _delay_ms(1000);
    play_note(o2a, note_c4, 0, 0);
    _delay_ms(1000);
    play_note(o2a, note_0, 0, 0);
    while (1) {
    }
}
