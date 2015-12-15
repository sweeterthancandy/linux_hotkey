#pragma once

#include <string>

#include <boost/asio.hpp>

#include <linux/input.h>
#include <linux/uinput.h>

#include <boost/xpressive/xpressive.hpp>

namespace virtual_keyboard_detail{

        /*
         * point of this is to simply implement the uinput interface
         */
        struct virtual_keyboard_backend{
                explicit virtual_keyboard_backend(const std::string& name)
                {
                        
                        fd_ = open("/dev/uinput", O_WRONLY | O_NONBLOCK );
                        if( fd_ < 0 )
                                BOOST_THROW_EXCEPTION(std::domain_error("unable to open /dev/uinput"));
                        ioctl( fd_, UI_SET_EVBIT, EV_KEY );
                        ioctl( fd_, UI_SET_EVBIT, EV_SYN );
                        ioctl( fd_, UI_SET_EVBIT, EV_MSC );

                        for(int i=0;i!=KEY_MAX;++i)
                                ioctl( fd_, UI_SET_KEYBIT, i );

                        ioctl(fd_, UI_SET_MSCBIT, MSC_SCAN );


                        struct uinput_user_dev uidev;
                        memset(&uidev, 0, sizeof(uidev));
                        snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, name.c_str());
                        uidev.id.bustype = BUS_USB;
                        uidev.id.vendor  = 1;
                        uidev.id.product = 1;
                        uidev.id.version = 4;
                        write(fd_, &uidev, sizeof(uidev));
                        
