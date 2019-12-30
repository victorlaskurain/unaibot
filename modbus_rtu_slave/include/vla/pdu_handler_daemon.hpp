#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON

#include <vla/transmission_daemon_queue.hpp>
#include <vla/pdu_handler_daemon_queue.hpp>
#include <vla/pdu_handler_base.hpp>
#include <vla/adc_daemon_queue.hpp>
#include <vla/counters_daemon_queue.hpp>
#include <vla/protothreads.hpp>
#include <vla/timers.hpp>

namespace vla {

    class adc_state
    {
        uint8_t enabled = 0;
        uint16_t values[uint8_t(adc_id_t::ADC7) + 1] = {0};
    public:
        uint16_t get_value(adc_id_t id) const
        {
            return values[uint8_t(id)];
        }
        bool is_enabled(adc_id_t id) const
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

    class counters_state
    {
        uint32_t enabled = 0;
        uint16_t values[uint8_t(counter_id_t::COUNTER_MAX)] = {0};
        static_assert(sizeof(uint32_t) * 8 >= uint8_t(counter_id_t::COUNTER_MAX), "Too many counters.");
    public:
        uint16_t get_value(counter_id_t id) const
        {
            return values[uint8_t(id)];
        }
        bool is_enabled(counter_id_t id) const
        {
            return enabled & to_bit_mask(id);
        }
        void set_enabled(counter_id_t id, bool enable)
        {
            enabled = (enabled & ~to_bit_mask(id)) | to_bit_mask(id, enable);
        }
        void set_value(counter_id_t id, uint16_t v)
        {
            values[uint8_t(id)] = v;
        }
    };

    class register_values
    {
        register_values *next = nullptr;
        friend uint16_t* get(register_values *self, uint16_t i)
        {
            if (i < self->size) {
                return &self->data[i];
            }
            if (self->next) {
                return get(self->next, i - self->size);
            }
            return nullptr;

        }
        friend void append(register_values *&self, register_values *other)
        {
            if (self) {
                append(self->next, other);
            } else {
                self = other;
            }
        }
        friend void prepend(register_values *&self, register_values *other)
        {
            if (self) {
                other->next = self;
                self = other;
            } else {
                self = other;
            }
        }
    public:
        template<typename T>
        register_values(T *data)
            :register_values(reinterpret_cast<uint16_t*>(data), sizeof *data / 2)
        {}
        register_values(uint16_t *data, uint16_t size):data(data), size(size){}
        register_values() = default;
        uint16_t *data        = nullptr;
        uint16_t size         = 0;
    };

    class pdu_handler : public ptxx_thread, public pdu_handler_base<pdu_handler>
    {
        pdu_handler_queue_t  &in_q;
        transmission_queue_t &to_transmission_daemon_q;
        adc_queue_t          &to_adc_q;
        counters_queue_t     &to_counter_daemon_q;
        buffer_msg_t buffer_msg = {};
        cm_unit_2B::timer_t pwm_timer2;
        cm_unit_2B          pwm2b;
        cm_unit_2A          pwm2a;
        adc_state adc;
        counters_state counters;
        register_values *user_data       = nullptr;
        uint16_t         user_data_size  = 0;
        bool             program_running = true;
        using handler_base = pdu_handler_base<pdu_handler>;
        // this friend declaration let us keep the inherited protected
        // member functions protected and callable from the parent.
        friend class pdu_handler_base<pdu_handler>;
    protected:
        bool is_write_registers_supported()
        {
            return true;
        }
        bool is_read_coils_supported()
        {
            return true;
        }
        bool is_read_coils_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0000 && addr + count - 1 < 0x00D8) || // Output pin space
                   (addr == 0x0102 && count == 1);                  // running on/off address
        }
        bool is_read_registers_supported()
        {
            return true;
        }
        bool is_read_registers_valid_data_address(uint16_t addr, uint16_t count);
        bool is_write_registers_valid_data_address(uint16_t addr, uint16_t count);
        bool is_write_coils_supported()
        {
            return true;
        }
        bool is_write_coils_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0048 && addr + count < 0x00d8) ||  // Output pin space
                   (addr == 0x0100 && count == 1) ||             // save address
                   (addr == 0x0101 && count == 1) ||             // load address
                   (addr == 0x0102 && count == 1);               // running on/off address
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
            adc_queue_t          &to_adc_q,
            counters_queue_t     &to_counter_daemon_q
            ):
            handler_base(addr), in_q(in_q),
            to_transmission_daemon_q(to_transmission_daemon_q),
            to_adc_q(to_adc_q),
            to_counter_daemon_q(to_counter_daemon_q),
            pwm_timer2(tc_mode::PHASE_CORRECT_PWM, clock_source_2::STOP),
            pwm2b(pwm_timer2, cm_mode::DISCONNECTED),
            pwm2a(pwm_timer2, cm_mode::DISCONNECTED)
        {
            load_config();
        }
        void operator()();
        bool load_config();
        bool save_config();
        bool is_program_running()
        {
            return program_running;
        }
        void append_user_data(register_values *new_ud)
        {
            append(user_data, new_ud);
            user_data_size += new_ud->size;
        }
        void prepend_user_data(register_values *new_ud)
        {
            prepend(user_data, new_ud);
            user_data_size += new_ud->size;
        }
        void set_user_data(register_values *ud)
        {
            user_data = ud;
            user_data_size = ud ? ud->size : 0;
        }
        bool execute_read_single_coil(uint16_t address, bool *bit_value);
        bool execute_write_single_coil(uint16_t address, bool v);
        bool execute_read_single_register(uint16_t address, uint16_t *word);
        bool execute_write_single_register(uint16_t address, uint16_t v);
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
