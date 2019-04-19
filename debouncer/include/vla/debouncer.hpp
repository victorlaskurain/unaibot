#ifndef VLA_DEBOUNCER_HPP
#define VLA_DEBOUNCER_HPP

#include <vla/registers.hpp>
#include <stdlib.h>

namespace vla {

    struct signal_state
    {
        static constexpr uint8_t COUNTER_MAX = 3;
        bool    enabled:1;
        uint8_t current_value:1;
        uint8_t previous_value:1;
        uint8_t read_value:1;
        uint8_t counter:2;
        signal_state()
        {
            enabled        = false;
            current_value  = 0;
            previous_value = 0;
            read_value     = 0;
            counter        = 0;
        }
    };
    static_assert(sizeof(signal_state) == 1, "Signal state overflow");

    class get_signal_policy_portb
    {
    public:
        static constexpr size_t signal_count()
        {
            return 8 * sizeof (PORTB_t::ref());
        }
    protected:
        get_signal_policy_portb() = default;
        template<typename param_t>
        get_signal_policy_portb(param_t&& param = param_t{}){}
        void read_signals(signal_state *signals, size_t signal_count)
        {
            for (int8_t i = 0; i < 8; ++i) {
                if (signals[i].enabled) {
                    signals[i].read_value = bool(PORTB_t::pin_t::ref() & _BV(i));
                }
            }
        }
    };

    class get_signal_policy_all_pins
    {
    public:
        static constexpr size_t signal_count()
        {
            return 8 * (
                sizeof (PORTB_t::ref()) +
                sizeof (PORTC_t::ref()) +
                sizeof (PORTD_t::ref()));
        }
    protected:
        get_signal_policy_all_pins() = default;
        template<typename param_t>
        get_signal_policy_all_pins(param_t&& param = param_t{}){}
        void read_signals(signal_state *signals, size_t signal_count)
        {
            for (int8_t i = 0; i < 8; ++i) {
                if (signals[0  + i].enabled) {
                    signals[0  + i].read_value = bool(PORTB_t::pin_t::ref() & _BV(i));
                }
            }
            for (int8_t i = 0; i < 8; ++i) {
                if (signals[8  + i].enabled) {
                    signals[8  + i].read_value = bool(PORTC_t::pin_t::ref() & _BV(i));
                }
            }
            for (int8_t i = 0; i < 8; ++i) {
                if (signals[16 + i].enabled) {
                    signals[16 + i].read_value = bool(PORTD_t::pin_t::ref() & _BV(i));
                }
            }
        }
    };

    class debounce_policy_integral
    {
    protected:
        debounce_policy_integral() = default;
        template<class param_t>
        debounce_policy_integral(param_t&& p){}
        void debounce(signal_state *signals, size_t signal_count)
        {
            for (size_t i = 0; i < signal_count; ++i) {
                auto &s = signals[i];
                if (!s.enabled) {
                    continue;
                }
                s.previous_value = s.current_value;
                if (s.read_value) {
                    if (s.counter < signal_state::COUNTER_MAX) {
                        ++s.counter;
                    }
                    if (signal_state::COUNTER_MAX == s.counter) {
                        s.current_value = 1;
                    }
                } else {
                    if (s.counter > 0) {
                        --s.counter;
                    }
                    if (0 == s.counter) {
                        s.current_value = 0;
                    }
                }
            }
        }
    };

    template <class debouncer>
    class frequency_policy_sync
    {
    protected:
        frequency_policy_sync() = default;
        template<class param_t>
        frequency_policy_sync(param_t&& p){}
    public:
        void do_one()
        {
            static_cast<debouncer*>(this)->cycle();
        }
    };

    class edge_policy_none
    {
    protected:
        edge_policy_none() = default;
        template<class param_t>
        edge_policy_none(param_t&& p){}
        void detect_edge(signal_state *signals, size_t signal_count)
        {
        }
    };

    template<uint8_t signal_count>
    class edge_policy_count_fall
    {
        uint16_t _counters[signal_count];
    protected:
        edge_policy_count_fall()
        {
            for (auto i = 0; i < signal_count; ++i) {
                _counters[i] = 0;
            }
        };
        template<class param_t>
        edge_policy_count_fall(param_t&& p){}
        void detect_edge(signal_state *signals, size_t c)
        {
            for (size_t i = 0; i < signal_count && i < c; ++i) {
                if (signals[i].enabled) {
                    if (0 == signals[i].current_value &&
                        1 == signals[i].previous_value) {
                        ++_counters[i];
                    }
                }
            }
        }
    public:
        constexpr size_t number_of_counters()
        {
            return signal_count;
        }
        const uint16_t* counters() const
        {
            return _counters;
        }
    };

    template <class concrete,
              class get_signal_policy,
              class debounce_policy,
              class edge_policy,
              template<class c> class frequency_policy>
    class debouncer_base : public get_signal_policy,
                           public debounce_policy,
                           public edge_policy,
                           public frequency_policy<concrete>
    {
        static constexpr auto signal_count = get_signal_policy::signal_count();
        signal_state signals[get_signal_policy::signal_count()];
    public:
        debouncer_base() = default;
        template<typename param_t>
        debouncer_base(param_t&& param)
            :get_signal_policy{param},
             frequency_policy<concrete>{param}
        {}
        void cycle()
        {
            this->read_signals(signals, signal_count);
            this->debounce(signals, signal_count);
            this->detect_edge(signals, signal_count);
        }
        bool get_value(uint8_t i_signal) const
        {
            return signals[i_signal].current_value;
        }
        bool get_pin_value(uint8_t i_signal) const
        {
            return signals[i_signal].read_value;
        }
        bool is_enabled(uint8_t i_signal) const
        {
            return signals[i_signal].enabled;
        }
        void set_enabled(uint8_t i_signal, bool enable)
        {
            signals[i_signal].enabled = enable;
        }
    };

}

#endif // VLA_DEBOUNCER_HPP
