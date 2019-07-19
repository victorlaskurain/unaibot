#ifndef VLA_MSG_QUEUE_MSG_QUEUE_HPP
#define VLA_MSG_QUEUE_MSG_QUEUE_HPP

#include <vla/cli_sei.hpp>

#include <stddef.h>

namespace vla {

    // template sorcery to determine the smallest unsigned integer
    // type needed to hold a given value.
    template<bool cond, typename then_t, typename else_t>
    struct if_then_else{};
    template<typename then_t, typename else_t>
    struct if_then_else<true, then_t, else_t>
    {
        using type = then_t;
    };
    template<typename then_t, typename else_t>
    struct if_then_else<false, then_t, else_t>
    {
        using type = else_t;
    };
    template<size_t size>
    struct min_type_for
    {
        using type =
            typename if_then_else<size <= UINT8_MAX,
                uint8_t,
                typename if_then_else<size <= UINT16_MAX,
                    uint16_t,
                    typename if_then_else<size <= UINT32_MAX,
                       uint32_t,
                       uint64_t>::type>::type>::type;
    };

    template<typename ValueType, size_t size,
             // if reads (or writes) are guarantied to be executed in
             // a safe context (for example in an interruption
             // handler), then the corresponding lock can be a nop.
             typename _ReadLock,
             typename _WriteLock = _ReadLock>
    class basic_msg_queue
    {
        static_assert(size > 0, "basic_msg_queue size must be > 0");
        using index_t = typename min_type_for<size>::type;
        ValueType buffer[size];
        index_t read  = 0;
        index_t write = 0;
        bool is_empty = true;
    protected:
        using ReadLock  = _ReadLock;
        using WriteLock = _WriteLock;
        bool _empty()
        {
            return is_empty;
        }
        bool _full()
        {
            return !is_empty && read == write;
        }
        void _read_ptr_next()
        {
            read = (read + 1) % size;
            is_empty = read == write;
        }
        ValueType _pop()
        {
            auto read_old = read;
            _read_ptr_next();
            return buffer[read_old];
        }
        bool _push(const ValueType &v)
        {
            if (_full()) {
                return false;
            }
            buffer[write] = v;
            write = (write + 1) % size;
            is_empty = false;
            return true;
        }
        ValueType& _peek()
        {
            return buffer[read];
        }
    public:
        bool empty()
        {
            ReadLock lock;
            return _empty();
        }
        bool full()
        {
            WriteLock lock;
            return _full();
        }
        ValueType pop()
        {
            ReadLock lock;
            return _pop();
        }
        bool pop(ValueType &v)
        {
            ReadLock lock;
            if (_empty()) {
                return false;
            }
            v = _pop();
            return true;
        }
        bool push(const ValueType &v)
        {
            WriteLock lock;
            return _push(v);
        }
    };

    template<typename ValueType, size_t size>
    using msg_queue = basic_msg_queue<ValueType, size, CliSei>;
    template<typename ValueType, size_t size>
    using msg_queue_fast = basic_msg_queue<ValueType, size, NopLock>;

    /**
     * This class can be used to easily define messages with require a
     * reply in a given queue.
     *
     * For example, let's say that we have a service that reports the
     * current temperature as a float when requested. This service
     * could declare that it receives the get_temperature message and
     * replies to it with the temperature message this way:
     *
     * struct temperature_msg_t {
     *     float temp;
     * };
     * struct get_temperature_msg_t : public message_with_reply<temperature_msg_t>
     * {
     *     template<typename ReplyQueue>
     *     get_temperature_msg_t(ReplyQueue &q):
     *         message_with_reply<adc_set_value_msg_t>(q){}
     * }
     */
    template<typename MsgReply>
    class message_with_reply
    {
        typedef bool(*reply_callback_t)(void *queue, const MsgReply &reply);
        reply_callback_t cb    = nullptr;
        void            *queue = nullptr;
    public:
        message_with_reply() = default;
        template<typename ReplyQueue>
        message_with_reply(ReplyQueue &q)
        {
            queue = &q;
            cb = [](void *q, const MsgReply &reply) {
                ReplyQueue *queue = static_cast<ReplyQueue*>(q);
                if (queue->full()) {
                    return false;
                }
                queue->push(reply);
                return true;
            };
        }
        template<typename ConvertibleToMsgReply>
        bool reply(const ConvertibleToMsgReply &reply)
        {
            return cb && queue && cb(queue, reply);
        }
    };
}

#endif
