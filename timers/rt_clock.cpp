#include <vla/rt_clock.hpp>
#include <vla/cli_sei.hpp>
#include <string.h>

namespace vla {

    void rt_clock::on_tick(void* self)
    {
        static_cast<rt_clock*>(self)->on_tick();
    }

    void rt_clock::on_tick()
    {
        // this runs inside the interrupt controller so no
        // synchronization is required.
        now += period;
        decltype(alarm_count) i = 0;
        while (i < alarm_count && alarms[i].when < now) {
            auto &a = alarms[i];
            a.callback(a.data);
            ++i;
        }
        if (i > 0) {
            memmove(&alarms[0], &alarms[i], (alarm_count - i) * sizeof(alarms[0]));
        }
        alarm_count -= i;
    }

    void rt_clock::cancel_alarm(alarm_id id)
    {
        CliSei lock;
        decltype(alarm_count) i = 0;
        while (i < alarm_count && alarms[i].id != id) {
            ++i;
        }
        if (i < alarm_count) {
            memmove(&alarms[i], &alarms[i + 1], (alarm_count - i - 1) * sizeof(alarms[0]));
        }
        --alarm_count;
    }

    static alarm_id next_id = alarm_id{0};
    alarm_id rt_clock::set_alarm(period_us us, alarm_cb_t f, void* data)
    {
        CliSei lock;
        if (alarm_count == MAX_ALARMS || nullptr == f) {
            return alarm_id{0};
        }
        // find position of new alarm
        auto when = now + us;
        decltype(alarm_count) i = 0;
        while (i < alarm_count && alarms[i].when < when) {
            ++i;
        }
        // make space if necessary
        if (i < alarm_count) {
            memmove(&alarms[i + 1], &alarms[i], (alarm_count - i) * sizeof(alarms[0]));
        }
        // store new alarm and return its id
        ++next_id.id;
        alarms[i] = alarm{next_id, when, f, data};
        ++alarm_count;
        return next_id;
    }

    alarm_id rt_clock::set_alarm(period_us us, alarm_cb_const_t f, const void* data)
    {
        return set_alarm(us, reinterpret_cast<alarm_cb_t>(f), const_cast<void*>(data));
    }

}
