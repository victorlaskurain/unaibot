#ifndef VLA_CIRCULAR_BUFFER_HPP
#define VLA_CIRCULAR_BUFFER_HPP

#include <vla/iterator.hpp>
#include <stddef.h>

namespace vla {

/*
// programa de pruebas de ejemplo

#include "circular_buffer.hpp"
#include <string>
#include <assert.h>

using namespace std;

int main()
{
    vla::circular_buffer<char, 8> buff;
    assert(buff.empty());
    assert(!buff.full());

    buff.push_back('0');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "0");

    buff.push_back('1');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "01");

    buff.push_back('2');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "012");

    buff.push_back('3');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "0123");

    buff.push_back('4');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "01234");

    buff.push_back('5');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "012345");

    buff.push_back('6');
    assert(!buff.empty());
    assert(buff.full()); // caben size - 1 elementos
    assert(string(buff.begin(), buff.end()) == "0123456");

    assert(buff.pop_front() == '0');
    assert(!buff.empty());
    assert(!buff.full());
    assert(buff.pop_front() == '1');
    assert(string(buff.begin(), buff.end()) == "23456");

    buff.push_back('7');
    assert(!buff.empty());
    assert(!buff.full());
    assert(string(buff.begin(), buff.end()) == "234567");

    buff.push_back('8');
    assert(!buff.empty());
    assert(buff.full()); // caben size - 1 elementos
    assert(string(buff.begin(), buff.end()) == "2345678");
}

 */


    template <typename ValueType, typename size_type, size_type size>
    class circular_buffer_iterator :
        public std::iterator<std::forward_iterator_tag, ValueType>
    {
    public:
        typedef typename std::iterator<std::forward_iterator_tag, ValueType>::reference reference;
        explicit circular_buffer_iterator(ValueType *b, size_type i):base(b), idx(i){}
        circular_buffer_iterator& operator++()
        {
            idx = (idx + 1) % size;
            return *this;
        }
        circular_buffer_iterator operator++(int)
        {
            circular_buffer_iterator retval = *this;
            ++(*this);
            return retval;
        }
        bool operator==(const circular_buffer_iterator &other)
        {
            return base == other.base && idx == other.idx;
        }
        bool operator!=(const circular_buffer_iterator &other)
        {
            return !(*this == other);
        }
        reference operator*()
        {
            return base[idx];
        }
    private:
        ValueType *base;
        size_type     idx;
    };

    template<bool, typename if_true_t, typename if_false_t>
    struct vla_if_else
    {};
    template<typename if_true_t, typename if_false_t>
    struct vla_if_else<true, if_true_t, if_false_t>
    {
        typedef if_true_t type;
    };
    template<typename if_true_t, typename if_false_t>
    struct vla_if_else<false, if_true_t, if_false_t>
    {
        typedef if_true_t type;
    };
    template<typename ValueType, size_t size>
    class circular_buffer
    {
    public:
        typedef typename vla_if_else<size <= UINT8_MAX, uint8_t, size_t>::type size_type;
        typedef ValueType  value_type;
        typedef ValueType* pointer;
        typedef circular_buffer_iterator<      ValueType, size_type, size> iterator;
        typedef circular_buffer_iterator<const ValueType, size_type, size> const_iterator;
        iterator begin()
        {
            return iterator(buffer, data_begin);
        }
        iterator begin() const
        {
            return const_iterator(buffer, data_begin);
        }
        iterator end()
        {
            return iterator(buffer, data_end);
        }
        iterator end() const
        {
            return const_iterator(buffer, data_end);
        }

        bool empty() const
        {
            return data_begin == data_end;
        }
        bool full() const
        {
            return data_begin == (data_end + 1) % size;
        }
        void push_back(const ValueType& v)
        {
            buffer[data_end] = v;
            data_end = (data_end + 1) % size;
        }
        ValueType pop_front()
        {
            ValueType retval = buffer[data_begin];
            data_begin = (data_begin + 1) % size;
            return retval;
        }
        void clear()
        {
            data_begin = 0;
            data_end   = 0;
        }
    private:
        value_type buffer[size];
        volatile size_type data_begin = 0;
        volatile size_type data_end   = 0;
    };
}

#endif
