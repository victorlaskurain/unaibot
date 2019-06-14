#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON

#include "transmission_daemon_queue.hpp"
#include "pdu_handler_daemon_queue.hpp"
#include "pdu_handler_base.hpp"
#include "adc_daemon_queue.hpp"
#include <vla/protothreads.hpp>
#include <vla/timers.hpp>

namespace vla {

    class adc_state
    {
        uint8_t enabled = 0;
        uint16_t values[uint8_t(adc_id_t::ADC7) + 1] = {0};
    public:
        uint16_t get_value(adc_id_t id)
        {
            return values[uint8_t(id)];
        }
        bool is_enabled(adc_id_t id)
        {
            return enabled & (to_bit_mask(id));
        }
        void set_enabled(adc_id_t id, bool enable)
        {
            enabled = (enabled & ~to_bit_mask(id)) | (enable<<uint8_t(id));
        }
        void set_value(adc_id_t id, uint16_t v)
        {
            values[uint8_t(id)] = v;
        }
    };

    class pdu_handler : public ptxx_thread, public pdu_handler_base<pdu_handler>
    {
        pdu_handler_queue_t &in_q;
        transmission_queue_t &to_transmission_daemon_q;
        adc_queue_t &to_adc_q;
        buffer_msg_t buffer_msg = {};
        cm_unit_2B::timer_t pwm_timer2;
        cm_unit_2B          pwm2b;
        cm_unit_2A          pwm2a;
        adc_state adc;
        using handler_base = pdu_handler_base<pdu_handler>;
        // this friend declaration let us keep the inherited protected
        // member functions protected and callable from the parent.
        friend class pdu_handler_base<pdu_handler>;
    protected:
        bool execute_read_single_register(uint16_t address, uint16_t *word);
        bool execute_read_single_coil(uint16_t address, bool *bit_value);
        bool is_write_registers_supported()
        {
            return true;
        }
        bool execute_write_single_coil(uint16_t address, bool v);
        bool execute_write_single_register(uint16_t address, uint16_t v);
        bool is_read_coils_supported()
        {
            return true;
        }
        bool is_read_coils_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0000 && addr + count - 1 < 0x00D8);   // Output pin space
        }
        bool is_read_registers_supported()
        {
            return true;
        }
        bool is_read_registers_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0000 && addr + count < 0x0030);   // user data space
        }
        bool is_write_registers_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0000 && addr + count < 0x0018) || // counter space
                   (addr >= 0x0020 && addr + count < 0x0038) || // analog in space
                   (addr >= 0x0100 && addr + count < 0x0020);   // user data space
        }
        bool is_write_coils_supported()
        {
            return true;
        }
        bool is_write_coils_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0048 && addr + count < 0x00d8);   // Output pin space
        }
        bool is_write_single_coil_valid_data_address(uint16_t addr)
        {
            return is_write_coils_valid_data_address(addr, 1);
        }
    public:
        pdu_handler(
            rtu_address           addr,
            pdu_handler_queue_t  &in_q,
            transmission_queue_t &to_transmission_daemon_q,
            adc_queue_t          &to_adc_q):
            handler_base(addr), in_q(in_q),
            to_transmission_daemon_q(to_transmission_daemon_q),
            to_adc_q(to_adc_q),
            pwm_timer2(tc_mode::PHASE_CORRECT_PWM, clock_source_2::STOP),
            pwm2b(pwm_timer2, cm_mode::DISCONNECTED),
            pwm2a(pwm_timer2, cm_mode::DISCONNECTED)
        {}
        void operator()();
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
