#ifndef VLA_PROTOTHREADS_PROTOTHREADS_HPP
#define VLA_PROTOTHREADS_PROTOTHREADS_HPP

#include <vla/msg_queue.hpp>
#include <avr/interrupt.h>
#include <pt.h>
#ifdef pt_queue
#    undef pt_queue
#    undef pt_queue_init
#    undef pt_queue_len
#    undef pt_queue_cap
#    undef pt_queue_empty
#    undef pt_queue_full
#    undef pt_queue_reset
#    undef pt_queue_push
#    undef pt_queue_peek
#    undef pt_queue_pop
#endif

namespace vla {
}

#endif // VLA_PROTOTHREADS_PROTOTHREADS_HPP
