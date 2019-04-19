#ifndef VLA_SERIAL_SERIAL_HPP
#define VLA_SERIAL_SERIAL_HPP

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <vla/msg_queue.hpp>
#include <vla/hex_number.hpp>

namespace vla {

    const int SERIAL_ASYNC_BUFFER_SIZE_DEFAULT = 16;

    /**
     * According to ATmega328/P DATASHEET COMPLETE 24.4.1
     */
    constexpr uint16_t calculate_ubrr(uint32_t baud)
    {
        return F_CPU / baud / 16 - 1;
    }

    enum class serial_speed : uint16_t
    {
        BAUD_9600  = calculate_ubrr(9600),
        BAUD_19200 = calculate_ubrr(19200),
        BAUD_38400 = calculate_ubrr(38400)
    };

    typedef void (*read_cb_t)(void*, uint8_t);
    typedef void (*write_cb_t)(void*);

    extern read_cb_t  on_serial_received;
    extern write_cb_t on_serial_ready_to_send;
    extern void*      read_buffer;
    extern void*      write_buffer;

    template <size_t BufferSize>
    struct basic_serial_async_write
    {
        typedef msg_queue<uint8_t, BufferSize> write_buffer_t;
        inline basic_serial_async_write()
        {
            vla::write_buffer = &write_buffer;
            vla::on_serial_ready_to_send =
                basic_serial_async_write::on_serial_ready_to_send;
            UCSR0B |= 1<<TXEN0;
            sei();
        }
        inline ~basic_serial_async_write()
        {
            UCSR0B &= ~((1<<UDRIE0));
            vla::on_serial_ready_to_send = nullptr;
            vla::write_buffer            = nullptr;
        }
        inline void write(const uint8_t byte)
        {
            while (write_buffer.full());
            write_buffer.push(byte);
            UCSR0B |= (1<<UDRIE0);
        }
        template <typename BufferSizeType>
        inline BufferSizeType write_some(const uint8_t *bytes, BufferSizeType byte_count)
        {
            BufferSizeType bytes_written = 0;
            while (bytes_written < byte_count && !write_buffer.full()) {
                write_buffer.push(bytes[bytes_written]);
                ++bytes_written;
            }
            UCSR0B |= (1<<UDRIE0);
            return bytes_written;
        }
        inline write_buffer_t* buffer()
        {
            return &write_buffer;
        }
    private:
        static void on_serial_ready_to_send(void *p)
        {
            auto buff = static_cast<write_buffer_t*>(p);
            if (!buff->empty()) {
                UDR0 = buff->pop();
            } else {
                UCSR0B &= ~(1<<UDRIE0);
            }
        }
        write_buffer_t write_buffer;
    };

    using serial_async_write =
        basic_serial_async_write<SERIAL_ASYNC_BUFFER_SIZE_DEFAULT>;

    template<size_t BufferSize>
    struct basic_serial_async_read
    {
        typedef msg_queue<uint8_t, BufferSize>  read_buffer_t;
        inline basic_serial_async_read()
        {
            vla::read_buffer  = &read_buffer;
            vla::on_serial_received = on_serial_received;
            UCSR0B |= (1<<RXEN0)|(1<<RXCIE0);
            sei();
        }
        inline ~basic_serial_async_read()
        {
            UCSR0B &= ~((1<<RXEN0)|(1<<RXCIE0));
            vla::on_serial_received = nullptr;
            vla::read_buffer        = nullptr;
        }
        inline uint8_t read()
        {
            while (read_buffer.empty());
            return read_buffer.pop();
        }
        // :TODO: se podría optimizar usando funciones del del buffer
        // para extraer rangos.
        template<typename BufferSizeType>
        inline BufferSizeType read_some(uint8_t *bytes, BufferSizeType byte_count)
        {
            BufferSizeType read_count = 0;
            while (read_count < byte_count && !read_buffer.empty()) {
                bytes[read_count] = read_buffer.pop();
                ++read_count;
            }
            return read_count;
        }
        inline read_buffer_t* buffer()
        {
            return &read_buffer;
        }
    private:
        read_buffer_t read_buffer;
        static void on_serial_received(void *buff, uint8_t byte)
        {
            static_cast<read_buffer_t*>(buff)->push(byte);
        };
    };

    using serial_async_read =
        basic_serial_async_read<SERIAL_ASYNC_BUFFER_SIZE_DEFAULT>;

    struct serial_sync_write
    {
        serial_sync_write()
        {
            UCSR0B |= 1<<TXEN0;
        }
        inline ~serial_sync_write()
        {
            UCSR0B &= ~(1<<TXEN0);
        }
        inline void write(const uint8_t byte) const
        {
            while(!(UCSR0A & (1<<UDRE0)));
            UDR0 = byte;
        }
        template <typename SizeType>
        inline SizeType write_some(const uint8_t *bytes, SizeType byte_count)
        {
            if (byte_count && (UCSR0A & (1<<UDRE0))) {
                UDR0 = bytes[0];
                return 1;
            }
            return 0;
        }
    };

