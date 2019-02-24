#ifndef VLA_ANALOG_READ_ANALOG_READ_HPP
#define VLA_ANALOG_READ_ANALOG_READ_HPP

#include <vla/registers.hpp>
#include <stddef.h>

namespace vla {

    template<typename pin_t>
    uint8_t analog_read()
    {
        static_assert(is_analog_pin<pin_t>::value, "Trying analog read on digital pin");
        // clear PRR.PRADC to enable ADC
        PRADC_t::clear();
        // disable DIDR0 bit for the channel
        bit_t<DIDR0_t, pin_t::offset()>::set();
        // set input mode
        pin_t::set_mode_input();
        // set ADCSRA.ADEN to enable ADC
        ADEN_t::set();
        // preescaler
        ADPS0_t::set();
        ADPS1_t::set();
        ADPS2_t::set();
        // set left align, since we only need 1 byte
        ADLAR_t::set();
        // set reference in the ADMUX.REFS register
        ADMUX_t::set_ref(ADMUX_REF_t::AVCC);
        // select pin with the ADMUX.MUX register
        ADMUX_t::select_channel<pin_t>(); // ADMUX_t::select_channel(pin_t{}) is equivalent
        // set ADCSRA.ADSC to start conversion
        ADSC_t::set();
        // wait for ADCSRA.ADSC to clear
        while(ADSC_t::get());
        // return ADCH
        return ADCH;
    }

}

#endif // VLA_ANALOG_READ_ANALOG_READ_HPP
