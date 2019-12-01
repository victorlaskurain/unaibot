#ifndef VLA_RT_CLOCK_HPP
#define VLA_RT_CLOCK_HPP

#include <vla/timers.hpp>
#include <vla/cli_sei.hpp>
#include <avr/interrupt.h>

namespace vla {

    struct period_us
    {
        uint32_t us;
        constexpr explicit period_us(uint32_t us):us(us){}
        period_us operator-(period_us other) const
        {
            return period_us{us - other.us};
        }
    };

    /**
     *  This clock ticks at 20Khz if the CPU frequency is 16Mhz. The
     *  template ensures that the program won't compile unless the CPU
     *  frequency is the required 16Mhz.
     */
    template<class cm_unit_t, uint32_t FREQ=F_CPU>
    class tick_20khz_t
    {
        using timer_t = typename cm_unit_t::timer_t;
        typename cm_unit_t::timer_t timer;
        using clock_source = typename cm_unit_t::timer_t::clock_source;
        cm_unit_t cm;
    public:
        static constexpr period_us period = period_us{50};
        tick_20khz_t(cm_handler_t f = nullptr, void* data = nullptr):cm(timer)
        {
            static_assert(16000000UL == FREQ, "16Mhz CPU required for 20Khz ticks");
            static_assert(tc_id::TC0 == timer_t::id || tc_id::TC2 == timer_t::id, "Only timer 0 and 2 supported.");
            // 16*10^6Khz / 8 / 100 = 20Khz
            timer.set_mode(tc_mode::TOGGLE_ON_COMPARE_MATCH);
            cm.set_clock(clock_source::PRESCALE_8);
            cm.set_output_compare(99);
            set_handler(f, data);
        }
        ~tick_20khz_t()
        {
            cm.set_cm_handler(nullptr);
        }
        void set_handler(cm_handler_t f, void* data)
        {
            cm.set_cm_handler(f, data);
        }
    };
    using tick_0a_20khz = tick_20khz_t<cm_unit_0A, F_CPU>;
    using tick_0b_20khz = tick_20khz_t<cm_unit_0B, F_CPU>;
    using tick_2a_20khz = tick_20khz_t<cm_unit_2A, F_CPU>;
    using tick_2b_20khz = tick_20khz_t<cm_unit_2B, F_CPU>;

    struct timestamp_us
    {
        uint32_t us = 0;
        explicit timestamp_us(uint32_t us):us(us){}
        timestamp_us(const timestamp_us &other):us(other.us)
        {}
        timestamp_us(const volatile timestamp_us &other):us(other.us)
        {}
        timestamp_us& operator+=(period_us delta)
        {
            us += delta.us;
            return *this;
        }
        volatile timestamp_us& operator+=(period_us delta) volatile
        {
            us += delta.us;
            return *this;
        }
        timestamp_us operator+(period_us delta) volatile
        {
            auto newus = *this;
            return newus += delta;
        }
        timestamp_us operator+(period_us delta)
        {
            auto newus = *this;
            return newus += delta;
        }
        bool operator<(timestamp_us other) const
        {
            return us < other.us;
        }
    };
    inline period_us operator-(timestamp_us a, timestamp_us b)
    {
        if (a < b) {
            return period_us(0);
        }
        return period_us(a.us - b.us);
    }

    struct alarm_id
    {
        uint16_t id = 0;
        explicit alarm_id(uint16_t id = 0):id(id){}
        explicit operator bool() const { return id > 0; }
        bool operator==(alarm_id other) const
        {
            return id == other.id;
        }
        bool operator!=(alarm_id other) const
        {
            return !(*this == other);
        }
    };

    using alarm_cb_t       = void(*)(void*);
    using alarm_cb_const_t = void(*)(const void*);
    static constexpr auto MAX_ALARMS = 16;

    class rt_clock
    {
        struct alarm {
            alarm_id id         = alarm_id{0};
            timestamp_us when   = timestamp_us{0};
            alarm_cb_t callback = nullptr;
            void *data          = nullptr;
            alarm() = default;
            alarm(alarm_id id, timestamp_us when, alarm_cb_t callback, void* data)
                :id{id}, when{when}, callback{callback}, data{data}{}
        } alarms[MAX_ALARMS];
        uint8_t alarm_count = 0;
        volatile timestamp_us now = timestamp_us{0};
        const period_us period;
    public:
        template<class tick_t>
        rt_clock(tick_t &tick_generator):period{tick_generator.period}
        {
            tick_generator.set_handler(rt_clock::on_tick, static_cast<void*>(this));
        }
        void cancel_alarm(alarm_id id);
        timestamp_us get_current_time()
        {
            CliSei s;
            return timestamp_us{now};
        }
        // f runs inside the interrupt handler so no loitering
        alarm_id set_alarm(period_us us, alarm_cb_t       f,       void* data = nullptr);
        alarm_id set_alarm(period_us us, alarm_cb_const_t f, const void* data = nullptr);
    private:
        static void on_tick(void* self);
        void on_tick();
    };
}

#endif // VLA_RT_CLOCK_HPP
