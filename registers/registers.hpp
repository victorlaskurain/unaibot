#ifndef VLA_REGISTERS_REGISTERS_HPP
#define VLA_REGISTERS_REGISTERS_HPP

#include <avr/io.h>

namespace vla {

    template <typename param_port_t, int pin_number>
    struct pin_t
    {
        typedef param_port_t port_t;
        inline static int offset()
        {
            return pin_number;
        }
        inline static void set()
        {
            port_t::ref() |= _BV(pin_number);
        }
        inline static void clear()
        {
            port_t::ref() &= ~_BV(pin_number);
        }
        inline static void set_mode_input()
        {
            pin_t<typename port_t::ddr_t, pin_number>::clear();
        }
        inline static void set_mode_output()
        {
            pin_t<typename port_t::ddr_t, pin_number>::set();
        }
    };

    struct DDRB_t
    {
        inline static volatile uint8_t& ref()
        {
            return DDRB;
        }
    };
    typedef pin_t<DDRB_t, 0> DDRB0_t;
    typedef pin_t<DDRB_t, 1> DDRB1_t;
    typedef pin_t<DDRB_t, 2> DDRB2_t;
    typedef pin_t<DDRB_t, 3> DDRB3_t;
    typedef pin_t<DDRB_t, 4> DDRB4_t;
    typedef pin_t<DDRB_t, 5> DDRB5_t;
    typedef pin_t<DDRB_t, 6> DDRB6_t;
    typedef pin_t<DDRB_t, 7> DDRB7_t;
    struct PORTB_t
    {
        typedef DDRB_t ddr_t;
        inline static volatile uint8_t& ref()
        {
            return PORTB;
        }
    };
    typedef pin_t<PORTB_t, 0> PORTB0_t;
    typedef pin_t<PORTB_t, 1> PORTB1_t;
    typedef pin_t<PORTB_t, 2> PORTB2_t;
    typedef pin_t<PORTB_t, 3> PORTB3_t;
    typedef pin_t<PORTB_t, 4> PORTB4_t;
    typedef pin_t<PORTB_t, 5> PORTB5_t;
    typedef pin_t<PORTB_t, 6> PORTB6_t;
    typedef pin_t<PORTB_t, 7> PORTB7_t;

    struct DDRC_t
    {
        inline static volatile uint8_t& ref()
        {
            return DDRC;
        }
    };
    typedef pin_t<DDRC_t, 0> DDRC0_t;
    typedef pin_t<DDRC_t, 1> DDRC1_t;
    typedef pin_t<DDRC_t, 2> DDRC2_t;
    typedef pin_t<DDRC_t, 3> DDRC3_t;
    typedef pin_t<DDRC_t, 4> DDRC4_t;
    typedef pin_t<DDRC_t, 5> DDRC5_t;
    typedef pin_t<DDRC_t, 6> DDRC6_t;
    typedef pin_t<DDRC_t, 7> DDRC7_t;
    struct PORTC_t
    {
        typedef DDRC_t ddr_t;
        inline static volatile uint8_t& ref()
        {
            return PORTC;
        }
    };
    typedef pin_t<PORTC_t, 0> PORTC0_t;
    typedef pin_t<PORTC_t, 1> PORTC1_t;
    typedef pin_t<PORTC_t, 2> PORTC2_t;
    typedef pin_t<PORTC_t, 3> PORTC3_t;
    typedef pin_t<PORTC_t, 4> PORTC4_t;
    typedef pin_t<PORTC_t, 5> PORTC5_t;
    typedef pin_t<PORTC_t, 6> PORTC6_t;
    typedef pin_t<PORTC_t, 7> PORTC7_t;

    struct DDRD_t
    {
        inline static volatile uint8_t& ref()
        {
            return DDRD;
        }
    };
    typedef pin_t<DDRD_t, 0> DDRD0_t;
    typedef pin_t<DDRD_t, 1> DDRD1_t;
    typedef pin_t<DDRD_t, 2> DDRD2_t;
    typedef pin_t<DDRD_t, 3> DDRD3_t;
    typedef pin_t<DDRD_t, 4> DDRD4_t;
    typedef pin_t<DDRD_t, 5> DDRD5_t;
    typedef pin_t<DDRD_t, 6> DDRD6_t;
    typedef pin_t<DDRD_t, 7> DDRD7_t;
    struct PORTD_t
    {
        typedef DDRD_t ddr_t;
        inline static volatile uint8_t& ref()
        {
            return PORTD;
        }
    };
    typedef pin_t<PORTD_t, 0> PORTD0_t;
    typedef pin_t<PORTD_t, 1> PORTD1_t;
    typedef pin_t<PORTD_t, 2> PORTD2_t;
    typedef pin_t<PORTD_t, 3> PORTD3_t;
    typedef pin_t<PORTD_t, 4> PORTD4_t;
    typedef pin_t<PORTD_t, 5> PORTD5_t;
    typedef pin_t<PORTD_t, 6> PORTD6_t;
    typedef pin_t<PORTD_t, 7> PORTD7_t;

    struct TCCR0A_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR0A;
        }
    };
    typedef pin_t<TCCR0A_t, COM0A1> COM0A1_t;
    typedef pin_t<TCCR0A_t, COM0B1> COM0B1_t;
    typedef pin_t<TCCR0A_t, WGM00> WGM00_t;

    struct TCCR0B_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR0B;
        }
    };

    struct OCR0A_t
    {
        inline static volatile uint8_t& ref()
        {
            return OCR0A;
        }
    };

    struct OCR0B_t
    {
        inline static volatile uint8_t& ref()
        {
            return OCR0B;
        }
    };

    struct TCCR2A_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR2A;
        }
    };
    typedef pin_t<TCCR2A_t, COM0A1> COM2A1_t;
    typedef pin_t<TCCR2A_t, COM0B1> COM2B1_t;
    typedef pin_t<TCCR2A_t, WGM00> WGM20_t;

    struct TCCR2B_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR2B;
        }
    };

    struct OCR2A_t
    {
        inline static volatile uint8_t& ref()
        {
            return OCR2A;
        }
    };

    struct OCR2B_t
    {
        inline static volatile uint8_t& ref()
        {
            return OCR2B;
        }
    };

}

#endif // VLA_REGISTERS_REGISTERS_HPP
