#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON

#include "daemon_queues.hpp"
#include "pdu_handler_base.hpp"
#include <vla/protothreads.hpp>

namespace vla {

    class pdu_handler : public ptxx_thread, public pdu_handler_base<pdu_handler>
    {
        pdu_handler_queue_t &in_q;
        transmission_queue_t &out_q;
        buffer_msg_t msg = {};
        using handler_base = pdu_handler_base<pdu_handler>;
        // this friend declaration let us keep the inherited protected
        // member functions protected and callable from the parent.
        friend class pdu_handler_base<pdu_handler>;
    protected:
        bool execute_read_coils(uint16_t address, uint16_t bit_count, uint8_t *byte_count, uint8_t *bytes)
        {
            *byte_count = bit_count / 8 + int(bit_count % 8 > 0);
            for (uint8_t i = 0; i < *byte_count; ++i) {
                bytes[i] = i + 0x10;
            }
            return true;
        }

        bool execute_write_coils(uint16_t address, uint8_t *bits, uint8_t bit_count)
        {
            return true;
        }
        bool execute_write_single_coil(uint16_t address, bool v)
        {
            return true;
        }
        bool is_read_coils_supported()
        {
            return true;
        }
        bool is_read_coils_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0000 && addr + count < 0x0060) || // fixed configuration space
                   (addr >= 0x0100 && addr + count < 0x0160) || // I/O parameter space
                   (addr >= 0x0200 && addr + count < 0x0218) || // Input pin space
                   (addr >= 0x0300 && addr + count < 0x0318);   // Output pin space
        }
        bool is_write_coils_supported()
        {
            return true;
        }
        bool is_write_single_coil_supported()
        {
            return true;
        }
        bool is_write_coils_valid_data_address(uint16_t addr, uint16_t count)
        {
            return (addr >= 0x0100 && addr + count < 0x0160) || // I/O parameter space
                   (addr >= 0x0200 && addr + count < 0x0218) || // Input pin space
                   (addr >= 0x0300 && addr + count < 0x0318);   // Output pin space
        }
        bool is_write_single_coil_valid_data_address(uint16_t addr)
        {
            return (addr >= 0x0100 && addr < 0x0160) || // I/O parameter space
                   (addr >= 0x0200 && addr < 0x0218) || // Input pin space
                   (addr >= 0x0300 && addr < 0x0318);   // Output pin space
        }
    public:
        pdu_handler(
            rtu_address addr,
            pdu_handler_queue_t &in_q,
            transmission_queue_t &out_q):
            handler_base(addr), in_q(in_q), out_q(out_q) {}
        void operator()();
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_DAEMON
