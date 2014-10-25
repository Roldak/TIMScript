//
//  TSTDIterable.cpp
//  TIMScript
//
//  Created by Romain Beguet on 19.05.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDIterable.h"
#include "Interpreter.h"
#include "Array.h"
#include <vector>

namespace ts {
	namespace tstd {

		void importIterable(cmplr::Compiler& cmp) {
			using namespace nodes;
			using namespace tools;

			ClassNode* objectClass = cmp.findClass("Object");
			InterfaceNode* iterableInterface = cmp.addInterface("Iterable");
			ClassNode* iteratorClass = cmp.addClass("Iterator");

			cmp.setAbstractInterfaceData(iterableInterface, "iterator", UserDataInfo("(@Iterable)->Iterator"));

			iteratorClass->extends(objectClass);
			iteratorClass->setAbstract(true);

			cmp.setClassData(iteratorClass, "next", UserDataInfo("(Iterator)->Object"));
			cmp.setClassData(iteratorClass, "hasNext", UserDataInfo("(Iterator)->bool"));

			ImplementationLocation iteratorFuncPos = iterableInterface->getDefinitionIndex("iterator");

			size_t nextFuncPos = iteratorClass->getDefinitionIndex("next");
			size_t hasNextFuncPos = iteratorClass->getDefinitionIndex("hasNext");

            SymbolLocation stringClass = cmp.getSymbolLocation("String");
            SymbolLocation string_new = cmp.getSymbolLocation("String._new_");
            size_t toStringPos = objectClass->getDefinitionIndex("toString");

            cmp.setInterfaceData(iterableInterface, "forEach", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {
				TSDATA it = ctx->callImplementation(iteratorFuncPos, {args[0]});
				objects::Function* next = (objects::Function*)it.Instance->getVirtual(nextFuncPos).Ref;
				objects::Function* hasNext = (objects::Function*)it.Instance->getVirtual(hasNextFuncPos).Ref;
				objects::Function* function = (objects::Function*)args[1].Ref;

				FunctionCaller nextCaller(ctx, next);
				FunctionCaller hasNextCaller(ctx, hasNext);
				FunctionCaller functionCaller(ctx, function);

				std::vector<TSDATA> next_hasNext_Args = {it};
				std::vector<TSDATA> functionArgs(1);

				while (hasNextCaller.call(next_hasNext_Args).Int) {
					functionArgs[0] = nextCaller.call(next_hasNext_Args);
					functionCaller.call(functionArgs);
					ctx->gc->store(functionArgs[0].Instance);
				}

				ctx->gc->store(it.Instance);

				return TSDATA();

			}, "(@Iterable, (Object)->void)->void"));

            cmp.setInterfaceData(iterableInterface, "fold", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {
				TSDATA it = ctx->callImplementation(iteratorFuncPos, {args[0]});
				objects::Function* next = (objects::Function*)it.Instance->getVirtual(nextFuncPos).Ref;
				objects::Function* hasNext = (objects::Function*)it.Instance->getVirtual(hasNextFuncPos).Ref;
				objects::Function* function = (objects::Function*)args[2].Ref;

				FunctionCaller nextCaller(ctx, next);
				FunctionCaller hasNextCaller(ctx, hasNext);
				FunctionCaller functionCaller(ctx, function);

				std::vector<TSDATA> next_hasNext_Args = {it};
				std::vector<TSDATA> functionArgs(2);
				functionArgs[0] = args[1];

				while (hasNextCaller.call(next_hasNext_Args).Int) {
					functionArgs[1] = nextCaller.call(next_hasNext_Args);
					TSDATA tmp = functionCaller.call(functionArgs);
					ctx->gc->store(functionArgs[0].Instance);
					ctx->gc->store(functionArgs[1].Instance);
					functionArgs[0] = tmp;
				}

				ctx->gc->store(it.Instance);
				return functionArgs[0];

			}, "(@Iterable, Object, (Object, Object)->Object)->Object"));

