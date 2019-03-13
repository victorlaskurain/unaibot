#include <vla/serial.hpp>
#include <vla/circular_buffer.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

static void write_async()
{
    vla::serial_async ser(vla::serial_speed::BAUD_38400);
    while(1) {
        for (char i = '1'; i < '6'; ++i) {
            ser.write(i);
            vla::write(ser, "ASYNC01\r\n");
            PORTB = 0x20;
            _delay_ms(500);
            PORTB = 0x00;
            _delay_ms(500);
        }
        vla::write(ser, "\r\n");
        _delay_ms(1000);
    }
}

static void write_sync()
{
    vla::serial_sync_write_only ser(vla::serial_speed::BAUD_38400);
    // vla::serial_9600 ser;
    while(1) {
        for (char i = '1'; i < '6'; ++i) {
            ser.write(i);
            vla::write(ser, "SYNC012\r\n");
            PORTB = 0x20;
            _delay_ms(500);
            PORTB = 0x00;
            _delay_ms(500);
        }
        vla::write(ser, "\r\n");
        _delay_ms(1000);
    }
}

enum class br_t {
    BR_9600  = 9600,
    BR_19200 = 19200,
};

static void write_async_at_speed_inner(vla::serial_async_write &ser)
{
    while(1) {
        for (char i = '1'; i < '6'; ++i) {
            ser.write(i);
            vla::write(ser, "ASYNC012\r\n");
            PORTB = 0x20;
            _delay_ms(500);
            PORTB = 0x00;
            _delay_ms(500);
        }
        vla::write(ser, "\r\n");
        _delay_ms(1000);
    }
}

static void write_async_at_speed(vla::serial_speed br)
{
    vla::serial_async ser{br};
    write_async_at_speed_inner(ser);
}

static void write_sync_at_speed_inner(vla::serial_sync_write &ser)
{
    while(1) {
        for (char i = '1'; i < '6'; ++i) {
            ser.write(i);
            vla::write(ser, "SYNC012\r\n");
            PORTB = 0x20;
            _delay_ms(500);
            PORTB = 0x00;
            _delay_ms(500);
        }
        vla::write(ser, "\r\n");
        _delay_ms(1000);
    }
}

static void write_sync_at_speed(vla::serial_speed br)
{
    vla::serial_sync ser{br};
    write_sync_at_speed_inner(ser);
}

static void write_sync_async()
{
    while (1) {
        {
            vla::serial_sync ser;
            for (char i = '1'; i < '6'; ++i) {
                ser.write(i);
                vla::write(ser, "SYNC012\r\n");
                PORTB = 0x20;
                _delay_ms(500);
                PORTB = 0x00;
                _delay_ms(500);
            }
            vla::write(ser, "\r\n");
        }
        _delay_ms(1000);
        {
            vla::serial_async ser{vla::serial_speed::BAUD_38400};
            for (char i = '1'; i < '6'; ++i) {
                ser.write(i);
                vla::write(ser, "ASYNC01\r\n");
                PORTB = 0x20;
                _delay_ms(500);
                PORTB = 0x00;
                _delay_ms(500);
            }
            write(ser, "\r\n");
            _delay_ms(100);
        }
    }
}

static void echo()
{
    static const uint8_t buffer_size = UINT8_MAX;
    uint8_t buffer[buffer_size];
    vla::serial_async ser{vla::serial_speed::BAUD_38400};
    vla::write(ser, "READY\r\n");
    while(1) {
        vla::read_line(ser, buffer, buffer_size, '\r');
        _delay_ms(100);
        vla::write(ser, "->");
        vla::write(ser, buffer);
        vla::write(ser, "\r\n");
    }
}

int main(void)
{
    DDRB = 0xFF;
    _delay_ms(1000);
    echo();
    return 0;
}
