#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/make_vector.hpp>

#include <boost/range/algorithm.hpp>

#include "virtual_keyboard.h"
#include "event_monitor.h"
#include "keyboard_state.h"
#include "pattern_matcher.h"

/*
 * Issues:
 *      key repeats
 *
 */


struct driver{
        driver(int argc, char** argv){
                namespace bf = boost::filesystem;

                keyboard_state tmp;
                tmp.down( KEY_F2 );
                pm_.push( tmp, [this](){ 
                        kbd_->parse( "first<TAB>second" );
                } );

                for(bf::directory_iterator iter("/dev/input/"),end;iter!=end;++iter){
                        auto p = iter->path();
                        auto dev = p.filename().string();
                       
                      
                        if( ! bf::is_directory( iter->path() ) )
                        {
                                auto sptr = std::make_shared<event_monitor>(io_, iter->path().string());
                                sptr->connect( [this](const std::string& dev, const struct input_event& ev){
                                        if(ev.type == EV_KEY){
                                                if( ev.value == 0 ){ // uo
                                                        state_.up( ev.code );
                                                        pm_.match(state_);
                                                } else if (ev.value == 1 ){ // down
                                                        state_.down( ev.code );
                                                        pm_.match(state_);
                                                } else if( ev.value == 2 ){ // down repeated
                                                }
                                        }
                                                                        
                                });
                                sptr->start();
                                monitors_.emplace_back(sptr);
                        }
                }
                kbd_ = std::make_shared<virtual_keyboard_parser>();
        }
        int run(){
                io_.run();
        }
private:
        boost::asio::io_service io_;
        std::vector<std::shared_ptr<event_monitor> > monitors_;
        std::shared_ptr<virtual_keyboard_parser> kbd_;
        pattern_matcher pm_;
        keyboard_state state_;
};

int main(int argc, char** argv){
        try{
                driver d(argc,argv);
                return d.run();
        } catch(const std::exception& e){
                std::cerr << boost::diagnostic_information(e);
                return EXIT_FAILURE;
        }
}
