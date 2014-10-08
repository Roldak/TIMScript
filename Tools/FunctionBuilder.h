//
//  FunctionBuilder.h
//  TIMScript
//
//  Created by Romain Beguet on 19.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__FunctionBuilder__
#define __TIMScript__FunctionBuilder__

#include <iostream>
#include "TSType.h"
#include "UserData.h"
#include "NativeData.h"

namespace ts{
    namespace tools{
        
        class FunctionBuilder{
        public:

            static UserDataInfo Make(const std::function<TSDATA(ExecutionContext*, TSDATA*)>& f, type::FunctionType* t);
            static UserDataInfo Make(const std::function<TSDATA(ExecutionContext*, TSDATA*)>& f, const std::string& typestr);
            
            static UserDataInfo RealToReal(TSREAL (*f)(TSREAL));
            static UserDataInfo IntToInt(TSINT (*f)(TSINT));
            
            static UserDataInfo Get(TSINT (*f)(TSINT)){return IntToInt(f);}
            static UserDataInfo Get(TSREAL (*f)(TSREAL)){return RealToReal(f);}
        };
        
        template<typename Ret, typename ...Args>
        std::function<Ret(Args...)> toStdFunction(Ret (*func)(Args...)){
            return std::function<Ret(Args...)>(func);
        }
        
        template<typename T>
        struct function_traits;
        
        template<typename R, typename ...Args>
        struct function_traits<std::function<R(Args...)>>{
            static const size_t nargs=sizeof...(Args);

            typedef R result_type;
        
            template<size_t i>
            struct arg{
                typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
            };
        };
        
        template<typename T>
        type::Type* getTSType(){
            if(std::is_same<bool, T>::value)
                return type::BasicType::Bool;
            else if (std::is_same<int, T>::value || std::is_same<long, T>::value)
                return type::BasicType::Int;
            else if(std::is_same<float, T>::value || std::is_same<double, T>::value)
                return type::BasicType::Real;
            else
                return NULL;
        }
        /*
        
        template<typename Ret, typename ...Args>
        UserDataInfo Function(std::function<Ret(Args...)> func){
            
            typedef function_traits<decltype(func)> f_t;
            
            //type::Type* retType=getTSType<f_t::result_type>();
            std::vector<type::Type*> argTypes(f_t::nargs);
        
            std::cerr<<std::is_same<function_traits<std::function<Ret(Args...)>>::arg<0>::type, int>::value;
            
            for (size_t i=0; i<f_t::nargs; ++i) {
//                argTypes[i]=getTSType< f_t::arg<0>::type >();
            }
            
            std::function<TSDATA(ExecutionContext*, TSDATA*)> inner=[](ExecutionContext* ctx, TSDATA*)->TSDATA{
                
            };
            
            //return UserDataInfo(, );
        }*/
        
    }
}

#endif /* defined(__TIMScript__FunctionBuilder__) */