                        if( ioctl(fd_, UI_DEV_CREATE ) < 0 )
                                BOOST_THROW_EXCEPTION(std::domain_error("unable to create"));

                }
                ~virtual_keyboard_backend(){
                        ioctl(fd_, UI_DEV_DESTROY );
                }
                void put(__u16 type, __u16 code, __u16 value){
                        assert( code < KEY_MAX && "invalid code");
                        struct input_event ev;
                        memset(&ev,0,sizeof(ev));
                        gettimeofday(&ev.time, NULL );
                        ev.type = type;
                        ev.code = code;
                        ev.value = value;
                        buffer_.push_back(ev);
                }
                void sync(){
                        put( EV_SYN, SYN_REPORT, 0 );
                }
                void flush(){
                        if( buffer_.size() ){
                                buffer_.push_back( {0,0,0} );
                                int to_write = buffer_.size() * sizeof(struct input_event);
                                int wrote = write( fd_, static_cast<struct input_event*>(&buffer_.front()),to_write);
                                if( to_write != wrote )
                                        BOOST_THROW_EXCEPTION(std::domain_error("bad write"));

                                buffer_.clear();
                        }
                }
        private:
                int fd_{-1};
                std::vector<struct input_event> buffer_;
        };
        
        /*
         * point of this is to avoid boiler plate details,
         * abstructing the linux macros
         *
         */
        struct virtual_keyboard{

                explicit virtual_keyboard(const std::string& name)
                        : backend_( name )
                {}
                void esc();
                void backspace();
        private:
                virtual_keyboard_backend backend_;
        };

        struct command_buffer{
                void put(__u16 type, __u16 code, __u16 value){
                        assert( code < KEY_MAX && "invalid code");
                        struct input_event ev;
                        memset(&ev,0,sizeof(ev));
                        gettimeofday(&ev.time, NULL );
                        ev.type = type;
                        ev.code = code;
                        ev.value = value;
                        buffer_.push_back(ev);
                }
        private:
                std::vector<struct input_event> buffer_;
        };

        namespace xpr = boost::xpressove;

        namespace semantics{
                struct tab_impl{
                        using result_type = void;
                        void operator()(command_buffer& buf)const{
                        }
                }

                #define SEMANTIC_FUNCTIONS\
                        (tab_impl)
                #define AUX(r,data,elem)  const boost::xpressive::function<BOOST_PP_CAT(elem,_impl)>::type elem = {{}};
                BOOST_PP_SEQ_FOR_EACH( AUX, ~, SEMANTIC_FUNCTIONS )
                #undef AUX

        }

        /*
         * takes a string and executes them
         */
        struct virtual_keyboard_parser{

                xpr::sregex rgx_;
                xpr::sregex tab_;

                virtual_keyboard_parser()
                {
                        tab_ = xpr::as_xpr("<TAB>") [semantics::tab(xpr::ref(cmd_buffer_))];
                        rgx_ = xpr::bos >> tab_;
                }
                void parse(std::string const& cmd){
                        for(auto iter=cmd.begin(),end(cmd.end());iter!=end;++iter){

                                // parse token
                                
                                if( std::is_space(*iter))
                                        continue;
                                
                                if( try_parse_single_char(*iter) )
                                        continue;

                                else if ( xpr::regex_search( iter, end, rgx_ ) )
                                        continue;
                                else
                                        BOOST_THROW_EXCEPTION(std::domain_error("bad char " + std::string(1,*iter)));


                        }
                }
        private:
                bool try_parse_single_char(char c){
                        #define VIRTUAL_KBD_char_mapping\
                                (('a')(KEY_A)(0))\
                                (('A')(KEY_A)(1))\
                                (('b')(KEY_B)(0))\
                                (('B')(KEY_B)(1))\
                                (('c')(KEY_C)(0))\
                                (('C')(KEY_C)(1))\
                                (('d')(KEY_D)(0))\
                                (('D')(KEY_D)(1))\
                                (('e')(KEY_E)(0))\
                                (('E')(KEY_E)(1))\
                                (('f')(KEY_F)(0))\
                                (('F')(KEY_F)(1))\
                                (('g')(KEY_G)(0))\
                                (('G')(KEY_G)(1))\
                                (('h')(KEY_H)(0))\
                                (('H')(KEY_H)(1))\
                                (('i')(KEY_I)(0))\
                                (('I')(KEY_I)(1))\
                                (('j')(KEY_J)(0))\
                                (('J')(KEY_J)(1))\
                                (('k')(KEY_K)(0))\
                                (('K')(KEY_K)(1))\
                                (('l')(KEY_L)(0))\
                                (('L')(KEY_L)(1))\
                                (('m')(KEY_M)(0))\
                                (('M')(KEY_M)(1))\
                                (('n')(KEY_N)(0))\
                                (('N')(KEY_N)(1))\
                                (('o')(KEY_O)(0))\
                                (('O')(KEY_O)(1))\
                                (('p')(KEY_P)(0))\
                                (('P')(KEY_P)(1))\
                                (('q')(KEY_Q)(0))\
                                (('Q')(KEY_Q)(1))\
                                (('r')(KEY_R)(0))\
                                (('R')(KEY_R)(1))\
                                (('s')(KEY_S)(0))\
                                (('S')(KEY_S)(1))\
                                (('t')(KEY_T)(0))\
                                (('T')(KEY_T)(1))\
                                (('u')(KEY_U)(0))\
                                (('U')(KEY_U)(1))\
                                (('v')(KEY_V)(0))\
                                (('V')(KEY_V)(1))\
                                (('w')(KEY_W)(0))\
                                (('W')(KEY_W)(1))\
                                (('x')(KEY_X)(0))\
                                (('X')(KEY_X)(1))\
                                (('y')(KEY_Y)(0))\
                                (('Y')(KEY_Y)(1))\
                                (('z')(KEY_Z)(0))\
                                (('Z')(KEY_Z)(1))\

#define VIRTUAL_KBD_aux(r,data,elem) \
                        case BOOST_PP_SEQ_ELEM(0,elem): \
                                if( BOOST_PP_SEQ_ELEM(2,elem) ){ \
                                        cmd_buffer_.shift();\
                                        cmd_buffer_.put( \
                                                EV_KEY, \
                                                BOOST_PP_SEQ_ELEM(1,elem),\
                                                1);\
                                        cmd_buffer_.put( \
                                                EV_KEY, \
                                                BOOST_PP_SEQ_ELEM(1,elem),\
                                                0);\
                                        cmd_buffer_.unshift();\
                                } else{\
                                        cmd_buffer_.put( \
                                                EV_KEY, \
                                                BOOST_PP_SEQ_ELEM(1,elem),\
                                                1);\
                                        cmd_buffer_.put( \
                                                EV_KEY, \
                                                BOOST_PP_SEQ_ELEM(1,elem),\
                                                0);\
                                }\
                                break;
                        

                }

                command_buffer cmd_buffer_;
        };

}


using virtual_keyboard_detail::virtual_keyboard;

