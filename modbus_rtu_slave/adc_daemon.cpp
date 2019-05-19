#include "adc_daemon.hpp"
#include <vla/registers.hpp>

namespace vla {
    inline void enable_adc()
    {
        // clear PRR.PRADC to enable ADC
        PRADC_t::clear();
        // preescaler
        ADPS0_t::set();
        ADPS1_t::set();
        ADPS2_t::set();
        // set ADCSRA.ADEN to enable ADC
        ADEN_t::set();
        // set right align, we will use full precision
        ADLAR_t::clear();
    }

    inline void disable_adc()
    {
        ADEN_t::clear();
        PRADC_t::set();
    }

    inline bool read_ready()
    {
        return !ADSC_t::get();
    }

    inline uint16_t read_adc_value()
    {
        uint16_t v = ADCL;
        v |= (ADCH << 8);
        return v;
    }
}

void vla::adc_daemon::operator()()
{
    adc_msg_t msg;
    ptxx_begin();
    while (true) {
        if (!in_q.empty()) {
            in_q.pop(msg);
            const uint8_t adc_i = uint8_t(msg.id);
            if (msg.cb) { // activate channel
                bool must_enable = (0 == enabled_count);
                if (!channels[adc_i].cb) {
                    ++enabled_count;
                }
                channels[adc_i].cb   = msg.cb;
                channels[adc_i].data = msg.data;
                if (must_enable) {
                    enable_adc();
                    start_read_next_enabled_channel();
                }
            } else { // deactivate channel
                if (channels[adc_i].cb) {
                    --enabled_count;
                }
                channels[adc_i].cb   = 0;
                channels[adc_i].data = 0;
                if (!enabled_count) {
                    disable_adc();
                }
            }
        }
        if (enabled_count) {
            if (read_ready()) {
                if (channels[active_channel_i].cb) {
                    channels[active_channel_i].cb(
                        channels[active_channel_i].data,
                        adc_id_t(active_channel_i),
                        read_adc_value());
                }
                start_read_next_enabled_channel();
            }
        }
        ptxx_yield();
    }
    ptxx_end();
}

void vla::adc_daemon::start_read_next_enabled_channel()
{
    uint8_t i = uint8_t(next(adc_id_t(active_channel_i)));
    while (!channels[i].cb) {
        i = uint8_t(next(adc_id_t(active_channel_i)));
    }
    active_channel_i = i;
    ADSC_t::set();
}
