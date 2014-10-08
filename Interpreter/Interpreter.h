//
//  InterpreterV2.h
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__InterpreterV2__
#define __TIMScript__InterpreterV2__

#include <iostream>
#include "Utils.h"
#include "TSProgram.h"
#include "GC.h"
#include "Debugger.h"

namespace ts{
    
    namespace objects{
        class Function;
    }
    
    struct ExecutionContext{
        
        ExecutionContext(size_t gc_initial_size, size_t max_stack_size, size_t max_locals_size,
                         const TSDATA* const_pool, const TSDATA* natives_pool);
        
        ExecutionContext(ExecutionContext* ctx, size_t gc_initial_size, size_t max_stack_size, size_t max_locals_size);
        
        ~ExecutionContext();
        
        TSDATA newInstance(const SymbolLocation& classLocation);
        TSDATA callFunction(const SymbolLocation& funcLocation, const std::vector<TSDATA>& args);
        TSDATA callFunction(objects::Function* func, const std::vector<TSDATA>& args);
        TSDATA callNativeFunction(const SymbolLocation& funcLocation, std::vector<TSDATA> args);
        TSDATA callVirtual(size_t index, const std::vector<TSDATA>& args);
        TSDATA callImplementation(const ImplementationLocation& loc, const std::vector<TSDATA>& args);
        
        // pools
        const TSDATA* cPool; // constants pool
        const TSDATA* nPool; // natives pool
        
        // stack
        TSDATA* stack;
        char* data_stack;
        
        // locals
        TSDATA* locals;
        
        // roots array
        
        data_Array roots_ary;
        
        // call stack pointer
        
        size_t csp;
        
        // garbage collector
        GarbageCollector* gc;
        
        // current procedure
        const Procedure* caller;
    };
    
    class FunctionCaller{
    public:
        FunctionCaller(ExecutionContext* ctx, const SymbolLocation& funcLocation);
        FunctionCaller(ExecutionContext* ctx, const objects::Function* func);
        ~FunctionCaller();
        
        TSDATA call(const std::vector<TSDATA>& args);
        TSDATA callUser(const std::vector<TSDATA>& args);
        TSDATA callNative(const std::vector<TSDATA>& args);
        
    private:
        
        ExecutionContext* _ctx;
        const objects::Function* _func;
        size_t _nbCap;
        
        size_t _o_csp;
        const Procedure* _o_proc;
        const Procedure* _n_proc;
    };
    
    class Interpreter{
    public:
        
        Interpreter(Program* prg);
        
        ExecutionContext* createExecutionContext();
        
        template<bool USE_GC>
        static TSDATA exec(ExecutionContext* ctx, const Procedure* proc);
        
        static AbstractDebugger* Debugger;

    private:
        
        void computeConstantPool();
        
        Program* _prg;
        TSDATA* _constantPool;
        
    };

    inline TSDATA FunctionCaller::callUser(const std::vector<TSDATA>& args){
        for (size_t i=0; i<args.size(); ++i)
            _ctx->locals[_nbCap+i]=args[i];
        
        return Interpreter::exec<true>(_ctx, _n_proc);
    }
}

#endif /* defined(__TIMScript__InterpreterV2__) */
