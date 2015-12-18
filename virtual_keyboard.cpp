#include "virtual_keyboard.h"

#include <string>
#include <iostream>

#include <boost/asio.hpp>

#include <linux/input.h>
#include <linux/uinput.h>

#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/xpressive/regex_constants.hpp>
#include <boost/log/trivial.hpp>

#include "key_conv.h"

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
        

}



virtual_keyboard::virtual_keyboard(const std::string& name)
        : backend_( std::make_shared<virtual_keyboard_detail::virtual_keyboard_backend>( name ) )
        , kconv_(std::make_shared<key_conv>())
{}
void virtual_keyboard::graph_or_space(char c){
        if( ! kconv_->operator()( c, [this](__u16 key, bool is_upper){
                press_(key,is_upper); })
        ){
                BOOST_THROW_EXCEPTION(std::domain_error(
                        "unknown key ("
                        + std::string(1,c) + ")"));
        }
        
}
void virtual_keyboard::tab(){
        press_( KEY_TAB, false );
}
void virtual_keyboard::sync_flush(){
        backend_->sync();
        backend_->flush();
}
void virtual_keyboard::shift_(){
        backend_->put( EV_KEY, KEY_RIGHTSHIFT, 1 );
}
void virtual_keyboard::unshift_(){
        backend_->put( EV_KEY, KEY_RIGHTSHIFT, 0 );
}
void virtual_keyboard::press_(__u16 key,bool is_upper){
        BOOST_LOG_TRIVIAL(error) << "press_("
                + boost::lexical_cast<std::string>(key) + ","
                + boost::lexical_cast<std::string>(is_upper) + ")";
        if( is_upper )
                shift_();
        backend_->put( EV_KEY, key, 1 );
        backend_->put( EV_KEY, key, 0 );
        if( is_upper )
                unshift_();
}
void virtual_keyboard::from_string(const std::string& s){
        auto iter = s.begin(), end = s.end();

        std::vector<std::tuple<__u16,bool> > buffer;

        auto result = kconv_->operator()(iter,end,[&buffer](__u16 key, bool is_upper){
                buffer.emplace_back(key,is_upper);
        });
        if( result != end ){
                BOOST_THROW_EXCEPTION(std::domain_error(
                        "unable to full prase, stuck at \""
                        + std::string(result,end) + "\""));
        }
        using std::get;
        for( auto const& p : buffer ){
                press_(get<0>(p),get<1>(p));
        }
}