    struct serial_sync_read
    {
        inline serial_sync_read()
        {
            UCSR0B |= 1<<RXEN0;
        }
        inline ~serial_sync_read()
        {
            UCSR0B &= ~(1<<RXEN0);
        }
        inline uint8_t read()
        {
            while(!(UCSR0A & (1<<RXC0)));
            return UDR0;
        }
        template <typename SizeType>
        inline SizeType read_some(uint8_t *bytes, SizeType byte_count)
        {
            for (SizeType i = 0; i < byte_count; ++i) {
                bytes[i] = read();
            }
            return byte_count;
        }
    };

    enum class serial_mode : uint8_t {
        ASYNC      = (0x0<<UMSEL00),
        SYNC       = (0x1<<UMSEL00),
        MASTER_SPI = (0x3<<UMSEL00)
    };
    enum class serial_char_size : uint8_t {
        FIVE  = (0x0<<UCSZ00),
        SIX   = (0x1<<UCSZ00),
        SEVEN = (0x2<<UCSZ00),
        EIGHT = (0x3<<UCSZ00),
        NINE  = (0x7<<UCSZ00)
    };
    enum class serial_parity : uint8_t {
        NONE = (0x0<<UPM00),
        EVEN = (0x2<<UPM00),
        ODD  = (0x3<<UPM00)
    };
    enum class serial_stop_bits : uint8_t {
        ONE = (0x0<<USBS0),
        TWO = (0x1<<USBS0)
    };
    constexpr uint8_t calculate_serial_config(
        serial_parity parity, serial_stop_bits stop_bits,
        serial_char_size char_size, serial_mode mode)
    {
        return static_cast<uint8_t>(mode)
            |  static_cast<uint8_t>(char_size)
            |  static_cast<uint8_t>(parity)
            |  static_cast<uint8_t>(stop_bits);
    }

    template<typename SerialReadMode, typename SerialWriteMode>
    class serial : public SerialReadMode, public SerialWriteMode
    {
    public:
        inline serial(
            serial_speed     speed     = serial_speed::BAUD_9600,
            serial_parity    parity    = serial_parity::NONE,
            serial_stop_bits stop_bits = serial_stop_bits::ONE,
            serial_char_size char_size = serial_char_size::EIGHT,
            serial_mode      mode      = serial_mode::ASYNC)
        {
            set_config(parity, stop_bits, char_size, mode);
            set_speed(speed);
        }
        inline void set_speed(serial_speed speed)
        {
            UBRR0  = static_cast<uint16_t>(speed);
        }
        inline void set_config(
            serial_parity    parity   , serial_stop_bits stop_bits,
            serial_char_size char_size, serial_mode       mode)
        {
            UCSR0C = calculate_serial_config(parity, stop_bits, char_size, mode);
        }
    private:
        serial(const serial&)                  = delete;
        serial(serial&&)                       = delete;
        serial& operator=(const serial &other) = delete;
        serial& operator=(serial &&other)      = delete;

    };

    struct not_available{};

    template<typename Serial>
    inline void write_hex(Serial &s, uint8_t v)
    {
        write(s, "0x");
        write(s, hex_number(v).data());
    }

    template<typename Serial>
    inline void write_hex(Serial &s, uint16_t v)
    {
        write(s, "0x");
        write(s, hex_number(v).data());
    }

    template<typename Serial>
    inline void write(Serial &s, uint8_t byte)
    {
        s.write(byte);
    }
    template<typename Serial>
    inline void write(Serial &s, const char *str)
    {
        write(s, reinterpret_cast<const uint8_t*>(str));
    }
    template<typename Serial>
    inline void write(Serial &s, const uint8_t *str)
    {
        for (; *str; ++str) {
            s.write(*str);
        }
    }
    template<typename Serial, typename SizeType>
    inline void write(Serial &s, const uint8_t *buff, SizeType buff_len)
    {
        SizeType written = s.write_some(buff, buff_len);
        for (; written < buff_len; ++written) {
            s.write(buff[written]);
        }
    }
    template<typename Serial>
    inline void write_line(Serial &s, const char *str)
    {
        write(s, reinterpret_cast<const uint8_t*>(str));
        write(s, "\r\n");

    }
    template <typename Serial>
    uint8_t read(Serial &s)
    {
        return s.read();
    }
    template<typename Serial, typename SizeType>
    inline SizeType read(Serial &s, uint8_t *buff, SizeType size)
    {
        return s.read_some(buff, size);
    }
    template<typename Serial, typename SizeType>
    inline SizeType read_line(Serial &s, uint8_t *buff, SizeType size,
                              uint8_t mark = '\n')
    {
        SizeType i = 0;
        while (i < size - 1 && (buff[i] = s.read()) != mark) {
            ++i;
        }
        buff[i] = '\0';
        return i;
    }
    template<typename serial_t>
    void wait(serial_t &ser, const char* msg = 0)
    {
        uint8_t buffer[80];
        if (msg) {
            write(ser, msg);
            write(ser, "\r\n");
        }
        write(ser, "press enter\r\n");
        read_line(ser, buffer, sizeof buffer, '\r');
    }

    using serial_sync            = serial<serial_sync_read , serial_sync_write>;
    using serial_async           = serial<serial_async_read, serial_async_write>;
    using serial_sync_write_only = serial<not_available    , serial_sync_write>;

    serial_sync& get_serial_debug();
    serial_async& get_serial_async_debug();
}

#endif // VLA_SERIAL_SERIAL_HPP
