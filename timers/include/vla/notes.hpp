#ifndef VLA_NOTES_HPP
#define VLA_NOTES_HPP

#include <vla/timers.hpp>

namespace vla {

    void find_closest_tc0(double target_freq,
                          clock_source_0 &best_cs,
                          uint8_t &best_ocr);

    void find_closest_tc1(double target_freq,
                          clock_source_1 &best_cs,
                          uint16_t &best_ocr);

    void find_closest_tc2(double target_freq,
                          clock_source_2 &best_cs,
                          uint8_t &best_ocr);

    const double
        note_0  = 0,
        note_c4 = 261.626,
        note_d4 = 293.665,
        note_e4 = 329.628,
        note_f4 = 369.994,
        note_g4 = 391.995,
        note_a4 = 440,
        note_b4 = 493.883,
        note_c5 = 523.251,
        note_d5 = 587.330,
        note_e5 = 659.255,
        note_f5 = 698.456,
        note_g5 = 783.991;

    template<typename cm_unit>
    void play_note(
        cm_unit &cm,
        const double freq_hz,
        const double duration_ms,
        const double silence)
    {
        using clock_source = typename cm_unit::timer_traits::clock_source;
        using cnt_reg_t    = typename cm_unit::timer_traits::cnt_reg_t;
        clock_source cs;
        cnt_reg_t ocr;
        find_closest_tc2(freq_hz, cs, ocr);
        cm.set_clock(cs);
        cm.set_output_compare(ocr);
    }

}

#endif
