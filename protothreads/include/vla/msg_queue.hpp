#ifndef VLA_MSG_QUEUE_MSG_QUEUE_HPP
#define VLA_MSG_QUEUE_MSG_QUEUE_HPP

#include <stddef.h>
#include <util/atomic.h>

namespace vla {

    template<typename ValueType, size_t size, typename Lock>
    class basic_msg_queue
    {
        ValueType buffer[size];
        unsigned int read  = 0;
        unsigned int write = 0;
        bool _empty()
        {
            return write == read;
        }
        bool _full()
        {
            return (write + 1) % size == read;
        }
        ValueType _pop()
        {
            auto read_old = read;
            read = (read + 1) % size;
            return buffer[read_old];
        }
        bool _push(const ValueType &v)
        {
            if (_full()) {
                return false;
            }
            buffer[write] = v;
            write = (write + 1) % size;
            return true;
        }
    public:
        bool empty()
        {
            Lock lock;
            return _empty();
        }
        bool full()
        {
            Lock lock;
            return _full();
        }
        ValueType pop()
        {
            Lock lock;
            return _pop();
        }
        bool pop(ValueType &v)
        {
            Lock lock;
            if (_empty()) {
                return false;
            }
            v = _pop();
            return true;
        }
        bool push(const ValueType &v)
        {
            Lock lock;
            return _push(v);
        }
    };

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
    template<typename ValueType, size_t size>
    using msg_queue = basic_msg_queue<ValueType, size, CliSei>;
    template<typename ValueType, size_t size>
    using msg_queue_fast = basic_msg_queue<ValueType, size, int>;

}

#endif
