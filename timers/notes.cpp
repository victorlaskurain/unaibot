#include <vla/timers.hpp>
#include <math.h>
#include <float.h>

namespace vla {

    template<typename uint>
    void find_closest(double target_freq,
                      uint8_t *scale,
                      uint8_t &best_scale_i,
                      uint &best_ocr)
    {
        double test_freq, current_error, best_error = DBL_MAX;
        uint test_ocr;
        for (uint8_t i = 0; scale[i]; ++i) {
            test_ocr      = uint(round(double(F_CPU>>scale[i]) / target_freq));
            test_freq     = double(F_CPU >> scale[i]) / double(test_ocr);
            current_error = fabs(target_freq - test_freq);
            if (current_error < best_error) {
                best_ocr     = test_ocr;
                best_error   = current_error;
                best_scale_i = i;
            }
        }
    }

    void find_closest_tc0(double target_freq,
                          clock_source_2 &best_cs,
                          uint8_t &best_ocr)
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
        best_cs = clock_source_2(scale_i + 1);
    }

    void find_closest_tc1(double target_freq,
                          clock_source_2 &best_cs,
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
        best_cs = clock_source_2(scale_i + 1);
    }

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
