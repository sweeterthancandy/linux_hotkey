#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <fstream>

#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <boost/program_options.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/log/trivial.hpp>

#include <boost/range/algorithm.hpp>

#include "virtual_keyboard.h"
#include "event_monitor.h"
#include "keyboard_state.h"
#include "pattern_matcher.h"
#include "key_conv.h"

/*
 * Issues:
 *      key repeats
 *
 */


struct driver{
        driver(int argc, char** argv){
                namespace bf = boost::filesystem;
                namespace po = boost::program_options;
                namespace xpr = boost::xpressive;


                bool help;
                std::vector<std::string> cmds;

                po::options_description desc;
                desc.add_options()
                        ("config",po::value<std::string>(),"configuration file to read")
                        ("cmd",po::value<std::vector<std::string> >(&cmds),"commands to use")
                        ("help",po::value<bool>(&help)->default_value(false)->implicit_value(true),
                         "print this message")
                        ;

                po::positional_options_description p;
                p.add("config",-1);

                po::store( 
                        po::command_line_parser(argc,argv)
                                .options(desc)
                                .positional(p)
                                .run()
                       , vm_ 
                );
                po::notify(vm_);

                xpr::sregex config_rgx;
                using xpr::s1;
                std::string seq, map;
                config_rgx = xpr::bos >> *xpr::_s
                        >> "{" 
                                >> (s1=(+xpr::_ ) )           [xpr::ref(seq) = s1 ]
                        >> "}"
                        >> *xpr::_s >> "->" >> *xpr::_s
                        >> "{" 
                                >> (s1=(+xpr::_ ) )           [xpr::ref(map) = s1 ]
                        >> "}" 
                        >> *xpr::_s >> xpr::eos;
                       ; 

                std::stringstream sstr;
                if( vm_.count("config") ){
                        std::string fn = vm_["config"].as<std::string>();
                        if( ! bf::exists( fn ) )
                                BOOST_THROW_EXCEPTION(std::logic_error("config file doesn't exist"));
                        std::ifstream ifstr(fn);
                        if( ! ifstr.is_open() )
                                BOOST_THROW_EXCEPTION(std::logic_error("unable to open config file"));
                        for( std::string line; std::getline(ifstr,line);)
                                sstr << line << "\n";
                } 
                BOOST_LOG_TRIVIAL(error) << cmds.size();
                for( auto const& s : cmds ){
                        sstr << s << "\n";
                }

                key_conv konv;
                for(std::string line; std::getline(sstr,line);){
                        BOOST_LOG_TRIVIAL(error) << "parsing line " << line;
                        xpr::smatch m;
                        auto ret = xpr::regex_search( line, m, config_rgx );
                        if( ret ){
                                //std::cout << boost::format("(%s)(%s)\n") % 
                                        //seq % map;
                                keyboard_state target_state;
                                konv( seq.begin(), seq.end(), [&target_state](__u16 key, bool is_upper){
                                        target_state.down( key );
                                });
                                auto iter = konv( map.begin(), map.end(), [](__u16 key, bool is_upper){
                                        //std::cout << boost::format("(%s,%s)\n")
                                                //% key % is_upper
                                        //;
                                });
                                if( iter != map.end() ){
                                        std::cerr << "failed to parse \"" << map << "\"\n";
                                        continue;
                                }
                                pm_.push( target_state, [this,seq,map](){ 
                                        std::cout << boost::format("dispatching {%s} -> {%s}\n")
                                                % seq % map;
                                        kbd_->from_string( map );
                                        kbd_->sync_flush();
                                } );
                        } else{
                                std::cerr << "unable to parse \"" << line << "\"\n";
                        }
                }

                for(bf::directory_iterator iter("/dev/input/"),end;iter!=end;++iter){
                        auto p = iter->path();
                        auto dev = p.filename().string();
                       
                      
                        if( ! bf::is_directory( iter->path() ) )
                        {
                                auto sptr = std::make_shared<event_monitor>(io_, iter->path().string());
                                sptr->connect( [this](const std::string& dev, const struct input_event& ev){
                                        BOOST_LOG_TRIVIAL(error) << 
                                                dev << " => " 
                                                        << "{"
                                                                << ev.type << ","
                                                                << ev.code << ","
                                                                << ev.value
                                                        << "}";

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
                kbd_ = std::make_shared<virtual_keyboard>("hotkey-kbd");
        }
        int run(){
                io_.run();
        }
private:
        boost::asio::io_service io_;
        std::vector<std::shared_ptr<event_monitor> > monitors_;
        std::shared_ptr<virtual_keyboard> kbd_;
        pattern_matcher pm_;
        keyboard_state state_;
        boost::program_options::variables_map vm_;
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
