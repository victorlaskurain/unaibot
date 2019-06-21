#ifndef VLA_MODBUS_RTU_SLAVE_TAGGED_UNION
#define VLA_MODBUS_RTU_SLAVE_TAGGED_UNION

#include <stdint.h>

/**
 * This header implements a poor man's variant that can be used to
 * express different message types in a safe and convenient way. It
 * depends neither on the standard library not in any typeinfo.
 */

/*
  // example of usage
enum class t1 {
    ONE,
};
enum class t2 {
    ONE,
};
 int main()
{
    tagged_union<int, const char*, t1, t2, const t1*, const t2*> tu;
    tu.data.head = 0;
    tu.data.tail.head = "";
    tu.data.tail.tail.head = t1::ONE;
    int         int_v = -1;
    const char *str_v = "kaixo";
    bool got_value = false;
    got_value = tu.get(int_v);
    std::cout << "Expect 0: " << got_value << "\n";
    std::cout << "Expect -1: " << int_v << "\n";
    tu.put(5);
    got_value = tu.get(int_v);
    std::cout << "Expect 1: " << got_value << "\n";
    std::cout << "Expect 5: " << int_v << "\n";
    tu.put("agur");
    got_value = tu.get(int_v);
    std::cout << "Expect 0: " << got_value << "\n";
    got_value = tu.get(str_v);
    std::cout << "Expect 1: " << got_value << "\n";
    std::cout << "Expect agur: " << str_v << "\n";
    std::cout << "sizeof int: " << sizeof (int) << "\n";
    std::cout << "sizeof tu: " << sizeof tu << "\n";
}
*/

namespace vla {

    /**
     * Implementation template. The member type "type" is a union of
     * all the parameters, thus it is a type that can hold any of the
     * types given as parameters with the proper alignment and minimal
     * size.
     */
    template<typename union_member_first, typename... union_members_rest>
    class recursive_union
    {
    public:
        union type
        {
            union_member_first head;
            typename recursive_union<union_members_rest...>::type tail;
        };
    };
    template<typename union_member_head>
    class recursive_union<union_member_head>
    {
    public:
        union type
        {
            union_member_head head;
        };
    };

    /**
     * Implementation template. Adds get and put functions for each
     * possible value type of the template. Keeps track of the type of
     * current stored value.
     *
     * The first (most general) specialization gets instantiated with
     * the empty union_members_types list.
     */
    template <typename union_t, const uint8_t MY_TAG, typename ... union_members_types>
    class tagged_union_impl
    {
    protected:
        union_t data;
        uint8_t tag = 0;
        // this two are here so that the using
        // parent_t::... declarations of the following specilization
        // don't fail.
        void get();
        void put();
    };
    /**
     * This specialization adds get/put functions and defines the tag
     * for the first type of the type list. Then delegates the rest of
     * the list to a recursive instantiation.
     */
    template<
        typename union_t,
        const uint8_t MY_TAG,
        typename first,
        typename ... union_members_types>
    class tagged_union_impl<union_t, MY_TAG, first, union_members_types...> :
        public tagged_union_impl<union_t, MY_TAG + 1, union_members_types...>
    {
        using parent_t = tagged_union_impl<union_t, MY_TAG + 1, union_members_types...>;
    public:
        using parent_t::get;
        bool get(first &value)
        {
            if (MY_TAG == this->tag) {
                value = *reinterpret_cast<first*>(&this->data);
                return true;
            }
            return false;
        }
        using parent_t::put;
        void put(const first &value)
        {
            *reinterpret_cast<first*>(&this->data) = value;
            this->tag = MY_TAG;
        }
    };

    /**
     * Public template. A tagged_union is a simple variant style
     * type. Initialized without any value by default.
     */
    template<typename... union_member_types>
    class tagged_union : public tagged_union_impl<
        typename recursive_union<union_member_types...>::type,
        1,
        union_member_types...>
    {
    public:
        tagged_union()                    = default;
        tagged_union(const tagged_union&) = default;
        template<typename T>
        tagged_union(const T& v)
        {
            this->put(v);
        }
    };
}

#endif // VLA_MODBUS_RTU_SLAVE_TAGGED_UNION
