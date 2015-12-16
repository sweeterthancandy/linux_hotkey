#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>

#include "virtual_keyboard.h"

struct driver{
        driver(int argc, char** argv){
        }
        int run(){
                using namespace std::chrono_literals;
                //std::this_thread::sleep_for( 1s );
                kbd_.parse( "gerry<TAB>candy" );
                //std::this_thread::sleep_for( 1s );
        }
private:
        virtual_keyboard_parser kbd_;
};

int main(int argc, char** argv){
        try{
                driver d(argc,argv);
                return d.run();
        } catch(const boost::exception& e){
                std::cerr << boost::diagnostic_information(e);
                return EXIT_FAILURE;
        } catch(const std::exception& e){
                std::cerr << boost::diagnostic_information(e);
                return EXIT_FAILURE;
        }
}
