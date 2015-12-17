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

/*
 * Issues:
 *      key repeats
 *
 */

struct pattern{
        void push(__u16 key){
        }
};

struct pattern_matcher{

        using callback_t = std::function<void()>;

        void push(std::vector<int> keys, callback_t callback){
                boost::sort( keys );
                patterns_.emplace_back( boost::fusion::make_vector( keys, std::move(callback) ) );
        }         
        template<typename Iter>
        void match(Iter first, Iter last){
                using boost::fusion::at_c;

                assert( std::is_sorted(first,last) && "precondition not met" );

                auto sz = std::distance(first,last);
                
                for( auto const& p: patterns_ ){
                        auto const& pat = at_c<0>(p);
                        if( pat.size() == sz ){
                                if( std::equal( first, last , pat.begin() ) ){
                                        at_c<1>(p)();
                                }
                        }
                }
        } 
private:
        std::vector<
                boost::fusion::vector<
                        std::vector<int>, 
                        std::function<void()> 
                > 
        > patterns_;
};

struct driver{
        driver(int argc, char** argv){
                namespace bf = boost::filesystem;

                std::vector<int> keys;
                keys.push_back( KEY_F2 );
                pm_.push( keys, [this](){ 
                        kbd_.parse( "gerry  candy" );
                        std::exit(EXIT_SUCCESS);
                } );

                for(bf::directory_iterator iter("/dev/input/"),end;iter!=end;++iter){
                        auto p = iter->path();
                        auto dev = p.filename().string();
                       
                      
                        if( ! bf::is_directory( iter->path() ) )
                        {
                                auto sptr = std::make_shared<event_monitor>(io_, iter->path().string());
                                sptr->start();
                                sptr->connect( [this](const std::string& dev, const struct input_event& ev){
                                        if(ev.type == EV_KEY){
                                                if( ev.value == 0 ){
                                                        auto iter = keys_.find( ev.code );
                                                        if( iter != keys_.end() )
                                                                keys_.erase( iter );
                                                        
                                                } else if (ev.value == 1 ){
                                                        keys_.insert( ev.code );
                                                }
                                                pm_.match( keys_.begin(), keys_.end() );
                                        }
                                                                        
                                });
                                monitors_.emplace_back(sptr);
                        }
                }
        }
        int run(){
                io_.run();
        }
private:
        boost::asio::io_service io_;
        std::vector<std::shared_ptr<event_monitor> > monitors_;
        virtual_keyboard_parser kbd_;
        pattern_matcher pm_;
        std::set<int> keys_;
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
