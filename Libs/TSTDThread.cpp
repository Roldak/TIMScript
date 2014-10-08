//
//  TSTDThread.cpp
//  TIMScript
//
//  Created by Romain Beguet on 26.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSTDThread.h"
#include "Utils.h"
#include "Interpreter.h"
#include "Function.h"
#include <condition_variable>

namespace ts {
    namespace tstd{

        void importThread(cmplr::Compiler& cmp){
            using namespace nodes;
            using namespace tools;

            ClassNode* threadclass=cmp.addClass("Thread");
            ClassNode* objectclass=cmp.findClass("Object");

            threadclass->extends(objectclass);

            // FUTURE



            threadclass->setAttributes({new Argument("_thrd", type::BasicType::Int)});

            cmp.setClassData(threadclass, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){

                objects::Function* f=(objects::Function*)args[1].Ref;

                std::thread* thrd=new std::thread([ctx, f](){
                    ExecutionContext tctx(ctx, GC_INITIAL_MAXSIZE, INITIAL_STACK_SIZE, INITIAL_LOCALS_SIZE);
                    tctx.gc->setBackupGC(ctx->gc);

                    FunctionCaller funcCaller(&tctx, f);
                    return funcCaller.call({});
                });

                args[0].Instance->setAttr(0, TSDATA{.Ref=thrd});

                return args[0];
            }, "(Thread, ()->void)->Thread"));

