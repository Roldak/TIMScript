//
//  TSTDVector.cpp
//  TIMScript
//
//  Created by Romain Beguet on 02.04.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDVector.h"
#include "Interpreter.h"
#include "Array.h"
#include <vector>

namespace ts {
    namespace tstd{
        
        void importVector(cmplr::Compiler& cmp){
            using namespace nodes;
            using namespace tools;

            ClassNode* vectorClass=cmp.addClass("Vector");
            InterfaceNode* iterableInterface=cmp.findInterface("Iterable");
            ClassNode* objectClass=cmp.findClass("Object");
            
            vectorClass->extends(objectClass);
            vectorClass->addInterface(iterableInterface);
            vectorClass->setAttributes({
                new Argument("_vector", new type::ArrayType(objectClass->typeClass(), -1))
            });
            
            cmp.setClassData(vectorClass, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(0, args[1]);
                return args[0];
            }, "(Vector, Object[])->Vector"));

            auto stringClass=cmp.getSymbolLocation("String");
            size_t toStringOffset=objectClass->getDefinitionIndex("toString");

            cmp.setClassData(vectorClass, "toString", FunctionBuilder::Make([stringClass, toStringOffset](ExecutionContext* ctx, TSDATA* args){
                std::string* c=new std::string("{");
                std::vector<TSDATA>& vec=*((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector();

                std::vector<TSDATA> toStringArgs(1);
                for(TSDATA s : vec){
                    toStringArgs[0]=s;
                    TSDATA tsString=ctx->callVirtual(toStringOffset, toStringArgs);
                    c->append(*((std::string*)tsString.Instance->getAttr(0).Ref));
                    c->append(", ");
                    ctx->gc->store(tsString.Instance);
                }
                if(c->size()>2)
                    c->resize(c->size()-2);

                c->append("}");

                TSDATA theString=ctx->newInstance(stringClass);
                theString.Instance->setAttr(0, TSDATA{.Ref=c});

                return theString;
            }, "(Vector)->String"));

            cmp.setClassData(vectorClass, "length", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return TSDATA{.Int=((objects::Array*)args[0].Instance->getAttr(0).Ref)->length()};
            }, "(Vector)->int"));

            cmp.setClassData(vectorClass, "push", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector()->push_back(args[1]);
                return args[0];
            }, "(Vector, Object)->Vector"));


            cmp.setClassData(vectorClass, "pop", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                objects::Array* ary=((objects::Array*)args[0].Instance->getAttr(0).Ref);
                TSDATA last=ary->vector()->at(ary->length()-1);
                ((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector()->pop_back();
                return last;
            }, "(Vector)->Object"));

            cmp.setClassData(vectorClass, "removeAt", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* vec=((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector();
                vec->erase(vec->begin()+args[1].Int);
                return TSDATA();
            }, "(Vector, int)->void"));

            cmp.setClassData(vectorClass, "remove", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* vec=((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector();
                size_t i=0;
                for(; i<vec->size(); ++i){
                    if(vec->operator [](i).Ref==args[1].Ref)
                        break;
                }
                if(i<vec->size()){
                    vec->erase(vec->begin()+i);
                }
                return TSDATA();
            }, "(Vector, Object)->void"));

            cmp.setClassData(vectorClass, "insert", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>* vec=((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector();
                vec->insert(vec->begin()+args[1].Int, args[2]);
                return TSDATA();
            }, "(Vector, int, Object)->void"));

            cmp.setClassData(vectorClass, "_[_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return ((objects::Array*)args[0].Instance->getAttr(0).Ref)->vector()->at(args[1].Int);
            }, "(Vector, int)->Object"));

            // Vector iterator

            ClassNode* vecIterClass=cmp.addClass("VectorIterator");
            ClassNode* iteratorClass=cmp.findClass("Iterator");
            vecIterClass->extends(iteratorClass);
            vecIterClass->setAttributes({
                                            new Argument("_vec", vectorClass->typeClass()),
                                            new Argument("_i", type::BasicType::Int)
                                        });

            cmp.setClassData(vecIterClass, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(0, args[1]);
                args[0].Instance->setAttr(1, TSDATA{.Int=0});
                return args[0];
            }, "(VectorIterator, Vector)->VectorIterator"));

            cmp.setClassData(vecIterClass, "next", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::vector<TSDATA>& vec(*((objects::Array*)args[0].Instance->getAttr(0).Instance->getAttr(0).Ref)->vector());
                TSDATA i=args[0].Instance->getAttr(1);
                TSDATA toRet=vec[i.Int++];
                args[0].Instance->setAttr(1, i);
                return toRet;
            }, "(VectorIterator)->Object"));

            cmp.setClassData(vecIterClass, "hasNext", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return TSDATA{.Int=args[0].Instance->getAttr(1).Int < ((objects::Array*)args[0].Instance->getAttr(0).Instance->getAttr(0).Ref)->length()};
            }, "(VectorIterator)->bool"));

            auto VectorIteratorLoc=cmp.getSymbolLocation("VectorIterator");
            auto VectorIterator_new_Loc=cmp.getSymbolLocation("VectorIterator._new_");

            cmp.setClassData(vectorClass, "iterator", FunctionBuilder::Make([VectorIteratorLoc, VectorIterator_new_Loc](ExecutionContext* ctx, TSDATA* args){
                TSDATA myIter=ctx->newInstance(VectorIteratorLoc);
                return ctx->callFunction(VectorIterator_new_Loc, {myIter, args[0]});
            }, "(Vector)->Iterator"));

            // Iterator-based functions


            auto vectorClassLoc=cmp.getSymbolLocation("Vector");
            auto vectorNewLoc=cmp.getSymbolLocation("Vector._new_");

            ImplementationLocation iteratorFuncPos=iterableInterface->getDefinitionIndex("iterator");
            size_t nextFuncPos=iteratorClass->getDefinitionIndex("next");
            size_t hasNextFuncPos=iteratorClass->getDefinitionIndex("hasNext");

            cmp.setClassData(vectorClass, "findAll", FunctionBuilder::Make([vectorClassLoc, vectorNewLoc, iteratorFuncPos, nextFuncPos, hasNextFuncPos](ExecutionContext* ctx, TSDATA* args){

                objects::Array* ary=new objects::Array(0, 1);
                ctx->gc->trace(ary);

                std::vector<TSDATA>* vec=ary->vector();

                TSDATA it=ctx->callImplementation(iteratorFuncPos, {args[0]});
                objects::Function* next=(objects::Function*)it.Instance->getVirtual(nextFuncPos).Ref;
                objects::Function* hasNext=(objects::Function*)it.Instance->getVirtual(hasNextFuncPos).Ref;
                objects::Function* function=(objects::Function*)args[1].Ref;

                FunctionCaller nextCaller(ctx, next);
                FunctionCaller hasNextCaller(ctx, hasNext);
                FunctionCaller functionCaller(ctx, function);

                std::vector<TSDATA> next_hasNext_Args={it};
                std::vector<TSDATA> functionArgs(1);

                while(hasNextCaller.call(next_hasNext_Args).Int){
                    functionArgs[0]=nextCaller.call(next_hasNext_Args);
                    if(functionCaller.call(functionArgs).Int)
                        vec->push_back(functionArgs[0]);
                    ctx->gc->store(functionArgs[0].Instance);
                }

                ctx->gc->store(it.Instance);

                return ctx->callFunction(vectorNewLoc, {ctx->newInstance(vectorClassLoc), TSDATA{.Ref=ary}});
            }, "(@Iterable, (Object)->bool)->Vector"));

        }
    }
}
