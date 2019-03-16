#ifndef VLA_CLI_SEI_HPP
#define VLA_CLI_SEI_HPP

#include <util/atomic.h>

namespace vla {
    struct Cli
    {
        Cli()
        {
            cli();
        }
        ~Cli()
        {
            sei();
        }
    };
    struct CliSei
    {
        uint8_t sreg_restore;
        CliSei()
        {
            sreg_restore = SREG;
            cli();
        }
        ~CliSei()
        {
            SREG = sreg_restore;
        }
    };

    struct NopLock
    {
        // leaving this out produces a variable defined but not used
        // warning in msg_queue_t::push
        NopLock() = default;
    };
}
#endif
