#ifndef VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_BASE
#define VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_BASE

#include "daemon_queues.hpp"
#include <vla/protothreads.hpp>
#include <util/crc16.h>
#include <string.h>

namespace vla {

    inline uint16_t flip_endianess(uint16_t v)
    {
        union {
            uint16_t ui16;
            uint8_t  ui8[2];
        } a, b;
        a.ui16 = v;
        b.ui8[0] = a.ui8[1];
        b.ui8[1] = a.ui8[0];
        return b.ui16;
    }

    struct rtu_address
    {
        uint8_t address;
        explicit rtu_address(uint8_t a):address(a){}
        bool operator==(rtu_address other)
        {
            return address == other.address;
        }
        bool operator!=(rtu_address other)
        {
            return address != other.address;
        }
    };

    enum class rtu_function_code : uint8_t
    {
        // physical discrete input
        READ_DISCRETE_INPUT           = 0x02,
        // internal bits or physical bits
        READ_COILS                    = 0x01,
        WRITE_SINGLE_COIL             = 0x05,
        WRITE_COILS                   = 0x0f,
        // physical input registers
        READ_INPUT_REGISTER           = 0x04,
        // internal registers or physical output registers
        READ_HOLDING_REGISTERS        = 0x03,
        WRITE_SINGLE_REGISTER         = 0x06,
        WRITE_MULTIPLE_REGISTERS      = 0x10,
        READ_WRITE_MULTIPLE_REGISTERS = 0x17,
        MASK_WRITE_REGISTER           = 0x16,
        READ_FIFO_QUEUE               = 0x18,
        // file records
        READ_FILE_RECORD              = 0x14,
        WRITE_FILE_RECORD             = 0x15,
        // diagnostics
        READ_EXCEPTION_STATUS         = 0x07,
        DIAGNOSTIC                    = 0x08,
        GET_COM_EVENT_COUNTER         = 0x0b,
        GET_COM_EVENT_LOG             = 0x0c,
        REPORT_SERVER_ID              = 0x11,
        READ_DEVICE_IDENTIFICATION    = 0x2b
    };

    enum class rtu_exception_code : uint8_t
    {
        ILLEGAL_FUNCTION                 = 0x01,
        ILLEGAL_DATA_ADDRESS             = 0x02,
        ILLEGAL_DATA_VALUE               = 0x03,
        SERVER_DEVICE_FAILURE            = 0x04,
        ACKNOLEDGE                       = 0x05,
        SERVER_DEVICE_BUSY               = 0x06,
        MEMORY_PARITY_ERROR              = 0x08,
        GATEWAY_PATH_UNAVAILABLE         = 0x0a,
        GATEWAY_TARGET_FAILED_TO_RESPOND = 0x0b
    };

    struct rtu_message
    {
        uint8_t *buffer;
        uint8_t length;
        rtu_message(uint8_t *b, uint8_t l):buffer{b}, length{l}{}
        rtu_address address() const
        {
            return rtu_address{buffer[0]};
        }
        rtu_function_code function_code() const
        {
            return static_cast<rtu_function_code>(buffer[1]);
        }
    };

