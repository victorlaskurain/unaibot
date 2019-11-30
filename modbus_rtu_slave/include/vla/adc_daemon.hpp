#ifndef VLA_MODBUS_RTU_SLAVE_ADC_DAEMON
#define VLA_MODBUS_RTU_SLAVE_ADC_DAEMON

#include <vla/adc_daemon_queue.hpp>
#include <vla/protothreads.hpp>

namespace vla {

    typedef bool(*adc_callback_t)(void *data, adc_id_t id, uint16_t value);
    class adc_daemon : public ptxx_thread
    {
        adc_queue_t &in_q;
        adc_msg_t channels[uint8_t(adc_id_t::ADC7) + 1];
        uint8_t enabled_count    = 0;
        uint8_t active_channel_i = 0; // index of the channel we are currently reading.
        uint16_t last_read       = 0;
        void start_read_next_enabled_channel();
    public:
        adc_daemon(adc_queue_t &in_q):in_q(in_q){}
        void operator()();
    };

}

#endif // VLA_MODBUS_RTU_SLAVE_ADC_DAEMON
