#pragma once

#include <string>
#include <memory>
#include <linux/input.h>
#include <boost/xpressive/xpressive.hpp>

namespace virtual_keyboard_detail{

        /*
         * point of this is to simply implement the uinput interface
         */
        struct virtual_keyboard_backend;
}

struct key_conv;

/*
 * point of this is to avoid boiler plate details,
 * abstructing the linux macros
 *
 */
struct virtual_keyboard{
        using backend_t = virtual_keyboard_detail::virtual_keyboard_backend;

        explicit virtual_keyboard(const std::string& name);
        void graph_or_space(char c);
        void from_string(const std::string& s);
        void tab();
        void sync_flush();
private:
        void shift_();
        void unshift_();
        void press_(__u16 key, bool is_upper);
        std::shared_ptr<backend_t>   backend_;
        std::shared_ptr<key_conv> kconv_;
};


