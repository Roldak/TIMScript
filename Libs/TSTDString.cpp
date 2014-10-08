//
//  TSTDString.cpp
//  TIMScript
//
//  Created by Romain Beguet on 24.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDString.h"
#include "Interpreter.h"

namespace ts {
    namespace tstd{

        void importString(cmplr::Compiler& cmp){
            using namespace nodes;
            using namespace tools;

            ClassNode* string=cmp.addClass("String");
            ClassNode* objectClass=cmp.findClass("Object");
            string->extends(objectClass);
            string->setAttributes({new Argument("_str", type::BasicType::Int)});

            auto stringIndex=cmp.getSymbolLocation("String");

            cmp.setClassData(string, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(0, args[1]);
                return args[0];
            }, "(String, Any)->String"));

            cmp.setClassData(string, "push", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((std::string*)args[0].Instance->getAttr(0).Ref)->push_back((char)args[1].Int);
                return args[0];
            }, "(String, int)->String"));

            cmp.setClassData(string, "at", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return TSDATA{.Int=((std::string*)args[0].Instance->getAttr(0).Ref)->operator[]((size_t)args[1].Int)};
            }, "(String, int)->int"));


            cmp.setClassData(string, "setAt", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((std::string*)args[0].Instance->getAttr(0).Ref)->operator[]((size_t)args[1].Int)=args[2].Int;
                return TSDATA();
            }, "(String, int, int)->void"));

            size_t toStringOffset=objectClass->getDefinitionIndex("toString");

            cmp.setClassData(string, "_+_", FunctionBuilder::Make([stringIndex, toStringOffset](ExecutionContext* ctx, TSDATA *args){
                TSDATA newstr=ctx->newInstance(stringIndex);

                std::string a(*((std::string*)args[0].Instance->getAttr(0).Ref));
                TSDATA strB=ctx->callVirtual(toStringOffset, {args[1]});
                std::string b(*((std::string*)strB.Instance->getAttr(0).Ref));

                ctx->gc->store(strB.Instance);
                newstr.Instance->setAttr(0, TSDATA{.Ref=new std::string(a+b)});

                return newstr;

            }, "(String, Object)->String"));

            cmp.setClassData(string, "_<_", FunctionBuilder::Make([stringIndex](ExecutionContext* ctx, TSDATA *args){
                std::string* a((std::string*)args[0].Instance->getAttr(0).Ref);
                std::string* b((std::string*)args[1].Instance->getAttr(0).Ref);

                return TSDATA{.Int=(*a<*b)};
            }, "(String, String)->bool"));

            cmp.setClassData(string, "_==_", FunctionBuilder::Make([stringIndex](ExecutionContext* ctx, TSDATA *args){
                if (args[0].Instance->getClass()==args[1].Instance->getClass()){
                    std::string* a((std::string*)args[0].Instance->getAttr(0).Ref);
                    std::string* b((std::string*)args[1].Instance->getAttr(0).Ref);

                    return TSDATA{.Int=(*a==*b)};
                }
                return TSDATA{.Int=false};
            }, "(String, Object)->bool"));

            cmp.setClassData(string, "length", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return TSDATA{.Int=(TSINT)((std::string*)args[0].Instance->getAttr(0).Ref)->size()};
            }, "(String)->int"));

            cmp.setClassData(string, "toString", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                return args[0];
            }, "(String)->String"));

            cmp.setClassData(string, "delete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                delete (std::string*)args[0].Instance->getAttr(0).Ref;
                return TSDATA();
            }, "(String)->void"));

            ClassNode* stringBuilder=cmp.addClass("StringBuilder");
            stringBuilder->extends(objectClass);

            stringBuilder->setAttributes({new Argument("_str", type::BasicType::Int)});

            cmp.setClassData(stringBuilder, "_new_", tools::FunctionBuilder::Make([stringIndex](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(0, TSDATA{.Ref=new std::string(*((std::string*)args[1].Instance->getAttr(0).Ref))});
                return args[0];
            }, "(StringBuilder, String)->StringBuilder"));

            cmp.setClassData(stringBuilder, "delete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                delete (std::string*)args[0].Instance->getAttr(0).Ref;
                return TSDATA();
            }, "(StringBuilder)->void"));

            cmp.setClassData(stringBuilder, "append", tools::FunctionBuilder::Make([toStringOffset](ExecutionContext* ctx, TSDATA* args){

                TSDATA str=ctx->callVirtual(toStringOffset, {args[1]});
                ((std::string*)args[0].Instance->getAttr(0).Ref)->append(*((std::string*)str.Instance->getAttr(0).Ref));
                ctx->gc->store(str.Instance);

                return args[0];
            }, "(StringBuilder, Object)->StringBuilder"));

            cmp.setClassData(stringBuilder, "toString", tools::FunctionBuilder::Make([stringIndex](ExecutionContext* ctx, TSDATA* args){
                TSDATA str=ctx->newInstance(stringIndex);
                str.Instance->setAttr(0, TSDATA{.Ref=new std::string(*((std::string*)args[0].Instance->getAttr(0).Ref))});
                return str;
            }, "(StringBuilder)->String"));
        }
    }
}
