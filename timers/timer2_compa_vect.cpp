#include <vla/timers.hpp>
#include <avr/interrupt.h>

using namespace vla;

cm_handler_t cm_unit_traits<tc_id::TC2, cm_id::A>::cm_handler = nullptr;

ISR(TIMER2_COMPA_vect)
{
    cm_unit_traits<tc_id::TC2, cm_id::A>::cm_handler();
}
