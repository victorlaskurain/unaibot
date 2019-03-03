#include <vla/timers.hpp>
#include "find_closest.hpp"

namespace vla {

    void find_closest_tc1(double target_freq,
                          clock_source_1 &best_cs,
                          uint16_t &best_ocr)
    {
        uint8_t scale[] = {
            2, // PRESCALE_1,
            4, // PRESCALE_8,
            7, // PRESCALE_64,
            9, // PRESCALE_256,
            11,// PRESCALE_1024
            0};
        uint8_t scale_i = 0;
        find_closest(target_freq, scale, scale_i, best_ocr);
        best_cs = clock_source_1(scale_i + 1);
    }

}
