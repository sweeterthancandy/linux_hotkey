#pragma once

#include "keyboard_state.h"

struct pattern_matcher{

        using callback_t = std::function<void()>;

        void push(const keyboard_state& state, callback_t callback){
                patterns_.emplace_back( boost::fusion::make_vector( state, std::move(callback) ) );
        }         
        void match(const keyboard_state& state){
                using boost::fusion::at_c;
                std::cout << "trying to match " << state << "\n";
                for( auto const& p: patterns_ ){
                        auto const& pat = at_c<0>(p);
                        std::cout << "pat = " << pat << "\n";
                        if( pat == state ){
                                at_c<1>(p)();
                        }
                }
        } 
private:
        std::vector<
                boost::fusion::vector<
                        keyboard_state,
                        std::function<void()> 
                > 
        > patterns_;
};
