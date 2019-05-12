#include "pdu_handler_daemon.hpp"
#include <vla/registers.hpp>
#include <util/crc16.h>

namespace vla {
    enum coil_record_number_t {
        AU_PORTD,
        AU_PORTB,
        AU_PORTC,
        AI_PORTD,
        AI_PORTB,
        AI_PORTC,
        AO_PORTD,
        AO_PORTB,
        AO_PORTC,
        IOD_PORTD,
        IOD_PORTB,
        IOD_PORTC,
        IOM_PORTD,
        IOM_PORTB,
        IOM_PORTC,
        EIP_PORTD,
        EIP_PORTB,
        EIP_PORTC,
        EC_PORTD,
        EC_PORTB,
        EC_PORTC,
        PI_PORTD,
        PI_PORTB,
        PI_PORTC,
        PO_PORTD,
        PO_PORTB,
        PO_PORTC,
    };
    template<typename registry_t>
    bool get_bit(uint8_t bit)
    {
        return registry_t::ref() & (1<<bit);
    }
    template<typename port_t>
    bool is_input_mode(uint8_t bit)
    {
        return !get_bit<typename port_t::ddr_t>(bit);
    }
}

bool vla::pdu_handler::execute_read_single_coil(uint16_t address, bool *bit_value)
{
    const uint8_t registry = address / 8;
    const uint8_t bit      = address % 8;
    *bit_value = 0;
    switch (coil_record_number_t(registry)) {
    case AU_PORTD:
        if (bit <= 1) {
            break;
        }
    case AU_PORTB:
    case AU_PORTC:
        *bit_value = 1;
        break;
    case AI_PORTC:
        *bit_value = 1;
        break;
    case AI_PORTB:
    case AI_PORTD:
        break;
    case AO_PORTC:
    case AO_PORTB:
    case AO_PORTD:
        break;
    case IOD_PORTD:
        *bit_value = get_bit<PORTD_t::ddr_t>(bit);
        break;
    case IOD_PORTB:
        *bit_value = get_bit<PORTB_t::ddr_t>(bit);
        break;
    case IOD_PORTC:
        *bit_value = get_bit<PORTC_t::ddr_t>(bit);
        break;
    case IOM_PORTD:
        // no analog mode supported
        break;
    case IOM_PORTB:
        // no analog mode supported
        break;
    case IOM_PORTC:
        // :TODO:
        break;
    case EIP_PORTD:
        *bit_value = is_input_mode<PORTD_t>(bit) && get_bit<PORTD_t>(bit);
        break;
    case EIP_PORTB:
        *bit_value = is_input_mode<PORTB_t>(bit) && get_bit<PORTB_t>(bit);
        break;
    case EIP_PORTC:
        *bit_value = is_input_mode<PORTC_t>(bit) && get_bit<PORTC_t>(bit);
        break;
    case EC_PORTD:
        // :TODO:
        break;
    case EC_PORTB:
        // :TODO:
        break;
    case EC_PORTC:
        // :TODO:
        break;
    case PI_PORTD:
        *bit_value = get_bit<PORTD_t::pin_t>(bit);
        break;
    case PI_PORTB:
        *bit_value = get_bit<PORTB_t::pin_t>(bit);
        break;
    case PI_PORTC:
        *bit_value = get_bit<PORTC_t::pin_t>(bit);
        break;
    case PO_PORTD:
        *bit_value = get_bit<PORTD_t>(bit);
        break;
    case PO_PORTB:
        *bit_value = get_bit<PORTB_t>(bit);
        break;
    case PO_PORTC:
        *bit_value = get_bit<PORTC_t>(bit);
        break;
    default:
        return false;
    }
    return true;
}

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
