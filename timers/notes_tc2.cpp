#include <vla/timers.hpp>
#include "find_closest.hpp"

namespace vla {

    void find_closest_tc2(double target_freq,
                          clock_source_2 &best_cs,
                          uint8_t &best_ocr)
    {
        uint8_t scale[] = {
            2, // PRESCALE_1,
            4, // PRESCALE_8,
            6, // PRESCALE_32,
            7, // PRESCALE_64,
            8, // PRESCALE_128,
            9, // PRESCALE_256,
            11,// PRESCALE_1024
            0};
        uint8_t scale_i = 0;
        find_closest(target_freq, scale, scale_i, best_ocr);
        best_cs = clock_source_2(scale_i + 1);
    }

}
