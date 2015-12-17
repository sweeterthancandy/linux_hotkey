#pragma once

#include <map>
#include <linux/input.h>
#include <boost/lexical_cast.hpp>

struct keyboard_state{

        using key_t = __u16;

        void down(key_t k){
                m_[k] = true;
        }
        void up(key_t k){
                auto iter = m_.find(k);
                if( iter != m_.end() )
                        m_.erase( iter );
        }
        bool is_down(key_t k)const{
                return m_.find(k) != m_.end();
        }
        size_t keys_down()const{
                return m_.size();
        }
        bool operator==(const keyboard_state& that)const{
                // check that both have the exact keys in map (ignore values)
                if( m_.size() != that.m_.size() )
                        return false;
                auto const end = that.m_.end();
                for( auto const& p : m_ )
                        if( that.m_.find(p.first) == end )
                                return false;
                return true;
        }
        friend std::ostream& operator<<(std::ostream& ostr, keyboard_state const& self){
                if( self.m_.size() == 0 )
                        return ostr << "{}";

                auto format = [](key_t k)->std::string{
                        return boost::lexical_cast<std::string>(k);
                };

                auto iter = self.m_.begin();
                auto end = self.m_.end();
                ostr << "{" << format(iter->first);
                ++iter;
                for(;iter!=end;++iter){
                        ostr << "," << format(iter->first);
                }
                return ostr << "}";
        }
private:
        // note that the value is irrelevant, just
        // making use of 
        std::map<key_t,bool> m_;
};
