#include <avr/io.h>
#include <util/delay.h>

namespace vla {

    enum class clock_source {
        STOP,
        PRESCALE_1,
        PRESCALE_8,
        PRESCALE_64,
        PRESCALE_256,
        PRESCALE_1024,
        EXTERNAL_CLOCK_T0_FALLING,
        EXTERNAL_CLOCK_T0_RISING
    };

    /*
     * Timer output	Arduino output	Chip pin	Pin name
     * OC0A	                     6        12        PD6
     * OC0B                      5        11        PD5
     * OC1A                      9        15        PB1
     * OC1B                     10        16        PB2
     * OC2A                     11        17        PB3
     * OC2B                      3         5        PD3
     */
    class pwm0pc
    {
    public:
        pwm0pc()
        {
            // ensure  OC0A and 0C0B are set to output
            DDRD  |= _BV(PD6) | _BV(PD5);
            // non inverted Phase correct
            TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
            set_level_a(0);
            set_level_b(0);
        }
        ~pwm0pc()
        {
            set_clock(clock_source::STOP);
            set_level_a(0);
            set_level_b(0);
        }
        void set_level_a(uint8_t l)
        {
            OCR0A = l;
        }
        void set_level_b(uint8_t l)
        {
            OCR0B = l;
        }
        void set_clock(clock_source cs)
        {
            // TCCR0B = _BV(CS00); // start with no preescaling
            TCCR0B = static_cast<uint8_t>(cs);
        }
    private:
        pwm0pc(const pwm0pc&)                  = delete;
        pwm0pc(pwm0pc&&)                       = delete;
        pwm0pc& operator=(const pwm0pc &other) = delete;
        pwm0pc& operator=(pwm0pc &&other)      = delete;
    };

    class pwm2pc
    {
    public:
        pwm2pc()
        {
            // ensure  OC0A and 0C0B are set to output
            DDRD  |= _BV(PD3);
            DDRB  |= _BV(PB3);
            // non inverted Phase correct
            TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20);
            set_level_a(0);
            set_level_b(0);
        }
        ~pwm2pc()
        {
            set_clock(clock_source::STOP);
            set_level_a(0);
            set_level_b(0);
        }
        void set_level_a(uint8_t l)
        {
            OCR2A = l;
        }
        void set_level_b(uint8_t l)
        {
            OCR2B = l;
        }
        void set_clock(clock_source cs)
        {
            // TCCR0B = _BV(CS00); // start with no preescaling
            TCCR2B = static_cast<uint8_t>(cs);
        }
    private:
        pwm2pc(const pwm2pc&)                  = delete;
        pwm2pc(pwm2pc&&)                       = delete;
        pwm2pc& operator=(const pwm2pc &other) = delete;
        pwm2pc& operator=(pwm2pc &&other)      = delete;
    };


}
