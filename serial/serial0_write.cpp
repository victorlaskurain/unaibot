#include <avr/io.h>
#include <avr/interrupt.h>
#include <vla/serial.hpp>

namespace vla {
    write_cb_t on_serial_ready_to_send = 0;
    void*      write_buffer            = 0;
}

ISR(USART_UDRE_vect)
{
    vla::on_serial_ready_to_send(vla::write_buffer);
}
