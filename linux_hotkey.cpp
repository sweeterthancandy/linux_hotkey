#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>

#include "virtual_keyboard.h"

struct driver{
        driver(int argc, char** argv){
                kbd_ = std::make_unique<virtual_keyboard_backend>("virtual-kbd");
        }
        int run(){
                for(;;){
                        kbd_->put( EV_KEY, KEY_A, 1 );
                        kbd_->sync();
                        kbd_->flush();

                        using namespace std::chrono_literals;

                        std::this_thread::sleep_for( 1s );

                        kbd_->put( EV_KEY, KEY_A, 0 );
                        kbd_->sync();
                        kbd_->flush();
                        
                        std::this_thread::sleep_for( 1s );
                }
        }
private:
        std::unique_ptr<virtual_keyboard_backend> kbd_;
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
