#ifndef VLA_COMMAND_COMMAND_HPP
#define VLA_COMMAND_COMMAND_HPP

#include <stdint.h>

namespace vla {

    template <typename ...Ts>
    class command_dispatcher
    {
    public:
        inline uint8_t operator()(const char *str)
        {
            return 1;
        }
    };
    template<typename T, typename ...Ts>
    class command_dispatcher<T, Ts...> {
    public:
        command_dispatcher(T t, const Ts...args)
            :cmd_head(t),
             cmd_rest(args...)
        {
        }
        inline uint8_t operator()(const char *str)
        {
            auto retval = cmd_head(str);
            if (retval != 0) {
                retval = cmd_rest(str);
            }
            return retval;
        }
    private:
        T                          cmd_head;
        command_dispatcher<Ts...> cmd_rest;
    };

    template<typename ...Ts>
    command_dispatcher<Ts...> make_command_dispatcher(const Ts&... args)
    {
        return command_dispatcher<Ts...>{args...};
    }
}

#endif // VLA_COMMAND_COMMAND_HPP
