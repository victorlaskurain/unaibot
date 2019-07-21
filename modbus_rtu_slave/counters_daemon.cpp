#include "counters_daemon.hpp"
#include <vla/registers.hpp>
#include <vla/debouncer.hpp>

namespace vla {
    class debounced_counters : public debouncer_base<
        debounced_counters,
        get_signal_policy_all_pins,
        debounce_policy_integral,
        edge_policy_count_fall<get_signal_policy_all_pins::signal_count()>,
        frequency_policy_sync>
    {
        using parent_t = debouncer_base<
            debounced_counters,
            get_signal_policy_all_pins,
            debounce_policy_integral,
            edge_policy_count_fall<get_signal_policy_all_pins::signal_count()>,
            frequency_policy_sync>;
        using parent_t::set_enabled;
    public:
        counters_set_enabled_msg_t callbacks[debounced_counters::signal_count];
        debounced_counters()
        {
            for (uint8_t i = 0; i < debounced_counters::signal_count; ++i) {
                callbacks[i] = counters_set_enabled_msg_t(counter_id_t(i), false);
            }
        }
        void set_enabled(const counters_set_enabled_msg_t& msg)
        {
            static_assert(256 > debounced_counters::signal_count, "Bad counter id possible");
            auto i_signal = uint8_t(msg.id);
            callbacks[i_signal] = msg;
            set_enabled(i_signal, msg.enabled);
        }
    };
    static debounced_counters counters;

    static_assert(
        uint8_t(counter_id_t::COUNTER_MAX) <= debounced_counters::signal_count,
        "Too many counters requested");

    counters_daemon::counters_daemon(rt_clock &clock, counters_queue_t &in_q):
        clock(clock),
        in_q(in_q)
    {}

    static constexpr auto DEBOUNCING_PERIOD = period_us{7000};
    static bool is_period_elapsed(rt_clock &clock)
    {
        static auto next_ts = clock.get_current_time() + DEBOUNCING_PERIOD;
        auto now = clock.get_current_time();
        if (next_ts < now) {
            next_ts += DEBOUNCING_PERIOD;
            return true;
        }
        return false;
    }

    void counters_daemon::operator()()
    {
        bool timeout = false;
        counters_set_enabled_msg_t enable_msg;
        counters_set_value_msg_t   value_msg;
        ptxx_begin();
        while (true) {
            ptxx_wait(!in_q.empty() || (timeout = is_period_elapsed(clock)));
            while (!in_q.empty()) {
                if (in_q.pop(enable_msg)) {
                    counters.set_enabled(enable_msg);
                    if (enable_msg.enabled) {
                        ptxx_wait(enable_msg.reply(
                            counters_set_value_msg_t{
                                enable_msg.id,
                                counters.get_counter_value(uint8_t(enable_msg.id))
                            }
                        ));
                    }
                } else if (in_q.pop(value_msg)) {
                    counters.set_value(uint8_t(value_msg.id), value_msg.value);
                }
            }
            if (timeout) {
                counters.do_one();
                for (i = 0; i < counters.signal_count; ++i) {
                    if (counters.is_enabled(i) && counters.is_fall_edge(i)) {
                        cb_data = &counters.callbacks[i];
                        ptxx_wait(cb_data->reply(
                            counters_set_value_msg_t{
                                cb_data->id,
                                counters.get_counter_value(i)
                            }
                        ));
                    }
                }
            }
        }
        ptxx_end();
    }
}
