#ifndef VLA_TIMERS_HPP
#define VLA_TIMERS_HPP

#include <vla/registers.hpp>
#include <avr/interrupt.h>

namespace vla {
    enum class clock_source_0
    {
        STOP,
        PRESCALE_1,
        PRESCALE_8,
        PRESCALE_64,
        PRESCALE_256,
        PRESCALE_1024,
        EXTERNAL_CLOCK_T0_FALLING,
        EXTERNAL_CLOCK_T0_RISING
    };

    using clock_source_1 = clock_source_0;

    enum class clock_source_2
    {
        STOP,
        PRESCALE_1,
        PRESCALE_8,
        PRESCALE_32,
        PRESCALE_64,
        PRESCALE_128,
        PRESCALE_256,
        PRESCALE_1024
    };

    enum tc_id {
        TC0,
        TC1,
        TC2
    };

    enum class tc_mode : uint8_t {
        NORMAL                  = 0,
        PHASE_CORRECT_PWM       = _BV(WGM00),
        TOGGLE_ON_COMPARE_MATCH = _BV(WGM01)
    };

    template<tc_id tcid>
    struct timer_counter_traits {
    };
    template<>
    struct timer_counter_traits<tc_id::TC0> {
        using ctrl_reg_a   = TCCR0A_t;
        using ctrl_reg_b   = TCCR0B_t;
        using int_mask_reg = TIMSK0_t;
        using cnt_reg      = TCNT0_t;
        using clock_source = clock_source_0;
        using cnt_reg_t    = uint8_t;
    };
    template<>
    struct timer_counter_traits<tc_id::TC1> {
        using ctrl_reg_a   = TCCR1A_t;
        using ctrl_reg_b   = TCCR1B_t;
        using int_mask_reg = TIMSK1_t;
        using cnt_reg      = TCNT1_t;
        using clock_source = clock_source_1;
        using cnt_reg_t    = uint16_t;
    };
    template<>
    struct timer_counter_traits<tc_id::TC2> {
        using ctrl_reg_a   = TCCR2A_t;
        using ctrl_reg_b   = TCCR2B_t;
        using int_mask_reg = TIMSK2_t;
        using cnt_reg      = TCNT2_t;
        using clock_source = clock_source_2;
        using cnt_reg_t    = uint8_t;
    };

    template<tc_id tid>
    class timer_counter
    {
    public:
        const static tc_id id = tid;
        using ctrl_reg_a   = typename timer_counter_traits<tid>::ctrl_reg_a;
        using ctrl_reg_b   = typename timer_counter_traits<tid>::ctrl_reg_b;
        using int_mask_reg = typename timer_counter_traits<tid>::int_mask_reg;
        using cnt_reg      = typename timer_counter_traits<tid>::cnt_reg;
        using clock_source = typename timer_counter_traits<tid>::clock_source;
        using cnt_reg_t    = typename timer_counter_traits<tid>::cnt_reg_t;

        inline timer_counter(
            tc_mode mode         = tc_mode::NORMAL,
            clock_source clk_src = clock_source::STOP)
        {
            set_mode(mode);
            set_clock(clk_src);
        }
        ~timer_counter()
        {
            set_mode(tc_mode::NORMAL);
            set_clock(clock_source::STOP);
        }
        timer_counter(const timer_counter&&)           = delete;
        timer_counter(const timer_counter&)            = delete;
        timer_counter& operator=(const timer_counter&) = delete;
        void set_mode(tc_mode mode)
        {
            switch (mode) {
            case tc_mode::PHASE_CORRECT_PWM:
                pin_t<ctrl_reg_a, 0>::set();
                pin_t<ctrl_reg_a, 1>::clear();
                break;
            case tc_mode::TOGGLE_ON_COMPARE_MATCH:
                pin_t<ctrl_reg_a, 0>::clear();
                pin_t<ctrl_reg_a, 1>::set();
                break;
            case tc_mode::NORMAL:
            default:
                pin_t<ctrl_reg_a, 0>::clear();
                pin_t<ctrl_reg_a, 1>::clear();
            }
        }
        void set_clock(clock_source cs)
        {
            ctrl_reg_b::ref() = static_cast<uint8_t>(cs);
        }
    };

    enum cm_id {
        A,
        B
    };