            cmp.setClassData(threadclass, "delete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                delete ((std::thread*)args[0].Instance->getAttr(0).Ref);
                return TSDATA();
            }, "(Thread)->void"));

            cmp.setClassData(threadclass, "join", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((std::thread*)args[0].Instance->getAttr(0).Ref)->join();
                return TSDATA();
            }, "(Thread)->void"));

            cmp.setClassData(threadclass, "detach", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((std::thread*)args[0].Instance->getAttr(0).Ref)->detach();
                return TSDATA();
            }, "(Thread)->void"));

            cmp.setClassData(threadclass, "forcecollection", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ctx->gc->forceCollection();
                return TSDATA();
            }, "()->void"));

            cmp.setClassData(threadclass, "sleep", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                std::this_thread::sleep_for(std::chrono::milliseconds(args[0].Int));
                return TSDATA();
            }, "(int)->void"));

            // FUTURE

            ClassNode* future=cmp.addClass("Future");
            future->extends(objectclass);

            future->setAttributes({
                new Argument("_value", objectclass->typeClass()),
                new Argument("_mutex", type::BasicType::Int),
                new Argument("_signal", type::BasicType::Int),
                new Argument("_complete", type::BasicType::Bool)
            });

            cmp.setClassData(future, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(1, TSDATA{.Ref=new std::mutex()});
                args[0].Instance->setAttr(2, TSDATA{.Ref=new std::condition_variable()});
                args[0].Instance->setAttr(3, TSDATA{.Int=false});
                return args[0];
            }, "(Future)->Future"));

            cmp.setClassData(future, "delete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                delete (std::mutex*)args[0].Instance->getAttr(1).Ref;
                delete (std::condition_variable*)args[0].Instance->getAttr(2).Ref;
                return TSDATA();
            }, "(Future)->void"));

            cmp.setClassData(future, "__set", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){

                args[0].Instance->setAttr(0, args[1]);
                args[0].Instance->setAttr(3, TSDATA{.Int=true});

                ((std::condition_variable*)args[0].Instance->getAttr(2).Ref)->notify_one();

                return TSDATA();
            }, "(Future, Object)->void"));

            cmp.setClassData(future, "get", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){

                std::unique_lock<std::mutex> lock(*((std::mutex*)args[0].Instance->getAttr(1).Ref));
                std::condition_variable* var=((std::condition_variable*)args[0].Instance->getAttr(2).Ref);

                if (!args[0].Instance->getAttr(3).Int) {
                    var->wait(lock);
                }

                return args[0].Instance->getAttr(0);
            }, "(Future)->Object"));

            cmp.setClassData(future, "await", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){

                std::unique_lock<std::mutex> lock(*((std::mutex*)args[0].Instance->getAttr(1).Ref));
                std::condition_variable* var=((std::condition_variable*)args[0].Instance->getAttr(2).Ref);

                if (!args[0].Instance->getAttr(3).Int) {
                    var->wait(lock);
                }

                return TSDATA();
            }, "(Future)->void"));

            cmp.setClassData(future, "getTimed", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){

                std::unique_lock<std::mutex> lock(*((std::mutex*)args[0].Instance->getAttr(1).Ref));
                std::condition_variable* var=((std::condition_variable*)args[0].Instance->getAttr(2).Ref);

                if (!args[0].Instance->getAttr(3).Int) {
                    var->wait_for(lock, std::chrono::milliseconds(args[1].Int));
                }

                return args[0].Instance->getAttr(0);
            }, "(Future, int)->Object"));

            cmp.setClassData(future, "isComplete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){

                std::unique_lock<std::mutex> lock(*((std::mutex*)args[0].Instance->getAttr(1).Ref));
                return args[0].Instance->getAttr(3);

            }, "(Future)->bool"));

            auto futAwait=cmp.getSymbolLocation("Future.await");
            auto objToString=cmp.getSymbolLocation("Object.toString");

            cmp.setClassData(future, "toString", FunctionBuilder::Make([futAwait, objToString](ExecutionContext* ctx, TSDATA* args){
                ctx->callNativeFunction(futAwait, {args[0]});
                TSDATA value=args[0].Instance->getAttr(0);
                return ctx->callFunction((objects::Function*)value.Instance->getVirtual(objToString.index).Ref, {value});
            }, "(Future)->String"));

            // ASYNC

            auto futureLoc=cmp.getSymbolLocation("Future");
            auto newFuture=cmp.getSymbolLocation("Future._new_");
            auto setFuture=cmp.getSymbolLocation("Future.__set");

            cmp.setClassData(threadclass, "_async", FunctionBuilder::Make([futureLoc, newFuture, setFuture](ExecutionContext* ctx, TSDATA* args){

                objects::Function* f=(objects::Function*)args[0].Ref;

                TSDATA future=ctx->newInstance(futureLoc);
                ctx->callNativeFunction(newFuture, {future});

                std::thread* thrd;
                thrd=new std::thread([ctx, f, future, setFuture](){
                    ExecutionContext tctx(ctx, GC_INITIAL_MAXSIZE, INITIAL_STACK_SIZE, INITIAL_LOCALS_SIZE);
                    tctx.gc->setBackupGC(ctx->gc);
                    tctx.callNativeFunction(setFuture, {future, tctx.callFunction(f, {})});
                });

                thrd->detach();
                return future;

            }, "(()->Object)->Future"));

            cmp.setClassData(threadclass, "_async_void",
                             FunctionBuilder::Make([futureLoc, newFuture, setFuture](ExecutionContext* ctx, TSDATA* args){

                objects::Function* f=(objects::Function*)args[0].Ref;

                TSDATA future=ctx->newInstance(futureLoc);
                ctx->callNativeFunction(newFuture, {future});

                std::thread* thrd;
                thrd=new std::thread([ctx, f, future, setFuture](){
                    ExecutionContext tctx(ctx, GC_INITIAL_MAXSIZE, INITIAL_STACK_SIZE, INITIAL_LOCALS_SIZE);
                    tctx.gc->setBackupGC(ctx->gc);
                    tctx.callNativeFunction(setFuture, {future, tctx.callFunction(f, {})});
                });

                thrd->detach();
                return future;
                return TSDATA();

            }, "(()->void)->Future"));

            // MUTEX

            ClassNode* mutexClass=cmp.addClass("Mutex");
            mutexClass->extends(objectclass);
            mutexClass->setAttributes({new Argument("_mutex", type::BasicType::Int)});

            cmp.setClassData(mutexClass, "_new_", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(0, TSDATA{.Ref=new std::mutex()});
                return args[0];
            }, "(Mutex)->Mutex"));

            cmp.setClassData(mutexClass, "delete", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                delete (std::mutex*)args[0].Instance->getAttr(0).Ref;
                return TSDATA();
            }, "(Mutex)->void"));

            cmp.setClassData(mutexClass, "lock", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((std::mutex*)args[0].Instance->getAttr(0).Ref)->lock();
                return TSDATA();
            }, "(Mutex)->void"));

            cmp.setClassData(mutexClass, "unlock", FunctionBuilder::Make([](ExecutionContext* ctx, TSDATA* args){
                ((std::mutex*)args[0].Instance->getAttr(0).Ref)->unlock();
                return TSDATA();
            }, "(Mutex)->void"));

            // LOCK

            ClassNode* lockClass=cmp.addClass("Lock");
            lockClass->extends(objectclass);
            lockClass->setAttributes({new Argument("_mutex", mutexClass->typeClass())});

            auto mutexLockLoc=cmp.getSymbolLocation("Mutex.lock");
            auto mutexUnlockLoc=cmp.getSymbolLocation("Mutex.unlock");

            cmp.setClassData(lockClass, "_new_", FunctionBuilder::Make([mutexLockLoc](ExecutionContext* ctx, TSDATA* args){
                args[0].Instance->setAttr(0, args[1]);
                ctx->callFunction(mutexLockLoc, {args[1]});
                return args[0];
            }, "(Lock, Mutex)->Lock"));

            cmp.setClassData(lockClass, "delete", FunctionBuilder::Make([mutexUnlockLoc](ExecutionContext* ctx, TSDATA* args){
                ctx->callFunction(mutexUnlockLoc, {args[0].Instance->getAttr(0)});
                return TSDATA();
            }, "(Lock)->void"));

        }
    }
}
