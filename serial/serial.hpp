#ifndef VLA_SERIAL_SERIAL_HPP
#define VLA_SERIAL_SERIAL_HPP

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "circular_buffer.hpp"

namespace vla {

    struct serial_speed_9600
    {
        serial_speed_9600()
        {
#define BAUD 9600
#include <util/setbaud.h>
        UBRR0H = UBRRH_VALUE;
        UBRR0L = UBRRL_VALUE;
        }
    };

    typedef void (*read_cb_t)(void*, uint8_t);
    typedef void (*write_cb_t)(void*);

    extern read_cb_t  on_serial_received;
    extern write_cb_t on_serial_ready_to_send;
    extern void*      read_buffer;
    extern void*      write_buffer;

    template <size_t BufferSize>
    struct serial_async_write
    {
        typedef circular_buffer<uint8_t, BufferSize> write_buffer_t;
        inline serial_async_write()
        {
            vla::write_buffer = &write_buffer;
            vla::on_serial_ready_to_send = [](void *p) {
                auto buff = static_cast<write_buffer_t*>(p);
                if (!buff->empty()) {
                    UDR0 = buff->pop_front();
                } else {
                    UCSR0B &= ~(1<<UDRIE0);
                }
            };
            UCSR0B |= 1<<TXEN0;
            sei();
        }
        inline ~serial_async_write()
        {
            UCSR0B &= ~((1<<UDRIE0));
            vla::on_serial_ready_to_send = nullptr;
            vla::write_buffer            = nullptr;
        }
        inline void write(const uint8_t byte)
        {
            while (write_buffer.full());
            write_buffer.push_back(byte);
            UCSR0B |= (1<<UDRIE0);
        }
        // :TODO: se podría optimizar usando funciones del del buffer
        // para agregar rangos.
        template <typename BufferSizeType>
        inline BufferSizeType write_some(const uint8_t *bytes, BufferSizeType byte_count)
        {
            BufferSizeType bytes_written = 0;
            while (bytes_written < byte_count && !write_buffer.full()) {
                write_buffer.push_back(bytes[bytes_written]);
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
        write_buffer_t write_buffer;
    };

    template<size_t BufferSize>
    struct serial_async_read
    {
        typedef circular_buffer<uint8_t, BufferSize>  read_buffer_t;
        inline serial_async_read()
        {
            vla::read_buffer  = &read_buffer;
            vla::on_serial_received = [](void *buff, uint8_t byte) {
                static_cast<read_buffer_t*>(buff)->push_back(byte);
            };
            UCSR0B |= (1<<RXEN0)|(1<<RXCIE0);
            sei();
        }
        inline ~serial_async_read()
        {
            UCSR0B &= ~((1<<RXEN0)|(1<<RXCIE0));
            vla::on_serial_received = nullptr;
            vla::read_buffer        = nullptr;
        }
        inline uint8_t read()
        {
            while (read_buffer.empty());
            return read_buffer.pop_front();
        }
        // :TODO: se podría optimizar usando funciones del del buffer
        // para extraer rangos.
        template<typename BufferSizeType>
        inline BufferSizeType read_some(uint8_t *bytes, BufferSizeType byte_count)
        {
            BufferSizeType read_count = 0;
            while (read_count < byte_count && !read_buffer.empty()) {
                bytes[read_count] = read_buffer.pop_front();
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
    };

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

    template<typename SerialSpeed, typename SerialReadMode, typename SerialWriteMode>
    class serial : public SerialReadMode, public SerialWriteMode
    {
    private:
        SerialSpeed  speed;
    public:
        inline serial()
        {
            UCSR0C = (3<<UCSZ00);
        }
        inline ~serial()
        {
            UCSR0C &= ~(3<<UCSZ00);
        }
    private:
        serial(const serial&)                  = delete;
        serial(serial&&)                       = delete;
        serial& operator=(const serial &other) = delete;
        serial& operator=(serial &&other)      = delete;

    };

    struct not_available{};

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

    typedef serial<serial_speed_9600,
                   serial_sync_read,
                   serial_sync_write> serial_9600;
    typedef serial<serial_speed_9600,
                   serial_async_read<16>,
                   serial_async_write<16>> serial_9600_async;
    typedef serial<serial_speed_9600,
                   not_available,
                   serial_sync_write> serial_9600_sync_write_only;

    serial_9600& get_serial_debug();
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

}

#endif // VLA_SERIAL_SERIAL_HPP