    enum class cm_mode : uint8_t {
        DISCONNECTED,
        TOGGLE_ON_COMPARE_MATCH,
        CLEAR_ON_COMPARE_MATCH,
        SET_ON_COMPARE_MATCH
    };

    template<tc_id tcid, cm_id cmid>
    struct cm_unit_traits
    {
    };
    template<>
    struct cm_unit_traits<tc_id::TC0, cm_id::A>
    {
        using ctrl_reg_a =
            typename timer_counter_traits<tc_id::TC0>::ctrl_reg_a;
        using int_enable_bit = OCIE0A_t;
        using oc_reg = OCR0A_t;
        static const int bit0 = 6;
        static const int bit1 = 7;
    };
    template<>
    struct cm_unit_traits<tc_id::TC0, cm_id::B>
    {
        using ctrl_reg_a =
            typename timer_counter_traits<tc_id::TC0>::ctrl_reg_a;
        using int_enable_bit = OCIE0B_t;
        using oc_reg = OCR0B_t;
        static const int bit0 = 4;
        static const int bit1 = 5;
    };
    template<>
    struct cm_unit_traits<tc_id::TC2, cm_id::A>
    {
        using ctrl_reg_a =
            typename timer_counter_traits<tc_id::TC2>::ctrl_reg_a;
        using int_enable_bit = OCIE2A_t;
        using oc_reg = OCR2A_t;
        static const int bit0 = 6;
        static const int bit1 = 7;
    };

    template<>
    struct cm_unit_traits<tc_id::TC2, cm_id::B>
    {
        using ctrl_reg_a =
            typename timer_counter_traits<tc_id::TC2>::ctrl_reg_a;
        using int_enable_bit = OCIE2B_t;
        using oc_reg = OCR2B_t;
        static const int bit0 = 4;
        static const int bit1 = 5;
    };

    using cm_handler_t = void(*)(void*);

    template<tc_id tcid, cm_id cmid>
    class cm_unit
    {
    public:
        using cm_traits    = cm_unit_traits<tcid, cmid>;
        using timer_traits = timer_counter_traits<tcid>;
        using clock_source = typename timer_traits::clock_source;
        using ctrl_reg     = typename cm_traits::ctrl_reg_a;
        using oc_reg       = typename cm_traits::oc_reg;
        using oc_reg_t     = typename timer_traits::cnt_reg_t;
        using timer_t      = timer_counter<tcid>;
    private:
        timer_counter<tcid> &timer;
        void set_cm_handler_ptr(cm_handler_t f, void* data);
    public:
        inline cm_unit(
            timer_counter<tcid> &t,
            cm_mode mode = cm_mode::DISCONNECTED)
            :timer(t)
        {
            set_mode(mode);
        }
        inline ~cm_unit()
        {
            set_mode(cm_mode::DISCONNECTED);
        }
        inline void set_cm_handler(cm_handler_t f, void* data = nullptr)
        {
            if (f) {
                set_cm_handler_ptr(f, data);
                cm_traits::int_enable_bit::set();
            } else {
                cm_traits::int_enable_bit::clear();
                set_cm_handler_ptr(nullptr, nullptr);
            }
        }
        inline void set_mode(cm_mode mode)
        {
            ctrl_reg::ref() = (ctrl_reg::ref() & ~(0x03<<cm_traits::bit0))
                | (static_cast<uint8_t>(mode)<<cm_traits::bit0);
        }
        inline void set_clock(clock_source cs)
        {
            timer.set_clock(cs);
        }
        inline void set_output_compare(oc_reg_t v)
        {
            oc_reg::ref() = v;
        }
        inline oc_reg_t get_output_compare()
        {
            return oc_reg::ref();
        }
    };

    using cm_unit_0A = cm_unit<tc_id::TC0, cm_id::A>;
    using cm_unit_0B = cm_unit<tc_id::TC0, cm_id::B>;
    // :TODO: add traits for the following two devices
    // using cm_unit_1A = cm_unit<tc_id::TC1, cm_id::A>;
    // using cm_unit_1B = cm_unit<tc_id::TC1, cm_id::B>;
    using cm_unit_2A = cm_unit<tc_id::TC2, cm_id::A>;
    using cm_unit_2B = cm_unit<tc_id::TC2, cm_id::B>;

}

#endif
