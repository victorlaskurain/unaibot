#include <vla/debouncer.hpp>
#include <vla/rt_clock.hpp>
#include <vla/cli_sei.hpp>
#include <vla/serial.hpp>

using namespace vla;

static auto& ser = get_serial_async_debug();
static uint16_t ticks = 0;

template <class debouncer>
class frequency_policy_7ms
{
    rt_clock *clock;
    volatile bool is_cycle_time = false;
    static void on_alarm(void *p)
    {
        // this runs in interrupt context
        ++ticks;
        auto is_cycle_time = static_cast<volatile bool*>(p);
        *is_cycle_time = true;
    }
protected:
    template<class param_t>
    frequency_policy_7ms(param_t&& p)
    {
        clock = p.clock;
    }
    void set_next_alarm()
    {
        is_cycle_time = false;
        clock->set_alarm(period_us{7000}, on_alarm, (void*)&this->is_cycle_time);
    }
public:
    void start()
    {
        set_next_alarm();
    }
    void cycle_if_ready()
    {
        if (is_cycle_time) {
            set_next_alarm();
            static_cast<debouncer*>(this)->cycle();
        }
    }
};

struct debouncer_param {
    rt_clock *clock;
};
class signal_debouncer : public debouncer_base<
    signal_debouncer,
    get_signal_policy_all_pins,
    debounce_policy_integral,
    edge_policy_count_fall<get_signal_policy_all_pins::signal_count()>,
    frequency_policy_7ms>
{
public:
    signal_debouncer(rt_clock *rtc):debouncer_base(debouncer_param{rtc}){}
};

static void write_counters(const signal_debouncer &csd)
{
    write_line(ser, "COUNTERS:");
    for (unsigned i = 0; i < 2; ++i) {
        write_hex(ser, csd.counters()[i]);
        write(ser, " ");
    }
    write_line(ser, "");
}

static void write_counters_if_changed(const signal_debouncer &csd)
{
    static uint16_t counter0 = -1, counter1 = -1;
    if (counter0 != csd.counters()[0] || counter1 != csd.counters()[1]) {
        write_counters(csd);
        counter0 = csd.counters()[0];
        counter1 = csd.counters()[1];
    }
}

static void write_if_pin_value_changed(const signal_debouncer &csd)
{
#ifdef DEBUG_PIN_READ
    static uint8_t pin_value = -1;
    static uint8_t value     = -1;
    if (PORTB0_t::get() != pin_value || (uint8_t)csd.get_value(0) != value) {
        pin_value = PORTB0_t::get();
        value     = (uint8_t)csd.get_value(0);
        write(ser, "\r\nPINB0    : ");
        write_hex(ser, (uint8_t)(PORTB_t::pin_t::ref() & _BV(0)));
        write(ser, "\r\nVALUE    : ");
        write_hex(ser, (uint8_t)csd.get_value(0));
        write(ser, "\r\nPIN VALUE: ");
        write_hex(ser, (uint8_t)csd.get_value(0));
        write(ser, "\r\nENABLED  : ");
        write_hex(ser, (uint8_t)csd.is_enabled(0));
        write(ser, "\r\nTICKS    : ");
        write_hex(ser, ticks);
        write_line(ser, "");
        write_counters(csd);
    }
#endif
}

int main(void)
{
    sei();
    ser.set_speed(serial_speed::BAUD_115200);
    write_line(ser, "BEGIN");
    switch_on_pull_ups<PORTB_t>();
    tick_0a_20khz tick;
    auto clock = rt_clock{tick};
    signal_debouncer csd{&clock};
    csd.set_enabled(0, true);
    csd.set_enabled(1, true);
    write_if_pin_value_changed(csd);
    csd.start();
    while(true){
        csd.cycle_if_ready();
        write_if_pin_value_changed(csd);
        write_counters_if_changed(csd);
    }
}
