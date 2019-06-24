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

#define ptxx_begin()    pt_begin(&ctx)
#define ptxx_end()      pt_end(&ctx)
#define ptxx_wait(cond) pt_wait(&ctx, cond)
#define ptxx_yield()    pt_yield(&ctx)
#define ptxx_exit(stat) pt_exit(&ctx, stat)
#define ptxx_loop(cond) pt_loop(&ctx, cond)

namespace vla {
    class ptxx_thread
    {
    protected:
        pt ctx{};
    public:
        ptxx_thread()                      = default;
        ptxx_thread(ptxx_thread&&)         = default;
        ptxx_thread(const ptxx_thread&)    = delete;
        void operator=(const ptxx_thread&) = delete;
    };
}

#endif // VLA_PROTOTHREADS_PROTOTHREADS_HPP
