#ifndef VLA_REGISTERS_REGISTERS_HPP
#define VLA_REGISTERS_REGISTERS_HPP

#include <avr/io.h>

namespace vla {

    template <typename param_register_t, int bit_number>
    struct bit_t
    {
        typedef param_register_t register_t;
        constexpr inline static int offset()
        {
            return bit_number;
        }
        inline static bool get()
        {
            return register_t::ref() & _BV(bit_number);
        }
        inline static void set()
        {
            register_t::ref() |= _BV(bit_number);
        }
        inline static void clear()
        {
            register_t::ref() &= ~_BV(bit_number);
        }
    };

    template <typename param_port_t, int pin_number>
    struct pin_t : public bit_t<param_port_t, pin_number>
    {
        typedef param_port_t port_t;
        inline static void set_mode_input()
        {
            pin_t<typename port_t::ddr_t, pin_number>::clear();
        }
        inline static void set_mode_output()
        {
            pin_t<typename port_t::ddr_t, pin_number>::set();
        }
    };

    template<class port_t>
    struct is_analog_port {
        const static bool value = false;
    };
    template<class pin_t>
    struct is_analog_pin {
        const static bool value = is_analog_port<typename pin_t::port_t>::value;
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
    typedef bit_t<DDRC_t, 0> DDRC0_t;
    typedef bit_t<DDRC_t, 1> DDRC1_t;
    typedef bit_t<DDRC_t, 2> DDRC2_t;
    typedef bit_t<DDRC_t, 3> DDRC3_t;
    typedef bit_t<DDRC_t, 4> DDRC4_t;
    typedef bit_t<DDRC_t, 5> DDRC5_t;
    typedef bit_t<DDRC_t, 6> DDRC6_t;
    typedef bit_t<DDRC_t, 7> DDRC7_t;
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
    template<>
    struct is_analog_port<PORTC_t> {
        const static bool value = true;
    };

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
    typedef bit_t<TCCR0A_t, COM0A0> COM0A0_t;
    typedef bit_t<TCCR0A_t, COM0A1> COM0A1_t;
    typedef bit_t<TCCR0A_t, COM0B0> COM0B0_t;
    typedef bit_t<TCCR0A_t, COM0B1> COM0B1_t;
    typedef bit_t<TCCR0A_t, WGM00> WGM00_t;

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

    struct TCCR1A_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR1A;
        }
    };

    struct TCCR1B_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR1B;
        }
    };

    struct TCCR2A_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCCR2A;
        }
    };
    typedef bit_t<TCCR2A_t, COM2A1> COM2A1_t;
    typedef bit_t<TCCR2A_t, COM2B1> COM2B1_t;
    typedef bit_t<TCCR2A_t, WGM20> WGM20_t;

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

    struct TIMSK0_t
    {
        inline static volatile uint8_t& ref()
        {
            return TIMSK0;
        }
    };
    typedef bit_t<TIMSK0_t, OCIE0A> OCIE0A_t;
    typedef bit_t<TIMSK0_t, OCIE0B> OCIE0B_t;
    typedef bit_t<TIMSK0_t, TOIE0>  TOIE0_t;

    struct TIMSK1_t
    {
        inline static volatile uint8_t& ref()
        {
            return TIMSK1;
        }
    };
    typedef bit_t<TIMSK1_t, OCIE1A> OCIE1A_t;
    typedef bit_t<TIMSK1_t, OCIE1B> OCIE1B_t;
    typedef bit_t<TIMSK1_t, TOIE1>  TOIE1_t;

    struct TIMSK2_t
    {
        inline static volatile uint8_t& ref()
        {
            return TIMSK2;
        }
    };
    typedef bit_t<TIMSK2_t, OCIE2A> OCIE2A_t;
    typedef bit_t<TIMSK2_t, OCIE2B> OCIE2B_t;
    typedef bit_t<TIMSK2_t, TOIE2>  TOIE2_t;

    struct TCNT0_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCNT0;
        }
    };

    struct TCNT1_t
    {
        inline static volatile uint16_t& ref()
        {
            return TCNT1;
        }
    };

    struct TCNT2_t
    {
        inline static volatile uint8_t& ref()
        {
            return TCNT2;
        }
    };

    enum class ADMUX_REF_t:uint8_t {AREF = 0, AVCC = 1, INTERNAL_1_1 = 3};
    struct ADMUX_t
    {
        inline static volatile uint8_t& ref()
        {
            return ADMUX;
        }
        template<typename pin_t>
        inline static void select_channel(pin_t = pin_t{})
        {
            static_assert(is_analog_pin<pin_t>::value, "Trying analog read on digital pin");
            ADMUX = (ADMUX & ~0x03) | static_cast<uint8_t>(pin_t::offset());
        }
        inline static ADMUX_REF_t get_ref()
        {
            return ADMUX_REF_t((ADMUX & 0xc0) >> 6);
        }
        inline static void set_ref(ADMUX_REF_t ref)
        {
            ADMUX = (ADMUX & ~0xc0) | (static_cast<uint8_t>(ref)<<6);
        }
    };

    struct ADCSRA_t
    {
        inline static volatile uint8_t& ref()
        {
            return ADCSRA;
        }
    };
    typedef bit_t<ADCSRA_t, ADEN> ADEN_t;
    typedef bit_t<ADCSRA_t, ADSC> ADSC_t;

    struct DIDR0_t {
        inline static volatile uint8_t& ref()
        {
            return DIDR0;
        }
    };

    struct PRR_t
    {
        inline static volatile uint8_t& ref()
        {
            return PRR;
        }
    };
    typedef bit_t<PRR_t, PRADC> PRADC_t;

}

#endif // VLA_REGISTERS_REGISTERS_HPP
