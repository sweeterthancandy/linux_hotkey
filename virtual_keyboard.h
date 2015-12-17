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

/*
 * point of this is to avoid boiler plate details,
 * abstructing the linux macros
 *
 */
struct virtual_keyboard{
        using backend_t = virtual_keyboard_detail::virtual_keyboard_backend;

        explicit virtual_keyboard(const std::string& name);
        void graph_or_space(char c);
        void tab();
        void sync_flush();
private:
        void shift_();
        void unshift_();
        void press_(__u16 key);
        std::shared_ptr<backend_t>   backend_;
};


/*
 * takes a string and executes them
 */
struct virtual_keyboard_parser{

        boost::xpressive::sregex rgx_;
        boost::xpressive::sregex tab_;

        virtual_keyboard_parser();
        void parse(std::string const& cmd);
private:
        bool try_parse_single_char_(char c);

        virtual_keyboard kbd_;
};
