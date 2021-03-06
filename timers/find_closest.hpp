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

}
