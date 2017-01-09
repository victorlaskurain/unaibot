#include <avr/io.h>
#include <avr/interrupt.h>
#include <vla/serial.hpp>

namespace vla {
    read_cb_t  on_serial_received = 0;
    void*      read_buffer        = 0;
}

ISR(USART_RX_vect)
{
    vla::on_serial_received(vla::read_buffer, UDR0);
}
