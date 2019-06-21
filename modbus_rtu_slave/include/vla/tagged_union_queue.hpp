#ifndef VLA_MODBUS_RTU_SLAVE_TAGGED_UNION_QUEUE
#define VLA_MODBUS_RTU_SLAVE_TAGGED_UNION_QUEUE

#include <vla/tagged_union.hpp>
#include <vla/msg_queue.hpp>
#include <stdint.h>

namespace vla {

    template<typename ValueType, size_t size,
             typename _ReadLock,
             typename _WriteLock = _ReadLock>
    class basic_tagged_union_queue :
        public basic_msg_queue<ValueType, size, _ReadLock, _WriteLock>
    {
    public:
        using parent_t = basic_msg_queue<ValueType, size, _ReadLock, _WriteLock>;
        template<typename Msg>
        bool push(Msg &&m)
        {
            return parent_t::push(m);
        }
        template<typename Msg>
        bool pop(Msg &&msg)
        {
            typename parent_t::ReadLock lock;
            if (!this->_empty() && this->_peek().get(msg)) {
                this->_read_ptr_next();
                return true;
            }
            return false;
        }
        void ignore()
        {
            this->_read_ptr_next();
        }
    };
}

#endif //VLA_MODBUS_RTU_SLAVE_TAGGED_UNION_QUEUE
