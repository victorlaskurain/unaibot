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
    template<typename registry_t>
    void set_bit(uint8_t bit, bool v)
    {
        registry_t::ref() = (registry_t::ref() & ~(1<<bit)) | (v<<bit);
    }
}

bool vla::pdu_handler::execute_read_single_coil(uint16_t address, bool *bit_value)
{
    const uint8_t registry = address / 8;
    const uint8_t bit      = address % 8;
    *bit_value = 0;
    switch (coil_record_number_t(registry)) {
    case AU_PORTD:
        if (bit > 1) {
            *bit_value = 1;
        }
        break;
    case AU_PORTB:
        if (bit < 6) {
            *bit_value = 1;
        }
        break;
    case AU_PORTC:
        if (bit != 7) {
            *bit_value = 1;
        }
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
        *bit_value = adc.is_enabled(adc_id_t(bit));
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

bool vla::pdu_handler::execute_write_single_coil(uint16_t address, bool v)
{
    const uint8_t registry = address / 8;
    const uint8_t bit      = address % 8;
    switch (coil_record_number_t(registry)) {
    case IOD_PORTD:
        set_bit<PORTD_t::ddr_t>(bit, v);
        break;
    case IOD_PORTB:
        set_bit<PORTB_t::ddr_t>(bit, v);
        break;
    case IOD_PORTC:
        set_bit<PORTC_t::ddr_t>(bit, v);
        break;
    case IOM_PORTC:
        // :TODO:
        break;
    case EIP_PORTD:
        if (!is_input_mode<PORTD_t>(bit)) {
            return false;
        }
        set_bit<PORTD_t>(bit, v);
        break;
    case EIP_PORTB:
        if (!is_input_mode<PORTB_t>(bit)) {
            return false;
        }
        set_bit<PORTB_t>(bit, v);
        break;
    case EIP_PORTC:
        if (!is_input_mode<PORTC_t>(bit)) {
            return false;
        }
        set_bit<PORTC_t>(bit, v);
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
    case PO_PORTD:
        if (is_input_mode<PORTD_t>(bit)) {
            return false;
        }
        set_bit<PORTD_t>(bit, v);
        break;
    case PO_PORTB:
        if (is_input_mode<PORTB_t>(bit)) {
            return false;
        }
        set_bit<PORTB_t>(bit, v);
        break;
    case PO_PORTC:
        if (is_input_mode<PORTC_t>(bit)) {
            return false;
        }
        set_bit<PORTC_t>(bit, v);
        break;
    default:
        return false;
    }
    return true;
}

void vla::pdu_handler::operator()()
{
    adc_set_value_msg_t adc_msg;
    ptxx_begin();
    while (true) {
        ptxx_wait(!in_q.empty());
        if (in_q.pop(buffer_msg)) {
            rtu_message rtu_msg{buffer_msg.data, buffer_msg.size};
            handle_indication(rtu_msg, rtu_msg);
            buffer_msg.size = rtu_msg.length;
            to_transmission_daemon_q.push(buffer_msg);
        } else if (in_q.pop(adc_msg)) {
            adc.set_value(adc_msg.id, adc_msg.value);
        }
    }
    ptxx_end();
}