    template<class pdu_handler>
    class pdu_handler_base
    {
    public:
        pdu_handler_base(rtu_address address):address(address) {}
        // the message buffer must be large enough to hold any modbus
        // message, that means at least 256 bytes. Reply and
        // indication can be the same data structure.
        void handle_indication(const rtu_message &indication, rtu_message &reply)
        {
            if (self().is_address_valid(indication.address())) {
                self().execute_function(indication, reply);
                self().append_crc(reply);
            } else {
                reply.length = 0;
            }
        }
        pdu_handler& self()
        {
            return *static_cast<pdu_handler*>(this);
        }
    protected:
        bool execute_read_coils(uint16_t address, uint16_t bit_count, uint8_t *bytes)
        {
            for (uint16_t i = 0; i < bit_count / 8 + uint16_t(bool(bit_count % 8)); ++i) {
                bytes[i] = 0;
            }
            for (uint16_t i = 0; i < bit_count; ++i) {
                bool bit_value = 0;
                if (!self().execute_read_single_coil(address + i, &bit_value)) {
                    return false;
                }
                bytes[i / 8] |= uint8_t(bit_value) << (i % 8);
            }
            return true;
        }
        bool execute_read_single_coil(uint16_t address, bool *bit_value)
        {
            return false;
        }
        bool execute_read_registers(uint16_t address, uint16_t register_count, uint8_t *byte_count, uint16_t *words)
        {
            return false;
        }
        bool execute_write_coils(uint16_t address, uint8_t *bits, uint16_t bit_count)
        {
            return false;
        }
        bool execute_write_registers(uint16_t address, uint16_t *words, uint8_t word_count)
        {
            return false;
        }
        bool execute_write_single_coil(uint16_t address, bool vparam)
        {
            uint8_t v = vparam ? 1 : 0;
            return execute_write_coils(address, &v, 1);
        }
        bool is_read_coils_supported()
        {
            return false;
        }
        bool is_read_coils_valid_data_address(uint16_t address, uint16_t bit_count)
        {
            return true;
        }
        bool is_read_registers_supported()
        {
            return false;
        }
        bool is_read_registers_valid_data_address(uint16_t address, uint16_t register_count)
        {
            return true;
        }
        bool is_write_coils_valid_data_address(uint16_t address, uint16_t bit_count)
        {
            return true;
        }
        bool is_write_registers_valid_data_address(uint16_t addr, uint16_t register_count)
        {
            return true;
        }
        bool is_write_single_coil_valid_data_address(uint16_t addr)
        {
            return self().is_write_coils_valid_data_address(addr, 1);
        }
        bool is_write_coils_supported()
        {
            return false;
        }
        bool is_write_registers_supported()
        {
            return false;
        }
        bool is_write_single_coil_supported()
        {
            return self().is_write_coils_supported();
        }
        bool is_write_single_register_supported()
        {
            return self().is_write_registers_supported();
        }
    private:
        static constexpr int WRITE_COILS_REPLY_LENGTH      = 6;
        static constexpr int WRITE_REGISTERS_REPLY_LENGTH  = 6;
        static constexpr int READ_WRITE_COILS_MAX_COILS    = 0x07b0;
        static constexpr int WRITE_REGISTERS_MAX_REGISTERS = 0x07b;
        static constexpr int READ_REGISTERS_MAX_REGISTERS  = 0x007d;
        rtu_address address;
        void append_crc(rtu_message &reply)
        {
            uint16_t crc = 0xffff;
            for (auto i = 0; i < reply.length; ++i) {
                crc = _crc16_update(crc, reply.buffer[i]);
            }
            reply.buffer[reply.length    ] = crc;
            reply.buffer[reply.length + 1] = crc>>8;
            reply.length += 2;
        }
        bool is_address_valid(rtu_address addr)
        {
            if (addr != address && addr != rtu_address{0}) {
                return false;
            }
            return true;
        }
        bool is_read_coils_valid_data_value(uint16_t bit_count)
        {
            return bit_count > 0 && bit_count <= READ_WRITE_COILS_MAX_COILS;
        }
        bool is_read_registers_valid_data_value(uint16_t register_count)
        {
            return register_count > 0 && register_count <= READ_REGISTERS_MAX_REGISTERS;
        }
        bool is_write_coils_valid_data_value(uint16_t bit_count, uint16_t byte_count)
        {
            return bit_count > 0 && bit_count <= READ_WRITE_COILS_MAX_COILS &&
                (bit_count / 8 + int(bit_count % 8 > 0)) == byte_count;
        }
        bool is_write_registers_valid_data_value(uint16_t register_count, uint8_t byte_count)
        {
            return register_count > 0 && register_count <= WRITE_REGISTERS_MAX_REGISTERS &&
                byte_count == register_count * 2;
        }
        bool is_write_single_coil_valid_data_value(uint16_t v)
        {
            return 0x0000 == v || 0xff00 == v;
        }
        void execute_function(const rtu_message &indication, rtu_message &reply)
        {
            switch (indication.function_code()) {
            case rtu_function_code::READ_COILS:
                execute_read_coils(indication, reply);
                break;
            case rtu_function_code::WRITE_SINGLE_COIL:
                execute_write_single_coil(indication, reply);
                break;
            case rtu_function_code::WRITE_COILS:
                execute_write_coils(indication, reply);
                break;
            case rtu_function_code::READ_HOLDING_REGISTERS:
                execute_read_registers(indication, reply);
                break;
            case rtu_function_code::WRITE_MULTIPLE_REGISTERS:
                execute_write_registers(indication, reply);
                break;
            case rtu_function_code::WRITE_SINGLE_REGISTER:
                execute_write_single_register(indication, reply);
                break;
            default:
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
        }
        void execute_read_coils(const rtu_message &indication, rtu_message &reply)
        {
            uint16_t address   = flip_endianess(*(uint16_t*)&indication.buffer[2]),
                     bit_count = flip_endianess(*(uint16_t*)&indication.buffer[4]);
            if (!self().is_read_coils_supported()) {
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
            if (!self().is_read_coils_valid_data_address(address, bit_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_ADDRESS, indication, reply);
                return;
            }
            if (!self().is_read_coils_valid_data_value(bit_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_VALUE, indication, reply);
                return;
            }
            if (!self().execute_read_coils(address, bit_count, &reply.buffer[3])) {
                make_exception_reply(rtu_exception_code::SERVER_DEVICE_FAILURE, indication, reply);
                return;
            }
            reply.buffer[2] = bit_count / 8 + uint8_t(bool(bit_count % 8));
            // copy address and function code
            reply.buffer[0] = indication.buffer[0];
            reply.buffer[1] = indication.buffer[1];
            reply.length    = reply.buffer[2] + 3;
        }
        void execute_read_registers(const rtu_message &indication, rtu_message &reply)
        {
            uint16_t address        = flip_endianess(*(uint16_t*)&indication.buffer[2]),
                     register_count = flip_endianess(*(uint16_t*)&indication.buffer[4]);
            if (!self().is_read_registers_supported()) {
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
            if (!self().is_read_registers_valid_data_address(address, register_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_ADDRESS, indication, reply);
                return;
            }
            if (!self().is_read_registers_valid_data_value(register_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_VALUE, indication, reply);
                return;
            }
            uint16_t *words = (uint16_t*)&reply.buffer[3];
            if (self().execute_read_registers(address, register_count, words)) {
                for (uint16_t i = 0; i < register_count; ++i) {
                    words[i] = flip_endianess(words[i]);
                }
                reply.buffer[0] = indication.buffer[0];
                reply.buffer[1] = indication.buffer[1];
                reply.buffer[2] = register_count * 2;
                reply.length    = register_count * 2 + 3;
            } else {
                make_exception_reply(rtu_exception_code::SERVER_DEVICE_FAILURE, indication, reply);
            }
        }
        void execute_write_coils(const rtu_message &indication, rtu_message &reply)
        {
            uint16_t address    = flip_endianess(*(uint16_t*)&indication.buffer[2]),
                     bit_count  = flip_endianess(*(uint16_t*)&indication.buffer[4]),
                     byte_count = indication.buffer[6];
            uint8_t  *bits      = &indication.buffer[7];
            if (!self().is_write_coils_supported()) {
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
            if (!self().is_write_coils_valid_data_address(address, bit_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_ADDRESS, indication, reply);
                return;
            }
            if (!self().is_write_coils_valid_data_value(bit_count, byte_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_VALUE, indication, reply);
                return;
            }
            if (!self().execute_write_coils(address, bits, bit_count)) {
                make_exception_reply(rtu_exception_code::SERVER_DEVICE_FAILURE, indication, reply);
                return;
            }
            make_echo_reply(indication, reply, WRITE_COILS_REPLY_LENGTH);
        }
        void execute_write_registers(const rtu_message &indication, rtu_message &reply)
        {
            uint16_t address        = flip_endianess(*(uint16_t*)&indication.buffer[2]),
                     register_count = flip_endianess(*(uint16_t*)&indication.buffer[4]);
            uint8_t  byte_count     = indication.buffer[6];
            uint16_t *words         = (uint16_t*)&indication.buffer[7];
            if (!self().is_write_registers_supported()) {
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
            if (!self().is_write_registers_valid_data_address(address, register_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_ADDRESS, indication, reply);
                return;
            }
            if (!self().is_write_registers_valid_data_value(register_count, byte_count)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_VALUE, indication, reply);
                return;
            }
            if (!self().execute_write_registers(address, words, register_count)) {
                make_exception_reply(rtu_exception_code::SERVER_DEVICE_FAILURE, indication, reply);
                return;
            }
            make_echo_reply(indication, reply, WRITE_REGISTERS_REPLY_LENGTH);
        }
        void execute_write_single_coil(const rtu_message &indication, rtu_message &reply)
        {
            uint16_t address = flip_endianess(*(uint16_t*)&indication.buffer[2]),
                     value   = flip_endianess(*(uint16_t*)&indication.buffer[4]);
            if (!self().is_write_single_coil_supported()) {
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
            if (!self().is_write_single_coil_valid_data_address(address)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_ADDRESS, indication, reply);
                return;
            }
            if (!self().is_write_single_coil_valid_data_value(value)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_VALUE, indication, reply);
                return;
            }
            if (!self().execute_write_single_coil(address, bool(value))) {
                make_exception_reply(rtu_exception_code::SERVER_DEVICE_FAILURE, indication, reply);
                return;
            }
            make_echo_reply_no_crc(indication, reply); // discard CRC in echo, it will be recalculated
            return;
        }
        void execute_write_single_register(const rtu_message &indication, rtu_message &reply)
        {
            uint16_t address        = flip_endianess(*(uint16_t*)&indication.buffer[2]),
                     register_value = flip_endianess(*(uint16_t*)&indication.buffer[4]);
            if (!self().is_write_single_register_supported()) {
                make_exception_reply(rtu_exception_code::ILLEGAL_FUNCTION, indication, reply);
                return;
            }
            if (!self().is_write_registers_valid_data_address(address, 1)) {
                make_exception_reply(rtu_exception_code::ILLEGAL_DATA_ADDRESS, indication, reply);
                return;
            }
            if (!self().execute_write_registers(address, &register_value, 1)) {
                make_exception_reply(rtu_exception_code::SERVER_DEVICE_FAILURE, indication, reply);
                return;
            }
            make_echo_reply_no_crc(indication, reply);
        }
        void make_echo_reply_no_crc(const rtu_message &indication, rtu_message &reply)
        {
            memmove(reply.buffer, indication.buffer, indication.length - 2);
            reply.length = indication.length - 2;
        }
        void make_echo_reply(const rtu_message &indication, rtu_message &reply, uint8_t length)
        {
            memmove(reply.buffer, indication.buffer, length);
            reply.length = length;
        }
        void make_exception_reply(rtu_exception_code ex, const rtu_message &indication, rtu_message &reply)
        {
            reply.buffer[0] = indication.buffer[0];
            reply.buffer[1] = indication.buffer[1] | 0x80;
            reply.buffer[2] = uint8_t(ex);
            reply.length    = 3;
        }
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_PDU_HANDLER_BASE
