//
//  TSTDArray.cpp
//  TIMScript
//
//  Created by Romain Beguet on 28.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDArray.h"
#include <vector>
#include <algorithm>
#include <string.h>
#include "Array.h"
#include "Interpreter.h"

namespace ts {
    namespace tstd{

        void importArray(cmplr::Compiler& cmp){
            using namespace nodes;
            using namespace tools;

            ClassNode* array=cmp.addClass("Array");

            cmp.setClassData(array, "length", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return TSDATA{.Int=((objects::Array*)args[0].Ref)->length()};
            }, "(Any)->int"));

            cmp.setClassData(array, "copy", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                TSDATA* ary1=((objects::Array*)args[0].Ref)->array();
                TSDATA* ary2=((objects::Array*)args[1].Ref)->array();
                TSINT len=((objects::Array*)args[0].Ref)->length();
                memcpy(ary1, ary2, (size_t)len*sizeof(TSDATA));
                return TSDATA();
            }, "(Any, Any)->void"));

            cmp.setClassData(array, "push", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((objects::Array*)args[0].Ref)->vector()->push_back(args[1]);
                return args[0];
            }, "(Any, Any)->Any"));

            cmp.setClassData(array, "pop", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* vec=((objects::Array*)args[0].Ref)->vector();
                TSDATA toRet=vec->operator[](vec->size()-1);
                vec->pop_back();
                return toRet;
            }, "(Any)->Any"));

            cmp.setClassData(array, "swap", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                TSDATA* ary=((objects::Array*)args[0].Ref)->array();
                TSDATA tmp=ary[args[1].Int];
                ary[args[1].Int]=ary[args[2].Int];
                ary[args[2].Int]=tmp;
                return TSDATA();
            }, "(Any, int, int)->void"));

            cmp.setClassData(array, "sort", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* data(((objects::Array*)args[0].Ref)->vector());
                std::sort(data->begin(), data->end(), [](TSDATA a, TSDATA b){return a.Int<b.Int;});
                return TSDATA();
            }, "(int[])->void"));

            cmp.setClassData(array, "customSort", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* data(((objects::Array*)args[0].Ref)->vector());

                FunctionCaller funcCaller(ctx, (objects::Function*)args[1].Ref);
                std::vector<TSDATA> funcArgs(2);

                std::sort(data->begin(), data->end(), [&funcCaller, &funcArgs](TSDATA a, TSDATA b){
                    funcArgs[0]=a;
                    funcArgs[1]=b;
                    return funcCaller.call(funcArgs).Int;
                });
                return TSDATA();
            }, "(Any, Any)->void"));

            cmp.setClassData(array, "foreach", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                TSDATA* ary=((objects::Array*)args[0].Ref)->array();
                TSINT length=((objects::Array*)args[0].Ref)->length();
                objects::Function* func=(objects::Function*)args[1].Ref;

                FunctionCaller funcCaller(ctx, func);

                std::vector<TSDATA> funcArgs(1);

                for (TSINT i=0; i<length; ++i){
                    funcArgs[0]=ary[i];
                    funcCaller.call(funcArgs);
                }

                return TSDATA();
            }, "(Any, Any)->void"));

            cmp.setClassData(array, "fill", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                TSDATA* ary=((objects::Array*)args[0].Ref)->array();
                TSINT length=((objects::Array*)args[0].Ref)->length();

                objects::Function* func=(objects::Function*)args[1].Ref;

                FunctionCaller funcCaller(ctx, func);

                std::vector<TSDATA> funcargs(1);

                for (TSINT i=0; i<length; ++i){
                    funcargs[0].Int=(TSINT)i;
                    ary[i]=funcCaller.call(funcargs);
                }

                return args[0];
            }, "(Any, Any)->Any"));

            cmp.setClassData(array, "printInt", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* data(((objects::Array*)args[0].Ref)->vector());
                std::cout<<"[";

                for (size_t i=0, e=data->size(); i<e; ++i) {
                    std::cerr<<data->operator[](i).Int;
                    if (i!=e-1) {
                        std::cout<<", ";
                    }
                }
                std::cout<<"]";
                std::cout.flush();
                return TSDATA();
            }, "(int[])->void"));
        }
    }
}
