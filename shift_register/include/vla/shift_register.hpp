#ifndef VLA_SHIFT_REGISTER_SHIFT_REGISTER_HPP
#define VLA_SHIFT_REGISTER_SHIFT_REGISTER_HPP

#include <avr/io.h>
#include <vla/registers.hpp>
#include <stddef.h>

namespace vla {

    template<typename shift_clk,
             typename store_clk,
             typename output_enable,
             typename ds,
             typename bit_field_t,
             size_t bit_field_size = 8 * sizeof(bit_field_t)>
    struct shift_register
    {
    public:
        shift_register():data(0)
        {
            commit();
            output_enable::set();
            shift_clk::set_mode_output();
            store_clk::set_mode_output();
            output_enable::set_mode_output();
            ds::set_mode_output();
            output_enable::clear();
        }
        ~shift_register()
        {
            commit(0);
        }
        bit_field_t data;
        inline void commit(bit_field_t d)
        {
            data = d;
            commit();
        }
        inline void commit() const
        {
            store_clk::clear();
            for (size_t i = bit_field_size; i > 0; --i) {
                shift_clk::clear();
                if (data & (1 << (i - 1))) {
                    ds::set();
                } else {
                    ds::clear();
                }
                shift_clk::set();
            }
            store_clk::set();
        }
    private:
        shift_register(const shift_register&)                  = delete;
        shift_register(shift_register&&)                       = delete;
        shift_register& operator=(const shift_register &other) = delete;
        shift_register& operator=(shift_register &&other)      = delete;
    };

}

#endif // VLA_SHIFT_REGISTER_SHIFT_REGISTER_HPP
