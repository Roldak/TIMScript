//
//  InterpreterV2.cpp
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Interpreter.h"
#include "InterpreterDefines.h"
#include "Instance.h"
#include "Array.h"
#include "Function.h"
#include "NativeData.h"
#include "Tuple.h"
#include "DebugInfos.h"
#include <cmath>

namespace ts{

    // EXECUTION CONTEXT

    ExecutionContext::ExecutionContext(size_t gc_initial_size, size_t max_stack_size, size_t max_locals_size,
                                       const TSDATA* const_pool, const TSDATA* natives_pool){

        cPool=const_pool;
        nPool=natives_pool;

        stack=new TSDATA[max_stack_size]();
        locals=new TSDATA[max_locals_size]();

        TSDATA* raw_roots_ary=new TSDATA[max_locals_size]();
        TSDATA z; z.Int=0;

        for(size_t i=0; i<max_locals_size; ++i)
            raw_roots_ary[i]=z;

        roots_ary=data_Array(0, raw_roots_ary);
        csp=0;

        data_stack=new char[10000000];

        if (gc_initial_size>0)
            gc=new GarbageCollector(gc_initial_size, &roots_ary, this);
        else
            gc=NULL;

        caller=NULL;
    }

    ExecutionContext::ExecutionContext(ExecutionContext* ctx, size_t gc_initial_size, size_t max_stack_size, size_t max_locals_size):
        ExecutionContext(gc_initial_size, max_stack_size, max_locals_size, ctx->cPool, ctx->nPool)
    {}

    ExecutionContext::~ExecutionContext(){
        delete[] stack;
        delete[] locals;
        delete[] data_stack;
        if (gc)
            delete gc;
    }

    TSDATA ExecutionContext::newInstance(const SymbolLocation& classLocation){
        TSDATA inst=((objects::Class*)cPool[classLocation.index].Ref)->newInstance();
        gc->trace(inst.Instance);
        return inst;
    }

    TSDATA ExecutionContext::callFunction(const SymbolLocation& funcLocation, const std::vector<TSDATA>& args){
        return FunctionCaller(this, funcLocation).call(args);
    }

    TSDATA ExecutionContext::callFunction(objects::Function* func, const std::vector<TSDATA>& args){
        return FunctionCaller(this, func).call(args);
    }

    TSDATA ExecutionContext::callNativeFunction(const SymbolLocation& funcLocation, std::vector<TSDATA> args){
        return ((objects::Function*)nPool[funcLocation.index].Ref)->getNative()->call(this, args.data());
    }

    TSDATA ExecutionContext::callVirtual(size_t index, const std::vector<TSDATA>& args){
        return FunctionCaller(this, (objects::Function*)args[0].Instance->getVirtual(index).Ref).call(args);
    }

    TSDATA ExecutionContext::callImplementation(const ImplementationLocation& loc, const std::vector<TSDATA> &args)
    {
        return FunctionCaller(this, (objects::Function*)args[0].Instance->getImplementation(loc.interfaceID, loc.index).Ref).call(args);
    }

    // FUNCTION CALLER

    FunctionCaller::FunctionCaller(ExecutionContext* ctx, const SymbolLocation& funcLocation)
    : FunctionCaller(ctx,
                     ((objects::Function*)(funcLocation.pool==SymbolLocation::Pool::K_POOL
                                           ?    ctx->cPool[funcLocation.index]
                                           :    ctx->nPool[funcLocation.index]).Ref))
    {
    }

    FunctionCaller::FunctionCaller(ExecutionContext* ctx, const objects::Function* func)
    : _ctx(ctx), _func(func), _nbCap(func->captures().size())
    {

        _o_csp=ctx->csp;
        _ctx->csp=0;

        _o_proc=ctx->caller;

        if (func->getNative())
            return;

        _n_proc=func->getProcPtr();

        if (_o_proc){
            ctx->locals+=_o_proc->numberOfLocals();
            ctx->stack+=_o_proc->stackSize();
        }

        for (size_t i=0; i<_nbCap; ++i)
            ctx->locals[i]=func->captures()[i];
    }

    FunctionCaller::~FunctionCaller(){
        _ctx->caller=_o_proc;
        _ctx->csp=_o_csp;

        if (_func->getNative())
            return;

        if (_o_proc){
            _ctx->locals-=_o_proc->numberOfLocals();
            _ctx->stack-=_o_proc->stackSize();
        }
    }

    TSDATA FunctionCaller::call(const std::vector<TSDATA>& args){
        if (_func->getNative())
            return callNative(args);
        else
            return callUser(args);
    }

    TSDATA FunctionCaller::callNative(const std::vector<TSDATA>& args){
        return _func->getNative()->call(_ctx, (TSDATA*)args.data());
    }

    // INTERPRETER

