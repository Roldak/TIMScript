//
//  TSTDClock.cpp
//  TIMScript
//
//  Created by Romain Beguet on 26.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDClock.h"
#include <ctime>

namespace ts {
    namespace tstd{

        void importClock(cmplr::Compiler& cmp){
            using namespace nodes;
            using namespace tools;

            ClassNode* clockclss=cmp.addClass("Clock");
            clockclss->setAttributes({new Argument("clk", type::BasicType::Int)});

            cmp.setClassData(clockclss, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                clock_t* clk=new clock_t;
                args[0].Instance->setAttr(0, TSDATA{.Ref=clk});
                (*clk)=clock();
                return args[0];
            }, "(Clock)->Clock"));

            cmp.setClassData(clockclss, "delete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                delete ((clock_t*)args[0].Instance->getAttr(0).Ref);
                return TSDATA();
            }, "(Clock)->void"));

            cmp.setClassData(clockclss, "elapsedTime", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return TSDATA{.Real=((clock()-*((clock_t*)args[0].Instance->getAttr(0).Ref)))/(TSREAL)CLOCKS_PER_SEC};
            }, "(Clock)->real"));

            cmp.setClassData(clockclss, "reset", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                *((clock_t*)args[0].Instance->getAttr(0).Ref)=clock();
                return TSDATA();
            }, "(Clock)->void"));
        }
    }
}
