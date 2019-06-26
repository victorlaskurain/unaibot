#include "pdu_handler_daemon.hpp"
#include "counters_daemon_queue.hpp"
#include <vla/registers.hpp>
#include <vla/timers.hpp>
#include <util/crc16.h>

namespace vla {
    enum coil_record_number_t {
        AU_PORTB,
        AU_PORTC,
        AU_PORTD,
        AI_PORTB,
        AI_PORTC,
        AI_PORTD,
        AO_PORTB,
        AO_PORTC,
        AO_PORTD,
        IOD_PORTB,
        IOD_PORTC,
        IOD_PORTD,
        IOM_PORTB,
        IOM_PORTC,
        IOM_PORTD,
        EIP_PORTB,
        EIP_PORTC,
        EIP_PORTD,
        EC_PORTB,
        EC_PORTC,
        EC_PORTD,
        PI_PORTB,
        PI_PORTC,
        PI_PORTD,
        PO_PORTB,
        PO_PORTC,
        PO_PORTD,
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
        if (bit != 7 && bit != 6) {
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
    case EC_PORTB:
        *bit_value = counters.is_enabled(counter_id_t( 0 + bit));
        break;
    case EC_PORTC:
        *bit_value = counters.is_enabled(counter_id_t( 8 + bit));
        break;
    case EC_PORTD:
        *bit_value = counters.is_enabled(counter_id_t(16 + bit));
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
        if (v) {
            to_adc_q.push(adc_msg_t{adc_id_t(bit), in_q});
        } else {
            to_adc_q.push(adc_msg_t::msg_disable(adc_id_t(bit)));
            adc.set_enabled(adc_id_t(bit), 0);
        }
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
    case EC_PORTB:
    case EC_PORTC:
    {
        uint8_t counter_offset;
        switch (coil_record_number_t(registry)) {
        case EC_PORTB:
            counter_offset =  0 + bit;
            break;
        case EC_PORTC:
            counter_offset =  8 + bit;
            break;
        case EC_PORTD:
            counter_offset = 16 + bit;
            break;
        default:
            // never happens, just keeping the compiler from spitting out warnings
            break;
        }
        const counter_id_t counter_id = counter_id_t(counter_offset);
        if (v) {
            to_counter_daemon_q.push(counters_daemon_msg_t{counter_id, in_q});
        } else {
            to_counter_daemon_q.push(counters_daemon_msg_t::msg_disable(counter_id));
            counters.set_enabled(counter_id, 0);
        }
        break;
    }
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

constexpr uint16_t TC2A_CONFIG_ADDR   = 0x0008;
constexpr uint16_t TC2B_CONFIG_ADDR   = 0x0009;
constexpr uint16_t COUNTER_ADDR_BEGIN = 0x000a;
constexpr uint16_t COUNTER_ADDR_END   =
COUNTER_ADDR_BEGIN + uint8_t(vla::counter_id_t::COUNTER_MAX);

static_assert(uint16_t(vla::adc_id_t::ADC_MAX) == TC2A_CONFIG_ADDR, "Bad TC2_CONFIG_ADDR");

/**
 * Utility class to parse/generate words containing TC configuration
 * messages.
 */
class tc2_config
{
    const uint8_t _duty;
    const uint8_t _clock;
    const uint8_t _mode;
public:
    tc2_config(uint8_t d, uint8_t c, uint8_t m):
        _duty(d), _clock(c), _mode(m){}
    tc2_config(uint8_t d, vla::clock_source_2 c, vla::cm_mode m)
        :tc2_config(uint8_t(d), uint8_t(c), uint8_t(m)){}
    tc2_config(uint16_t v):
        _duty(v & 0xff), _clock((v >>  8) & 0x7), _mode((v >> 12) & 0x3){}
    uint16_t as_word() const
    {
        return _duty | (_clock << 8) | (_mode << 12);
    }
    bool is_valid() const
    {
        return !(_mode == 1 || _clock > 7);
    }
    uint8_t duty() const
    {
        return _duty;
    }
    vla::cm_mode mode() const
    {
        return vla::cm_mode(_mode);
    }
    vla::clock_source_2 clock() const
    {
        return vla::clock_source_2(_clock);
    }
};

bool vla::pdu_handler::execute_write_single_register(uint16_t address, uint16_t v)
{
    if (TC2A_CONFIG_ADDR == address) {
        // 0x277f -> non inverting pcpwm, 1024 prescaler, half duty
        // 0x270c -> non inverting pcpwm, servo center
        const tc2_config conf{v};
        if (!conf.is_valid()) {
            return false;
        }
        if (conf.mode() != vla::cm_mode::DISCONNECTED) {
            vla::PORTB3_t::set_mode_output();
        }
        pwm2a.set_mode(conf.mode());
        pwm2a.set_output_compare(conf.duty());
        pwm2a.set_clock(conf.clock());
        return true;
    }
    if (TC2B_CONFIG_ADDR == address) {
        // 0x277f -> non inverting pcpwm, 1024 prescaler, half duty
        // 0x270c -> non inverting pcpwm, servo center
        const tc2_config conf{v};
        if (!conf.is_valid()) {
            return false;
        }
        if (conf.mode() != vla::cm_mode::DISCONNECTED) {
            vla::PORTD3_t::set_mode_output();
        }
        pwm2b.set_mode(conf.mode());
        pwm2b.set_output_compare(conf.duty());
        pwm2b.set_clock(conf.clock());
        return true;
    }
    if (address >= COUNTER_ADDR_BEGIN && address < COUNTER_ADDR_END) {
        counter_id_t cid = counter_id_t(address - COUNTER_ADDR_BEGIN);
        counters.set_value(cid, v);
        to_counter_daemon_q.push(counters_daemon_msg_t{cid, v});
        return true;
    }
    return false;
}

bool vla::pdu_handler::execute_read_single_register(uint16_t address, uint16_t *word)
{
    if (address < uint8_t(adc_id_t::ADC_MAX)) {
        *word = adc.get_value(adc_id_t(address));
        return true;
    }
    if (TC2A_CONFIG_ADDR == address) {
        *word = tc2_config(
            pwm2a.get_output_compare(),
            pwm2a.get_clock(),
            pwm2a.get_mode()).as_word();
        return true;
    }
    if (TC2B_CONFIG_ADDR == address) {
        *word = tc2_config(
            pwm2b.get_output_compare(),
            pwm2b.get_clock(),
            pwm2b.get_mode()).as_word();
        return true;
    }
    if (address >= COUNTER_ADDR_BEGIN && address < COUNTER_ADDR_END) {
        *word = counters.get_value(counter_id_t(address - COUNTER_ADDR_BEGIN));
        return true;
    }
    *word = 0;
    return true;
}

void vla::pdu_handler::operator()()
{
    adc_set_value_msg_t      adc_msg;
    counters_set_value_msg_t counters_msg;
    ptxx_begin();
    while (true) {
        ptxx_wait(!in_q.empty());
        if (in_q.pop(buffer_msg)) {
            rtu_message rtu_msg{buffer_msg.data, buffer_msg.size};
            handle_indication(rtu_msg, rtu_msg);
            buffer_msg.size = rtu_msg.length;
            to_transmission_daemon_q.push(buffer_msg);
        } else if (in_q.pop(adc_msg)) {
            adc.set_enabled(adc_msg.id, true);
            adc.set_value(adc_msg.id, adc_msg.value);
        } else if (in_q.pop(counters_msg)) {
            counters.set_enabled(counters_msg.id, true);
            counters.set_value(counters_msg.id, counters_msg.value);
        }
    }
    ptxx_end();
}