    Interpreter::Interpreter(Program* prg) : _prg(prg){
        computeConstantPool();
    }

    void Interpreter::computeConstantPool(){
        _constantPool=new TSDATA[_prg->constantPool().size()];
        ExecutionContext ctx(0, 200, 10, _constantPool, _prg->nativesPool());

        for (size_t i=0; i<_prg->constantPool().size(); ++i){
            _constantPool[i]=exec<false>(&ctx, _prg->constantPool()[i]);
        }

        _prg->main()->setRefMap((size_t_Array**)(_constantPool+_prg->mainRefMapIndex()));
        _prg->main()->setExTable((ExceptionTable**)(_constantPool+_prg->mainExTableIndex()));
    }

    ExecutionContext* Interpreter::createExecutionContext(){
        return new ExecutionContext(GC_INITIAL_MAXSIZE, INITIAL_STACK_SIZE, INITIAL_LOCALS_SIZE, _constantPool, _prg->nativesPool());
    }

    AbstractDebugger* Interpreter::Debugger=NULL;

    template<bool USE_GC>
    TSDATA Interpreter::exec(ExecutionContext* ctx, const Procedure* proc){

        // get the content of the execution context

        const TSDATA* cPool=ctx->cPool;
        const TSDATA* nPool=ctx->nPool;
        const TSINSTR* p=proc->byteCode();

        // create the arrays that will store states for each function call

        data_Array& roots_ary=ctx->roots_ary;

        if (USE_GC)
            roots_ary._count+=proc->getRefMap()->_count;

        TSDATA* cur_roots=roots_ary._array;

        TSDATA* stack=ctx->stack;
        TSDATA* locals=ctx->locals;
        char* data_stack=ctx->data_stack;

        TSINSTR ip=0; // instruction pointer
        size_t sp=0; // stack pointer
        size_t start_sp=0; // stack pointer at the start of the current function call
        size_t data_sp=0; // data stack pointer

        ctx->caller=proc;

        static const void* jump_table[]{
            // Variables

            &&STORE_N, &&LOAD_N, &&STORE_R, &&GC_STORE, &&LOAD_CST, &&LOAD_NAT, &&DUP, &&POP, &&POP_R,

            // Booleans operations

            &&PUSH_FALSE, &&PUSH_TRUE, // Constants
            &&AND_B, &&OR_B,// Binary operations
            // Unary operations
            // Branches
            &&PRINT_B,  // Others

            // Integers operations

            &&PUSH_I, // Constants
            &&ADD_I, &&SUB_I, &&MUL_I, &&DIV_I, &&MOD_I, &&POW_I, // Binary operations
            &&CMP_I_L, &&CMP_I_G, &&CMP_I_EQ, &&CMP_I_NEQ, // Comparisons
            &&INC_I, &&INC_I_1, &&DEC_I, &&DEC_I_1, &&NEG_I, // Unary operations
            &&IF_I_TRUE, &&IF_I_FALSE, &&IFG_I, // Branches
            &&I_TO_R, // Casts
            &&PRINT_I, // Others

            // Reals operations

            &&PUSH_R, // Constants
            &&ADD_R, &&SUB_R, &&MUL_R, &&DIV_R, &&MOD_R, &&POW_R, // Binary operations
            &&CMP_R_L, &&CMP_R_G, &&CMP_R_EQ, &&CMP_R_NEQ, // Comparisons
            &&INC_R, &&INC_R_1, &&DEC_R, &&DEC_R_1, &&NEG_R, // Unary operations
            &&IFG_R, // Branches
            &&R_TO_I, // Casts
            &&PRINT_R, // Others

            // Raw datas

            &&RAW_ARRAY,

            // Functions operations

            &&MK_FUNC, &&CALL_FUNC, &&CALL_FUNC_R, &&CALL_FUNC_V, &&RET_R, &&RET, &&RET_V,

            // Class operations

            &&MK_CLASS, &&NEW, &&NEW_SCOPED,

            // Instance operations

            &&GET_F, &&SET_F, &&SET_F_R, &&GET_V, &&GET_I,
            &&LD_0_GET_F, &&LD_0_SET_F, &&LD_0_SET_F_R,
            &&LD_CALL_V, &&INSTANCEOF,

            // String operations

            &&MK_STRING, &&PRINT_STR,

            // Array operations

            &&NEW_ARRAY, &&MK_ARRAY, &&GET_AT, &&SET_AT, &&SET_AT_R,

            // Tuple operations

            &&NEW_TUPLE, &&MK_TUPLE, &&GET_TE, &&SET_TE, &&SET_TE_R,

            // Scope allocations

            &&INIT_SCOPE, &&CLEAN_SCOPE,

            // Jump/lookup tables

            &&LU_TABLE, &&J_TABLE,

            // Exceptions

            &&MK_EX_TABLE, &&THROW,

            // Other control flow operations

            &&GOTO, &&EXIT,

            // Debug Instructions

            &&MK_DBG_INFO, &&DBG_EOS, &&DBG_CALL, &&DBG_RET, &&DBG_NEW
        };

        while (true) {
            goto NEXT_INSTR;

            ///////////////////////////////////////
            // VARIABLES
            ///////////////////////////////////////

        STORE_N:
            locals[p[++ip]]=stack[--sp];
            goto INC_NEXT_INSTR;

        LOAD_N:
            stack[sp++]=locals[p[++ip]];
            goto INC_NEXT_INSTR;

        STORE_R:
            locals[p[++ip]]=stack[--sp];

            if (USE_GC){
                cur_roots[proc->getRefMap()->_array[p[ip]]]=stack[sp];
                SAFE_STORE_IF_TEMP(stack[sp])
            }

            goto INC_NEXT_INSTR;

        GC_STORE:
            if (USE_GC)
                ctx->gc->store((objects::Object*)stack[sp-1].Ref);
            goto INC_NEXT_INSTR;

        LOAD_CST:
            stack[sp++]=cPool[p[++ip]];
            goto INC_NEXT_INSTR;

        LOAD_NAT:
            stack[sp++]=nPool[p[++ip]];
            goto INC_NEXT_INSTR;

        DUP:
            stack[sp]=stack[sp-p[++ip]];
            ++sp;
            goto INC_NEXT_INSTR;

        POP:
            --sp;
            goto INC_NEXT_INSTR;

        POP_R:
            SAFE_STORE_IF_TEMP(stack[sp-1])
            --sp;
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // BOOLEANS OPERATIONS
            ///////////////////////////////////////

        PUSH_FALSE:
            stack[sp++].Int=0;
            goto INC_NEXT_INSTR;

        PUSH_TRUE:
            stack[sp++].Int=1;
            goto INC_NEXT_INSTR;

            BINARY_OPER_CODE(AND_B, &&, Int)
            BINARY_OPER_CODE(OR_B, ||, Int)

        PRINT_B:
            --sp;
            std::cout<<(stack[sp].Int ? "true" : "false");
            std::cout.flush();
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // INTEGERS OPERATIONS
            ///////////////////////////////////////

            // Constants

        PUSH_I:
            stack[sp++].Int=(TSINT)p[++ip];
            goto INC_NEXT_INSTR;

            // Binary operations

            BINARY_OPER_CODE(ADD_I, +, Int)
            BINARY_OPER_CODE(SUB_I, -, Int)
            BINARY_OPER_CODE(MUL_I, *, Int)
            BINARY_OPER_CODE(DIV_I, /, Int)
            BINARY_OPER_CODE(MOD_I, %, Int)

        POW_I:
            --sp;
            stack[sp-1].Int=pow(stack[sp-1].Int, stack[sp].Int);
            goto INC_NEXT_INSTR;

            // Comparisons

            BINARY_BOOLOPER_CODE(CMP_I_L, <, Int)
            BINARY_BOOLOPER_CODE(CMP_I_G, >, Int)
            BINARY_BOOLOPER_CODE(CMP_I_EQ, ==, Int)
            BINARY_BOOLOPER_CODE(CMP_I_NEQ, !=, Int)

            // Unary operations

        INC_I:
            ip+=2;
            locals[p[ip-1]].Int+=p[ip];
            goto INC_NEXT_INSTR;

        INC_I_1:
            stack[sp++].Int=++locals[p[++ip]].Int;
            goto INC_NEXT_INSTR;

        DEC_I:
            ip+=2;
            locals[p[ip-1]].Int-=p[ip];
            goto INC_NEXT_INSTR;

        DEC_I_1:
            stack[sp++].Int=--locals[p[++ip]].Int;
            goto INC_NEXT_INSTR;

        NEG_I:
            stack[sp-1].Int=-stack[sp-1].Int;
            goto INC_NEXT_INSTR;

            // Branches

        IF_I_TRUE:
            sp--;
            ++ip;
            if (stack[sp].Int)
                ip=p[ip]-1;
            goto INC_NEXT_INSTR;

        IF_I_FALSE:
            sp--;
            ++ip;
            if (!stack[sp].Int)
                ip=p[ip]-1;
            goto INC_NEXT_INSTR;

        IFG_I:
            sp-=2;
            ++ip;
            if (stack[sp].Int>stack[sp+1].Int)
                ip=p[ip]-1;
            goto INC_NEXT_INSTR;

            // Casts

        I_TO_R:
            stack[sp-1].Real=(TSREAL)stack[sp-1].Int;
            goto INC_NEXT_INSTR;

            // Others

        PRINT_I:
            --sp;
            std::cout<<stack[sp].Int;
            std::cout.flush();
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // REALS OPERATION
            ///////////////////////////////////////

            // Constants

        PUSH_R:
            stack[sp++].Real=instr_to_r(p[++ip]);
            goto INC_NEXT_INSTR;

            // Binary operations

            BINARY_OPER_CODE(ADD_R, +, Real)
            BINARY_OPER_CODE(SUB_R, -, Real)
            BINARY_OPER_CODE(MUL_R, *, Real)
            BINARY_OPER_CODE(DIV_R, /, Real)

        MOD_R:
            --sp;
            stack[sp-1].Real=fmod(stack[sp-1].Real, stack[sp].Real);
            goto INC_NEXT_INSTR;

        POW_R:
            --sp;
            stack[sp-1].Real=pow(stack[sp-1].Real, stack[sp].Real);
            goto INC_NEXT_INSTR;

            // Comparisons

            BINARY_BOOLOPER_CODE(CMP_R_L, <, Real)
            BINARY_BOOLOPER_CODE(CMP_R_G, >, Real)
            BINARY_BOOLOPER_CODE(CMP_R_EQ, ==, Real)
            BINARY_BOOLOPER_CODE(CMP_R_NEQ, !=, Real)

            // Unary operations

        INC_R:
            ip+=2;
            locals[p[ip-1]].Real+=p[ip];
            goto INC_NEXT_INSTR;

        INC_R_1:
            stack[sp++].Real=++locals[p[++ip]].Real;
            goto INC_NEXT_INSTR;

        DEC_R:
            ip+=2;
            locals[p[ip-1]].Real-=p[ip];
            goto INC_NEXT_INSTR;

        DEC_R_1:
            stack[sp++].Real=--locals[p[++ip]].Real;
            goto INC_NEXT_INSTR;

        NEG_R:
            stack[sp-1].Real=-stack[sp-1].Real;
            goto INC_NEXT_INSTR;

            // Branches

        IFG_R:
            sp-=2;
            ++ip;
            if (stack[sp].Real>stack[sp+1].Real)
                ip=p[ip]-1;
            goto INC_NEXT_INSTR;

            // Casts

        R_TO_I:
            stack[sp-1].Int=(TSINT)stack[sp-1].Real;
            goto INC_NEXT_INSTR;

            // Others

        PRINT_R:
            --sp;
            std::cout.precision(10);
            std::cout<<stack[sp].Real;
            std::cout.precision(6);
            std::cout.flush();
            goto INC_NEXT_INSTR;

        RAW_ARRAY:
            {
                size_t nb=p[++ip];
                size_t* ary=new size_t[nb];
                size_t max=0;

                for (size_t i=0; i<nb; ++i){
                    ary[i]=p[++ip];
                    if(ary[i]!=std::string::npos)
                        max=std::max(max, ary[i]);
                }

                stack[sp++].Ref=new size_t_Array(nb==0 ? 0 : max+1, ary);
            }
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // FUNCTIONS OPERATIONS
            ///////////////////////////////////////

        MK_FUNC:
            {
                size_t max_sz=p[++ip];
                size_t nb_locals=p[++ip];
                size_t nb_captures=p[++ip];
                size_t ref_map_index=p[++ip];
                size_t ex_table_index=p[++ip];
                size_t dbg_infos_index=p[++ip];
                size_t toIgnore=p[++ip];

                std::vector<TSDATA> captures(nb_captures);

                for (size_t i=0; i<nb_captures; ++i)
                    captures[nb_captures-i-1]=stack[--sp];

                stack[sp++].Ref=new objects::Function(captures,
                                                      Procedure(max_sz, nb_locals,
                                                                p+ip+1, toIgnore,
                                                                (size_t_Array**)cPool+ref_map_index,
                                                                (ExceptionTable**)cPool+ex_table_index,
                                                                dbg_infos_index==std::string::npos ? NULL : (DebugInfos**)cPool+dbg_infos_index,
                                                                proc->name()));

                ip+=toIgnore+1;

                if(USE_GC)
                    ctx->gc->trace(stack[sp-1].Instance);
            }
            goto INC_NEXT_INSTR;

        CALL_FUNC:
            {
                if (((objects::Function*)stack[sp-1].Ref)->getNative()) {
                    NAT_FUNC_CALL_OPER_RET
                    stack[sp++]=ret;
                }
                else goto CALL_USER;
            }
            goto INC_NEXT_INSTR;

        CALL_FUNC_R:
            {
                if (((objects::Function*)stack[sp-1].Ref)->getNative()) {
                    NAT_FUNC_CALL_OPER_RET
                    stack[sp++]=ret;

                    if(USE_GC)
                        ctx->gc->makeTemp((objects::Object*)stack[sp-1].Ref);
                }
                else goto CALL_USER;
            }
            goto INC_NEXT_INSTR;

        CALL_FUNC_V:
            {
                if (((objects::Function*)stack[sp-1].Ref)->getNative()) {
                    NAT_FUNC_CALL_OPER_NO_RET
                }
                else goto CALL_USER;
            }
            goto INC_NEXT_INSTR;

        CALL_USER:
            {
                objects::Function* funcToCall=(objects::Function*)stack[--sp].Ref; // get the function to call
                size_t nbArg=p[++ip]; // get the number of args

                // Load args on the stack into funcToCall's locals

                TSDATA* funcLocals=locals+proc->numberOfLocals();

                // Load captures

                const std::vector<TSDATA>& captures(funcToCall->captures());
                size_t e=funcToCall->captures().size();

                for (size_t i=0; i<e; ++i)
                    funcLocals[i]=captures[i];

                sp-=nbArg;

                for (size_t i=0; i<nbArg; ++i)
                    funcLocals[i+e]=stack[sp+i];

                cur_roots+=proc->getRefMap()->_count;

                // Save states

                stack[sp++].Int=(TSINT)start_sp; // save the start stack pointer
                stack[sp++].Ref=(void*)proc; // save the current procedures's pointer
                stack[sp++].Int=(TSINT)(ip+1+p[ip+1]); // save the instruction pointer

                // Adjust pointers

                locals=funcLocals;
                proc=funcToCall->getProcPtr();
                start_sp=sp;

                // store new references
                if (USE_GC){
                    size_t nbRef=p[++ip];
                    ++ip;
                    for(size_t i=0; i<nbRef; ++i, ++ip){
                        cur_roots[proc->getRefMap()->_array[p[ip]]]=locals[e+p[ip]];
                        SAFE_STORE_IF_TEMP(locals[e+p[ip]]);
                    }
                }

                // Get ByteCode to execute
                p=proc->byteCode();
                ip=0;

                ++ctx->csp; // increment call stack pointer

                // Increment roots array pointer

                roots_ary._count+=proc->getRefMap()->_count;

            }
            goto NEXT_INSTR;

        RET_R:
            if(USE_GC)
                if(stack[sp-1].Ref!=NULL)
                    ctx->gc->makeTemp((objects::Object*)stack[sp-1].Ref);

        RET:
            {
                if (ctx->csp==0){
                    if (USE_GC)
                        roots_ary._count-=proc->getRefMap()->_count;
                    return stack[--sp];
                }

                TSDATA value=stack[--sp]; // get the value to return
                sp=start_sp;
                roots_ary._count-=proc->getRefMap()->_count; // reset the roots counter

                ip=(TSINSTR)stack[--sp].Int; // reset instruction pointer
                proc=(const Procedure*)stack[--sp].Ref; // reset procedure pointer
                start_sp=stack[--sp].Int; // reset the start stack pointer
                locals=locals-proc->numberOfLocals(); // reset locals pointer

                cur_roots-=proc->getRefMap()->_count;

                p=proc->byteCode(); // reset bytecode

                stack[sp++]=value; // add returned value to the stack

                --ctx->csp; // decrement call stack pointer
            }
            goto INC_NEXT_INSTR;

        RET_V:

            if (ctx->csp==0){
                if (USE_GC)
                    roots_ary._count-=proc->getRefMap()->_count;
                return stack[0];
            }

            sp=start_sp;
            roots_ary._count-=proc->getRefMap()->_count; // reset the roots counter

            ip=(TSINSTR)stack[--sp].Int; // reset instruction pointer
            proc=(const Procedure*)stack[--sp].Ref; // reset procedure pointer
            start_sp=stack[--sp].Int; // reset the start stack pointer
            locals=locals-proc->numberOfLocals(); // reset locals pointer

            cur_roots-=proc->getRefMap()->_count;

            p=proc->byteCode(); // reset bytecode

            --ctx->csp; // decrement call stack pointer

            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // CLASS OPERATIONS
            ///////////////////////////////////////

        MK_CLASS:
            {
                size_t nb_attrs=p[++ip];
                size_t nb_defs=p[++ip];
                size_t debugInfo_defindex=p[++ip];
                size_t superclass_defindex=p[++ip];
                size_t destructorIndex=p[++ip];
                size_t nb_refs=p[++ip];

                TSDATA* defs=new TSDATA[nb_defs];
                for (size_t i=nb_defs; i!=0; --i)
                    defs[i-1]=stack[--sp];

                std::vector<size_t> refs(nb_refs);
                for (size_t i=0; i<nb_refs; ++i)
                    refs[i]=p[++ip];

                DebugInfos** debugInfoPtr=(debugInfo_defindex==std::string::npos ? NULL : (DebugInfos**)cPool+debugInfo_defindex);
                objects::Class** superClassPtr=(superclass_defindex==std::string::npos ? NULL : (objects::Class**)cPool+superclass_defindex);

                size_t nbInterface=p[++ip];
                TSDATA** realItable=NULL;

                if(nbInterface>0){

                    size_t highestIndex=p[ip+1];
                    size_t lowestIndex=highestIndex;
                    std::vector<TSDATA*> itable(highestIndex+1);

                    for(size_t i=0; i<nbInterface; ++i){
                        size_t interfaceID=p[++ip];

                        if(interfaceID>highestIndex){
                            highestIndex=interfaceID;
                            itable.resize(interfaceID+1);
                        }
                        else if(interfaceID<lowestIndex){
                            lowestIndex=interfaceID;
                        }

                        size_t nbFunc=p[++ip];
                        itable[interfaceID]=new TSDATA[nbFunc];

                        for(size_t j=0; j<nbFunc; ++j){
                            itable[interfaceID][j]=defs[p[++ip]];
                        }
                    }

                    realItable=new TSDATA*[highestIndex-lowestIndex+1];
                    realItable-=lowestIndex;

                    for(size_t i=lowestIndex; i<highestIndex+1; ++i){
                        realItable[i]=itable[i];
                    }
                }

                stack[sp++].Ref=new objects::Class(nb_attrs, superClassPtr, defs, realItable, refs, destructorIndex, debugInfoPtr, proc->name());

            }
            goto INC_NEXT_INSTR;

        NEW:
            stack[sp-1]=((objects::Class*)stack[sp-1].Ref)->newInstance();
            if (USE_GC)
                ctx->gc->trace((objects::Object*)stack[sp-1].Ref);
            goto INC_NEXT_INSTR;

        NEW_SCOPED:
            {
                objects::Instance* inst=new (data_stack+data_sp) objects::Instance((objects::Class*)stack[sp-1].Ref,
                                                                                   data_stack+data_sp+sizeof(objects::Instance));
                inst->unmark();
                stack[sp-1].Ref=inst;
                const size_t instanceSize=sizeof(objects::Instance)+sizeof(TSDATA)*inst->getClass()->numberOfAttributes();
                data_sp+=instanceSize;
                new (data_stack+data_sp) size_t(instanceSize);
                data_sp+=sizeof(size_t);
            }
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // INSTANCE OPERATIONS
            ///////////////////////////////////////

        GET_F:
            STORE_IF_TEMP(stack[sp-1])
            stack[sp-1]=stack[sp-1].Instance->getAttr(p[++ip]);
            goto INC_NEXT_INSTR;
        SET_F:
            STORE_IF_TEMP(stack[sp-1])
            sp-=2;
            stack[sp+1].Instance->setAttr(p[++ip], stack[sp]);
            goto INC_NEXT_INSTR;

        SET_F_R:
            STORE_IF_TEMP(stack[sp-1])
            sp-=2;
            stack[sp+1].Instance->setAttr(p[++ip], stack[sp]);
            SAFE_STORE_IF_TEMP(stack[sp])
            goto INC_NEXT_INSTR;

        GET_V:
            if(ip<=2 || p[ip-2]!=DUP){
                STORE_IF_TEMP(stack[sp-1])
            }
            stack[sp-1]=stack[sp-1].Instance->getVirtual(p[++ip]);
            goto INC_NEXT_INSTR;

        GET_I:
            if(ip<=2 || p[ip-2]!=DUP){
                STORE_IF_TEMP(stack[sp-1])
            }
            stack[sp-1]=stack[sp-1].Instance->getImplementation(p[ip+1], p[ip+2]);
            ip+=2;
            goto INC_NEXT_INSTR;

        LD_0_GET_F:
            STORE_IF_TEMP(locals[0])
            stack[sp++]=locals[0].Instance->getAttr(p[++ip]);
            goto INC_NEXT_INSTR;

        LD_0_SET_F:
            STORE_IF_TEMP(locals[0])
            locals[0].Instance->setAttr(p[++ip], stack[--sp]);
            goto INC_NEXT_INSTR;

        LD_0_SET_F_R:
            STORE_IF_TEMP(locals[0])
            locals[0].Instance->setAttr(p[++ip], stack[--sp]);
            SAFE_STORE_IF_TEMP(stack[sp])
            goto INC_NEXT_INSTR;

        LD_CALL_V:
            stack[sp++]=locals[p[++ip]];
            stack[sp]=stack[sp-1].Instance->getVirtual(p[++ip]);
            ++sp;
            goto CALL_FUNC;

        INSTANCEOF:
            --sp;
            stack[sp-1].Int=stack[sp-1].Instance->instanceOf((objects::Class*)stack[sp].Ref);
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // STRING OPERATIONS
            ///////////////////////////////////////

        MK_STRING:
            {
                size_t count=p[++ip];
                ++ip;
                size_t base=ip;
                std::string* array=new std::string;
                array->resize(count);
                count+=ip;
                for (; ip<count; ++ip)
                    array->at(ip-base)=p[ip];

                stack[sp++].Ref=array;
            }
            goto NEXT_INSTR;

        PRINT_STR:
            SAFE_STORE_IF_TEMP(stack[sp-1])
            std::cerr<<*((std::string*)stack[--sp].Instance->getAttr(0).Ref);
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // ARRAY OPERATIONS
            ///////////////////////////////////////

        NEW_ARRAY:
            stack[sp-1].Ref=new objects::Array(stack[sp-1].Int, p[++ip]);
            if (USE_GC)
                ctx->gc->trace((objects::Array*)stack[sp-1].Ref);
            goto INC_NEXT_INSTR;

        MK_ARRAY:
            {
                objects::Array* obj=new objects::Array((TSINT)p[++ip], p[++ip]);

                for (TSINT i=obj->length()-1; (TSINT)i>-1; --i)
                    obj->array()[i]=stack[--sp];

                stack[sp++].Ref=obj;

                if (USE_GC)
                    ctx->gc->trace(obj);
            }
            goto INC_NEXT_INSTR;

        GET_AT:
            --sp;
            stack[sp-1]=((objects::Array*)stack[sp-1].Ref)->array()[stack[sp].Int];
            goto INC_NEXT_INSTR;

        SET_AT:
            sp-=2;
            ((objects::Array*)stack[sp].Ref)->array()[stack[sp+1].Int]=stack[sp-1];
            --sp;
            goto INC_NEXT_INSTR;

        SET_AT_R:
            sp-=2;
            ((objects::Array*)stack[sp].Ref)->array()[stack[sp+1].Int]=stack[sp-1];
            --sp;
            SAFE_STORE_IF_TEMP(stack[sp])
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // TUPLE OPERATIONS
            ///////////////////////////////////////

        NEW_TUPLE:
            stack[sp++].Ref=new objects::Tuple(p[++ip], (size_t_Array**)(cPool+p[++ip]));
            if (USE_GC)
                ctx->gc->trace((objects::Array*)stack[sp-1].Ref);
            goto INC_NEXT_INSTR;

        MK_TUPLE:
            {
                size_t nbElem=p[++ip];
                TSDATA* elems=new TSDATA[nbElem];

                for (size_t i=0; i<nbElem; ++i)
                    elems[nbElem-i-1]=stack[--sp];

                objects::Tuple* obj=new objects::Tuple(elems, (size_t_Array**)(cPool+p[++ip]));

                stack[sp++].Ref=obj;

                if (USE_GC)
                    ctx->gc->trace(obj);
            }
            goto INC_NEXT_INSTR;

        GET_TE:
            stack[sp-1]=((objects::Tuple*)stack[sp-1].Ref)->getAttr(p[++ip]);
            goto INC_NEXT_INSTR;
        SET_TE:
            sp-=2;
            ((objects::Tuple*)stack[sp+1].Ref)->setAttr(p[++ip], stack[sp]);
            goto INC_NEXT_INSTR;

        SET_TE_R:
            sp-=2;
            ((objects::Tuple*)stack[sp+1].Ref)->setAttr(p[++ip], stack[sp]);
            SAFE_STORE_IF_TEMP(stack[sp])
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // SCOPE ALLOCATIONS
            ///////////////////////////////////////

        INIT_SCOPE:
            new (data_stack+data_sp) size_t(data_sp);
            data_stack+=data_sp+sizeof(size_t);
            data_sp=0;
            goto INC_NEXT_INSTR;

        CLEAN_SCOPE:
            {
                while (data_sp>0) {
                    size_t size=*((size_t*)(data_stack+data_sp-sizeof(size_t)));
                    data_sp-=size+sizeof(size_t);
                    ((objects::Object*)(data_stack+data_sp))->freeThis(ctx);
                    ((objects::Object*)(data_stack+data_sp))->~Object();
                }
                data_sp=*((size_t*)(data_stack-sizeof(size_t)));
                data_stack-=data_sp+sizeof(size_t);
            }
            goto INC_NEXT_INSTR;

        LU_TABLE:
            {
                --sp;
                size_t end=p[ip+3];
                TSINT val=stack[sp].Int;
                if (val<p[ip+1] || val>p[ip+2]){
                    ip=end;
                    goto NEXT_INSTR;
                }
                ip+=4;

                size_t count=p[ip++];

                for (size_t i=0; i<count; i=i+1, ip+=2) {
                    if (val==p[ip]) {
                        ip=p[ip+1];
                        goto NEXT_INSTR;
                    }
                }

                ip=end;
            }
            goto NEXT_INSTR;

        J_TABLE:
            {
                --sp;
                size_t end=p[ip+2];
                size_t count=p[ip+3];
                TSINT val=stack[sp].Int-(TSINT)p[ip+1];
                if (val<0 || val>=count){
                    ip=end;
                    goto NEXT_INSTR;
                }

                ip=p[ip+4+((size_t)val)];
            }
            goto NEXT_INSTR;

            ///////////////////////////////////////
            // EXCEPTIONS
            ///////////////////////////////////////

        MK_EX_TABLE:
            {
                size_t nb_try=p[++ip];
                ExceptionTable* ex=new ExceptionTable(nb_try);
                for(size_t i=0; i<nb_try; ++i){
                    ex->at(i).tryBlockIPStart=p[++ip];
                    ex->at(i).tryBlockIPEnd=p[++ip];

                    size_t nb_catch=p[++ip];
                    for(size_t j=0; j<nb_catch; ++j){
                        size_t localIndex=p[++ip];
                        size_t classIndex=p[++ip];
                        size_t catchEntryIP=p[++ip];

                        ex->at(i).catches.push_back(ExceptionTableCatch{localIndex, classIndex, catchEntryIP});
                    }
                }

                stack[sp++].Ref=ex;
            }
            goto INC_NEXT_INSTR;

        THROW:

            TSDATA val=stack[--sp];
            objects::Instance* ex=val.Instance;

            while(ctx->csp>=0){

                // We first try to find a suitable try-block in the current function

                const ExceptionTable* table=proc->getExTable();

                // traverse every try-blocks of the function and find one which start and end ips are around the current ip

                for(size_t i=0; i<table->size(); ++i){
                    if(table->operator [](i).tryBlockIPStart<ip && table->operator [](i).tryBlockIPEnd>ip){
                        // we are inside a try block. try to find a suitable catch clause

                        const std::vector<ExceptionTableCatch>& catches(table->at(i).catches);
                        for(size_t j=0; j<catches.size(); ++j){

                            if(ex->instanceOf((objects::Class*)cPool[catches[j].classIndex].Ref)){

                                // found a suitable catch. store the exception in the local variable

                                locals[catches[j].localIndex].Instance=ex;

                                if (USE_GC){
                                    cur_roots[proc->getRefMap()->_array[catches[j].localIndex]]=val;
                                    SAFE_STORE_IF_TEMP(val)
                                }

                                // load the code of the catch clause.

                                ip=catches[j].catchEntryIP;
                                goto NEXT_INSTR;
                            }

                        }
                    }
                }

                // else return

                sp=start_sp;
                roots_ary._count-=proc->getRefMap()->_count; // reset the roots counter

                ip=(TSINSTR)stack[--sp].Int; // reset instruction pointer
                proc=(const Procedure*)stack[--sp].Ref; // reset procedure pointer
                start_sp=stack[--sp].Int; // reset the start stack pointer
                locals=locals-proc->numberOfLocals(); // reset locals pointer

                cur_roots-=proc->getRefMap()->_count;

                p=proc->byteCode(); // reset bytecode

                --ctx->csp; // decrement call stack pointer
            }

            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // CONTROL FLOW
            ///////////////////////////////////////

        GOTO:
            ip=p[ip+1];
            goto NEXT_INSTR;

        EXIT:
            return TSDATA();

            ///////////////////////////////////////
            // DEBUG INSTRUCTIONS
            ///////////////////////////////////////

        MK_DBG_INFO:
            VarDeclTable vars(p[++ip]);

            for(size_t i=0; i<vars.size(); ++i){
                vars[vars.size()-i-1].type=(DEBUG_TYPE)stack[--sp].Int;
                vars[vars.size()-i-1].name=(std::string*)stack[--sp].Ref;
                vars[vars.size()-i-1].index=(size_t)stack[--sp].Int;
            }

            stack[sp++].Ref=new DebugInfos(vars);
            goto INC_NEXT_INSTR;

        DBG_EOS:
            if(Debugger){

                size_t pos=p[++ip];
                if(Debugger->shouldBreak(pos)){
                    Debugger->updateDebuggingInfo(pos, locals, proc->numberOfLocals());
                    Debugger->readMessage();
                }

            }else{
                ++ip;
            }
            goto INC_NEXT_INSTR;

        DBG_CALL:
            if(Debugger){
                ++ip;
                objects::Function* func=((objects::Function*)stack[sp-1].Ref);
                Debugger->functionCalled(func->getProc().name(), func);
            }else{
                ++ip;
            }
            goto INC_NEXT_INSTR;

        DBG_RET:
            if(Debugger){
                Debugger->functionReturned();
            }

            goto INC_NEXT_INSTR;

        DBG_NEW:
            if(Debugger){
                Debugger->dynAllocation();
            }
            goto INC_NEXT_INSTR;
        }

        return TSDATA();
    }

    void ________(){
        Interpreter::exec<false>(NULL, NULL);
        Interpreter::exec<true>(NULL, NULL);
    }
}
