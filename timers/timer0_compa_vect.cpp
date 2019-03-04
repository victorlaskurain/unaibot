#include <vla/timers.hpp>
#include <avr/interrupt.h>

static vla::cm_handler_t cm_handler = nullptr;
static void *cm_handler_data        = nullptr;
namespace vla {
    template<>void cm_unit_0A::set_cm_handler_ptr(cm_handler_t f, void* data)
    {
        cm_handler      = f;
        cm_handler_data = data;
    }
}

ISR(TIMER0_COMPA_vect)
{
    // the API ensures cm_handler will NOT be null if this interrupt
    // is enabled.
    cm_handler(cm_handler_data);
}
