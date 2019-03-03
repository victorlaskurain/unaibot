#include <vla/registers.hpp>
#include <util/delay.h>

using namespace vla;

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

inline void tone(
    const double duration_ms,
    const double freq_hz)
{
    const double period_ms       = 1 / freq_hz * 1000,
                 halve_period_ms = period_ms / 2;
    double elapsed = 0;
    while (elapsed < duration_ms) {
        PORTB3_t::set();
        _delay_ms(halve_period_ms);
        PORTB3_t::clear();
        _delay_ms(halve_period_ms);
        elapsed += period_ms;
    }
}

constexpr double tempo_bpm()
{
    return 60;
}

constexpr double whole_note_duration_ms()
{
    return 1.0 / (tempo_bpm() / 60.0) * 1000.0;
}

const double
    note_ms_whole     = whole_note_duration_ms() / 1.0,
    note_ms_half      = whole_note_duration_ms() / 2.0,
    note_ms_quarter   = whole_note_duration_ms() / 4.0,
    note_ms_eight     = whole_note_duration_ms() / 8.0,
    note_ms_sixteenth = whole_note_duration_ms() / 16.0;

inline void silence(const double duration_ms) {
    _delay_ms(duration_ms);
}

inline void play_note(
    const double freq_hz,
    const double duration_ms,
    const double silence)
{
    const double period_ms       = 1 / freq_hz * 1000,
                 halve_period_ms = period_ms / 2;
    double elapsed = 0;
    while (elapsed < duration_ms - silence) {
        PORTB3_t::set();
        _delay_ms(halve_period_ms);
        PORTB3_t::clear();
        _delay_ms(halve_period_ms);
        elapsed += period_ms;
    }
    _delay_ms(silence);
}

int main()
{
    PORTB5_t::set_mode_output();
    PORTB3_t::set_mode_output();
    PORTB5_t::set();
    PORTB3_t::clear();
    while (1) {
        // https://upload.wikimedia.org/wikipedia/commons/0/08/Bugle_charge.jpg
        // Unrolling  the  following  loops   results  in  a  call  to
        // play_note for each of the notes in the music sheet.
        const auto s = note_ms_sixteenth / 2;
        uint8_t i, j;
        for (j = 0; j < 2; ++j) {
            for (i = 0; i < 6; ++i) {
                play_note(note_c4, note_ms_eight + note_ms_sixteenth, s);
                play_note(note_c4, note_ms_sixteenth                , s);
            }
            for (i = 0; i < 3; ++i) {
                play_note(note_g4, note_ms_eight                    , s);
                play_note(note_e4, note_ms_eight                    , s);
            }
        }
        play_note(note_c4, note_ms_quarter * 3, s);
        silence(1000);
    }
}
