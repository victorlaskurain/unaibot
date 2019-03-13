#include <vla/serial.hpp>
#include <vla/rt_clock.hpp>
#include <stdlib.h>
#include <string.h>

using namespace vla;

static auto &ser = get_serial_async_debug();

void on_alarm(const void* msg)
{
    write(ser, "ALARM ");
    write_line(ser, static_cast<const char*>(msg));
}

static const auto after_10_seconds   = period_us{10UL*1000UL*1000UL};
static const auto after_8_seconds    = period_us{ 8UL*1000UL*1000UL};
static const auto after_750_useconds = period_us{             750UL};

int main(int argc, char **argv)
{
    sei();
    tick_0a_20khz tick;
    auto clock = rt_clock{tick};
    const size_t no_cancel = 5;
    write_line(ser, "BEGIN 750us");
    _delay_ms(10);
    clock.set_alarm(after_750_useconds, on_alarm, "750us");
    _delay_ms(10);
    write_line(ser, "BEGIN");
    static_assert(no_cancel < MAX_ALARMS, "Alarm in the middle");
    write_line(ser, "SET AFTER 10\"");
    clock.set_alarm(after_10_seconds, on_alarm, "10\"");
    write_line(ser, "SET ALL AVAILABLE ALARMS AFTER 8\" AND CANCEL ALL BUT ONE");
    alarm_id alarms[MAX_ALARMS];
    for (size_t i = 1; i < MAX_ALARMS; ++i) {
        if (no_cancel == i) { // have one alarm fire earlier then the rest to force moving the alarms
            alarms[i] = clock.set_alarm(after_8_seconds, on_alarm, "8\"");
        } else {
            alarms[i] = clock.set_alarm(after_10_seconds, on_alarm, "WRONG");
        }
        if (!alarms[i]) {
            write_line(ser, "set_alarm FAIL");
        }
    }
    alarm_id no_alarm = clock.set_alarm(after_10_seconds, on_alarm, "WRONG!");
    if (no_alarm) {
        write_line(ser, "set_alarm FAIL");
    }
    for (size_t i = 1; i < MAX_ALARMS; ++i) {
        if (no_cancel == i) {
            continue;
        }
        clock.cancel_alarm(alarms[i]);
    }
    write_line(ser, "All but two alarms cancelled.");
    _delay_ms(10000);
    write_line(ser, "DONE");
    while (true){}
}
