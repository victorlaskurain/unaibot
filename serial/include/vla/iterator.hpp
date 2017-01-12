#ifndef VLA_SERIAL_ITERATOR_HPP
#define VLA_SERIAL_ITERATOR_HPP

#ifdef HAVE_STL

#include <iterator>

#else

namespace std {
    template<
        class Category,
        class T> struct iterator
    {
    };

    struct forward_iterator_tag{};
}

#endif

#endif
