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
#include <functional>
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

            template<typename T>
            static UserDataInfo Native(const std::function<T>& func){
                return Function(func);
            }

            template<typename T, typename... Args>
            static UserDataInfo Constructor(){
                return Function((std::function<T*(Args...)>)_constructor<T, Args...>);
            }

            template<typename T>
            static UserDataInfo Destructor(){
                return Function((std::function<void(T*)>)_destructor<T>);
            }

        private:

            template<typename Ret, typename ...Args>
            static UserDataInfo Function(std::function<Ret(Args...)> func){

                typedef function_traits<decltype(func)> f_t;

                std::vector<type::Type*> argTypes;
                populate<int, Args...>(argTypes);

                std::function<TSDATA(ExecutionContext*, TSDATA*)> inner = [func](ExecutionContext* ctx, TSDATA* args)->TSDATA{
                    return ArgsFlattener<0, f_t::nargs, decltype(func)>::apply(args, func);
                };

                return FunctionBuilder::Make(inner, new type::FunctionType(argTypes, getTSType<Ret>()));
            }

            template<typename ...Args>
            static UserDataInfo Function(std::function<void(Args...)> func){

                typedef function_traits<decltype(func)> f_t;

                std::vector<type::Type*> argTypes;
                populate<int, Args...>(argTypes);

                std::function<TSDATA(ExecutionContext*, TSDATA*)> inner = [func](ExecutionContext* ctx, TSDATA* args)->TSDATA{
                    ArgsFlattener<0, f_t::nargs, decltype(func)>::applyVoid(args, func);
                    return TSDATA{.Int = 0};
                };

                return FunctionBuilder::Make(inner, new type::FunctionType(argTypes, type::BasicType::Void));
            }

            template<typename T>
            struct function_traits;

            template<typename R, typename ...Args>
            struct function_traits<std::function<R(Args...)>>{
                static const size_t nargs=sizeof...(Args);

                typedef R result_type;

                template<size_t i>
                using ArgAt = typename std::tuple_element<i, std::tuple<Args...>>::type;

            };

            template<typename T>
            static type::Type* getTSType(){
                if(std::is_same<bool, T>::value)
                    return type::BasicType::Bool;
                else if (std::is_integral<T>::value)
                    return type::BasicType::Int;
                else if(std::is_floating_point<T>::value)
                    return type::BasicType::Real;
                else
                    return type::BasicType::Int;
            }

            template<typename USELESS, typename Arg1, typename ...Args>
            static void populate(std::vector<type::Type*>& argTypes){
                argTypes.push_back(getTSType<Arg1>());
                populate<USELESS, Args...>(argTypes);
            }

            template<typename USELESS>
            static void populate(std::vector<type::Type*>&){ }

            template<size_t i, size_t N, typename Func, typename ...Args>
            struct ArgsFlattener
            {
                static TSDATA apply(TSDATA* args, const Func& func, Args... a) {
                    typedef function_traits<Func> f_t;
                    typedef typename f_t::template ArgAt<i> Arg;

                    return ArgsFlattener<i+1, N, Func, Args..., Arg>::apply(args, func, a..., fromTSDATA<Arg>(args[i]));
                }

                static void applyVoid(TSDATA* args, const Func& func, Args... a) {
                    typedef function_traits<Func> f_t;
                    typedef typename f_t::template ArgAt<i> Arg;

                    ArgsFlattener<i+1, N, Func, Args..., Arg>::applyVoid(args, func, a..., fromTSDATA<Arg>(args[i]));
                }
            };

            template<size_t N, typename Func, typename ...Args>
            struct ArgsFlattener<N, N, Func, Args...>
            {
                static TSDATA apply(TSDATA* args, const Func& func, Args... a) {
                    return toTSDATA(func(a...));
                }

                static void applyVoid(TSDATA* args, const Func& func, Args... a) {
                    func(a...);
                }
            };

            template<typename T, typename std::enable_if<std::is_same<bool, T>::value, T>::type* = nullptr>
            static T fromTSDATA(TSDATA x){
                return (T)x.Int;
            }

            template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
            static T fromTSDATA(TSDATA x){
                return (T)x.Int;
            }

            template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
            static T fromTSDATA(TSDATA x){
                return (T)x.Real;
            }

            template<typename T, typename std::enable_if<std::is_pointer<T>::value, T>::type* = nullptr>
            static T fromTSDATA(TSDATA x){
                return (T)x.Ref;
            }

            template<typename T, typename std::enable_if<std::is_class<T>::value, T>::type* = nullptr>
            static T fromTSDATA(TSDATA x){
                return *((T*)x.Ref);
            }

            template<typename T, typename std::enable_if<std::is_same<void, T>::value, T>::type* = nullptr>
            static TSDATA toTSDATA(void){
                return TSDATA{.Int = 0};
            }

            template<typename T, typename std::enable_if<std::is_same<bool, T>::value, T>::type* = nullptr>
            static TSDATA toTSDATA(T x){
                return TSDATA{.Int = x};
            }

            template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
            static TSDATA toTSDATA(T x){
                return TSDATA{.Int = x};
            }

            template<typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
            static TSDATA toTSDATA(T x){
                return TSDATA{.Real = x};
            }

            template<typename T, typename std::enable_if<std::is_pointer<T>::value, T>::type* = nullptr>
            static TSDATA toTSDATA(T x){
                return TSDATA{.Ref = x};
            }

            template<typename T, typename... Args>
            static T* _constructor(Args... args){
                return new T(args...);
            }

            template<typename T>
            static void _destructor(T* obj){
                delete obj;
            }
        };
    }
}

#endif /* defined(__TIMScript__FunctionBuilder__) */
