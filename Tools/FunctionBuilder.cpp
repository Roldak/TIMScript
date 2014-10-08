//
//  FunctionBuilder.cpp
//  TIMScript
//
//  Created by Romain Beguet on 19.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "FunctionBuilder.h"

namespace ts{
    namespace tools{
        
        UserDataInfo FunctionBuilder::Make(const std::function<TSDATA(ExecutionContext*, TSDATA*)>& f, type::FunctionType* t){
            return UserDataInfo(new objects::NativeFunction(f), t);
        }
        
        UserDataInfo FunctionBuilder::Make(const std::function<TSDATA(ExecutionContext*, TSDATA*)>& f, const std::string& typestr){
            return UserDataInfo(new objects::NativeFunction(f), typestr);
        }
        
        UserDataInfo FunctionBuilder::RealToReal(TSREAL (*func)(TSREAL)){
            
            std::function<TSDATA(ExecutionContext*, TSDATA*)> f=[func](ExecutionContext* ctx, TSDATA* argv)->TSDATA {
                argv[0].Real=func(argv[0].Real);
                return argv[0];
            };
            
            return UserDataInfo(new objects::NativeFunction(f), type::FunctionType::RealToReal);
            
        }
        
        UserDataInfo FunctionBuilder::IntToInt(TSINT (*func)(TSINT)){
            
            std::function<TSDATA(ExecutionContext*, TSDATA*)> f=[func](ExecutionContext* ctx, TSDATA* argv)->TSDATA {
                argv[0].Int=func(argv[0].Int);
                return argv[0];
            };
            
            return UserDataInfo(new objects::NativeFunction(f), type::FunctionType::IntToInt);
            
        }
        
    }
}