            cmp.setInterfaceData(iterableInterface, "exists", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {
				TSDATA it = ctx->callImplementation(iteratorFuncPos, {args[0]});
				objects::Function* next = (objects::Function*)it.Instance->getVirtual(nextFuncPos).Ref;
				objects::Function* hasNext = (objects::Function*)it.Instance->getVirtual(hasNextFuncPos).Ref;
				objects::Function* function = (objects::Function*)args[1].Ref;

				FunctionCaller nextCaller(ctx, next);
				FunctionCaller hasNextCaller(ctx, hasNext);
				FunctionCaller functionCaller(ctx, function);

				std::vector<TSDATA> next_hasNext_Args = {it};
				std::vector<TSDATA> functionArgs(1);

				TSDATA ret;

				while (hasNextCaller.call(next_hasNext_Args).Int) {
					functionArgs[0] = nextCaller.call(next_hasNext_Args);
					ret = functionCaller.call(functionArgs);
					ctx->gc->store(functionArgs[0].Instance);
					if (ret.Int)
						break;
				}

				ctx->gc->store(it.Instance);

				return ret;
			}, "(@Iterable, (Object)->bool)->bool"));

            cmp.setInterfaceData(iterableInterface, "count", FunctionBuilder::Make([=](ExecutionContext * ctx, TSDATA * args) {
				TSDATA it = ctx->callImplementation(iteratorFuncPos, {args[0]});
				objects::Function* next = (objects::Function*)it.Instance->getVirtual(nextFuncPos).Ref;
				objects::Function* hasNext = (objects::Function*)it.Instance->getVirtual(hasNextFuncPos).Ref;
				objects::Function* function = (objects::Function*)args[1].Ref;

				FunctionCaller nextCaller(ctx, next);
				FunctionCaller hasNextCaller(ctx, hasNext);
				FunctionCaller functionCaller(ctx, function);

				std::vector<TSDATA> next_hasNext_Args = {it};
				std::vector<TSDATA> functionArgs(1);

				TSDATA count;
				count.Int = 0;

				while (hasNextCaller.call(next_hasNext_Args).Int) {
					functionArgs[0] = nextCaller.call(next_hasNext_Args);
					count.Int += functionCaller.call(functionArgs).Int;
					ctx->gc->store(functionArgs[0].Instance);
				}

				ctx->gc->store(it.Instance);

				return count;
			}, "(@Iterable, (Object)->bool)->int"));

            cmp.setInterfaceData(iterableInterface, "join", FunctionBuilder::Make([=](ExecutionContext* ctx, TSDATA* args){

                TSDATA it = ctx->callImplementation(iteratorFuncPos, {args[0]});

                objects::Function* next = (objects::Function*)it.Instance->getVirtual(nextFuncPos).Ref;
                objects::Function* hasNext = (objects::Function*)it.Instance->getVirtual(hasNextFuncPos).Ref;
                std::string* joiner = (std::string*)args[1].Instance->getAttr(0).Ref;

                FunctionCaller nextCaller(ctx, next);
                FunctionCaller hasNextCaller(ctx, hasNext);

                std::vector<TSDATA> next_hasNext_Args = {it};
                std::vector<TSDATA> toStringArg(1);

                std::string* joined = new std::string();

                bool atLeastOne = false;

                while (hasNextCaller.call(next_hasNext_Args).Int) {
                    atLeastOne = true;

                    toStringArg[0] = nextCaller.call(next_hasNext_Args);

                    TSDATA objStr = ctx->callVirtual(toStringPos, toStringArg);
                    joined->append(*((std::string*)objStr.Instance->getAttr(0).Ref)).append(*joiner);

                    ctx->gc->store(objStr.Instance);
                    ctx->gc->store(toStringArg[0].Instance);
                }

                if(atLeastOne){
                    joined->resize(joined->size() - joiner->size());
                }

                ctx->gc->store(it.Instance);

                std::vector<TSDATA> strConstrArgs(2);
                strConstrArgs[0] = ctx->newInstance(stringClass);
                strConstrArgs[1].Ref = joined;

                return ctx->callFunction(string_new, strConstrArgs);

            }, "(@Iterable, String)->String"));

		}
	}
}
