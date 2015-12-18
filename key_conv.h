#pragma once

// this class converts between from string seqiunce, and real keys

namespace key_conv_detail{
        namespace xpr = boost::xpressive;

        struct contex{
                using key_t = __u16;
                using callback_t = std::function<void(key_t,bool)>;

                callback_t callback;
        };

        namespace semantics{
                
                struct dispatch_key_impl{
                        using result_type = void;
                        void operator()(contex& ctx, __u16 key, bool is_upper)const{
                                ctx.callback( key, is_upper);
                        }
                };


                #define SEMANTIC_FUNCTIONS\
                        (dispatch_key)
                #define AUX(r,data,elem)  const boost::xpressive::function<BOOST_PP_CAT(elem,_impl)>::type elem = {{}};
                BOOST_PP_SEQ_FOR_EACH( AUX, ~, SEMANTIC_FUNCTIONS )
                #undef AUX
        }
}

struct key_conv{
        key_conv(){
                using namespace key_conv_detail;
                tab_ = xpr::as_xpr("<TAB>")  [semantics::dispatch_key(xpr::ref(ctx_),KEY_TAB,false)];
                f1_ = xpr::as_xpr("<F1>")    [semantics::dispatch_key(xpr::ref(ctx_),KEY_F1,false)];
                f2_ = xpr::as_xpr("<F2>")    [semantics::dispatch_key(xpr::ref(ctx_),KEY_F2,false)];
                f3_ = xpr::as_xpr("<F3>")    [semantics::dispatch_key(xpr::ref(ctx_),KEY_F3,false)];
                rgx_ = tab_ | f1_ | f2_ | f3_;
        }
        // out(__u16, bool upper)
        // aBc<ESC> -> 
        //      out(KEY_A,false)
        //      out(KEY_B,true)
        //      out(KEY_ESC,false)
        template<class Iter, class Output>
        Iter operator()(Iter iter, Iter last, Output&& out){
                ctx_.callback = out;
                for(;iter!=last;++iter){
                        if( try_single_(*iter,out) )
                                continue;
                        namespace xpr = boost::xpressive;
                        xpr::smatch m;
                        if( xpr::regex_search(iter, last, m, rgx_ ) ){
                                iter += m.length() - 1;
                                continue;
                        }
                        break;
                }
                return iter;
        }
        template<class Output>
        bool operator()(char c, Output&& out){
                return try_single_(c,out);
        }
private:
        template<class Output>
        bool try_single_(char c, Output&& out){

                #define VIRTUAL_KBD_char_mapping\
                        (('@')(KEY_APOSTROPHE)(1))\
                        (('1')(KEY_1)(0))\
                        (('!')(KEY_1)(1))\
                        (('2')(KEY_2)(0))\
                        (('"')(KEY_2)(1))\
                        (('3')(KEY_3)(0))\
                        (('£')(KEY_3)(1))\
                        (('4')(KEY_4)(0))\
                        (('$')(KEY_4)(1))\
                        (('5')(KEY_5)(0))\
                        (('%')(KEY_5)(1))\
                        (('6')(KEY_6)(0))\
                        (('^')(KEY_6)(1))\
                        (('7')(KEY_7)(0))\
                        (('&')(KEY_7)(1))\
                        (('8')(KEY_8)(0))\
                        (('*')(KEY_8)(1))\
                        (('9')(KEY_9)(0))\
                        (('(')(KEY_9)(1))\
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
                        ((' ')(KEY_SPACE)(0))\
                        (('0')(KEY_0)(0))\
                        ((')')(KEY_0)(1))\

                        #define VIRTUAL_KBD_aux(r,data,elem) \
                                m_.insert( std::make_pair(\
                                        BOOST_PP_SEQ_ELEM(0,elem) , \
                                        std::make_tuple( \
                                                BOOST_PP_SEQ_ELEM(1,elem) , \
                                                BOOST_PP_SEQ_ELEM(2,elem) ) ) ); 
                        BOOST_PP_SEQ_FOR_EACH( VIRTUAL_KBD_aux,~,VIRTUAL_KBD_char_mapping )
                        #undef VIRTUAL_KBD_aux

                auto iter = m_.find( c );
                if( iter == m_.end() )
                        return false;
                using std::get;
                out( get<0>(iter->second), get<1>(iter->second) );
                return true;
                #undef VIRTUAL_KBD_char_mapping
        }

        boost::xpressive::sregex rgx_, tab_, f1_, f2_, f3_;
        key_conv_detail::contex ctx_;
        std::map<char,std::tuple<__u16,bool> > m_;
};
