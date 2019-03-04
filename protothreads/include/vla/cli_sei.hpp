#ifndef VLA_CLI_SEI_HPP
#define VLA_CLI_SEI_HPP

#include <util/atomic.h>

namespace vla {
    struct CliSei {
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

}
#endif
