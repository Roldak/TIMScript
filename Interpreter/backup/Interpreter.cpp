//
//  Interpreter.cpp
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Interpreter.h"
#include "Bytecode.h"
#include "Function.h"
#include "Class.h"
#include "Instance.h"
#include "Array.h"
#include "Tuple.h"
#include "NativeData.h"
#include "InterpreterDefines.h"
#include "BytecodePrinter.h"
#include "GC.h"
#include <unistd.h>

#include <cmath>

namespace ts {
    
    objects::Instance* ExecutionContext::newInstance(size_t clssLocation){
        return ((objects::Class*)cPool[clssLocation].Ref)->newInstance().Instance;
    }
    
    TSDATA ExecutionContext::callFunction(size_t funcLocation, const std::vector<TSDATA>& args){
        objects::Function* f=(objects::Function*)cPool[funcLocation].Ref;
        TSDATA* locals=new TSDATA[f->getProc().numberOfLocals()];
        for (size_t i=0; i<args.size(); ++i)
            locals[i]=args[i];
        
        return Interpreter::exec<true, true>(this, f->getProcPtr(), locals);
    }
    
    Interpreter::Interpreter(Program* p) : _prog(p), _cp_alrdyComputed(false) {}
    
    Interpreter::~Interpreter(){
        if (_cp_alrdyComputed)
            free(_constantPool);
    }
    
    TSDATA Interpreter::run(){
        if (!_cp_alrdyComputed)
            computeConstantPool();
        return exec<true, false>(_constantPool, _prog->nativesPool(), _prog->localsRefs(), _prog->procedure());
    }
    
    TSDATA Interpreter::run(size_t funcLoc, const std::vector<TSDATA>& args){
        
    }
    
    void Interpreter::computeConstantPool(){
        _cp_alrdyComputed=true;
        _constantPool=(TSDATA*)malloc(sizeof(TSDATA)*_prog->constantPool().size());
        for (size_t i=0; i<_prog->constantPool().size(); ++i)
            _constantPool[i]=exec<false, false>(_constantPool, _prog->nativesPool(), _prog->localsRefs(), _prog->constantPool()[i]);
    }
    
    /* runs the given bytecode (inside the procedure), according to the number of local variables and the supposed stack size */
    
    template<bool USE_GC, bool AS_THREAD>
    TSDATA Interpreter::exec(ExecutionContext* ctx, const Procedure* proc, TSDATA* args){
        
        // get the content of the execution context
        
        GarbageCollector& gc(ctx->gc);
        const TSDATA* cPool(ctx->cPool);
        const TSDATA* nPool(ctx->nPool);
        const size_t_Array* locrefs(ctx->locrefs);
        
        // create the arrays that will store states for each function call
        
        const TSINSTR* p_ary[MAX_RECURSION_DEPTH];
        TSDATA* stack_ary[MAX_RECURSION_DEPTH];
        TSDATA* locals_ary[MAX_RECURSION_DEPTH];
        TSINSTR ip_ary[MAX_RECURSION_DEPTH];
        data_Array roots_ary[MAX_RECURSION_DEPTH];
        size_t sp_ary[MAX_RECURSION_DEPTH];
        size_t_Array locRef_tables[MAX_RECURSION_DEPTH];
        
        size_t csp=0; // init callstack pointer to 0
        
        p_ary[csp]=proc->byteCode(); // bytecode of the procedure
        stack_ary[csp]=(TSDATA*)malloc(NB_BYTES_DATA*proc->stackSize()); // operand stack
        
        if (args)
            locals_ary[csp]=args;
        else
            locals_ary[csp]=(TSDATA*)malloc(NB_BYTES_DATA*proc->numberOfLocals()); // local variables
        
        locRef_tables[csp]=locrefs[0];
        roots_ary[csp]._count=locRef_tables[0]._count;
        roots_ary[csp]._array=(TSDATA*)calloc(roots_ary[csp]._count, NB_BYTES_DATA); /* roots variables.
                                                                                             it's necessary to put every byte to 0*/
        const TSINSTR* p(p_ary[csp]);
        TSDATA* stack(stack_ary[csp]);
        TSDATA* locals(locals_ary[csp]);
        
        char* data_stack=new char[10000000];
        size_t data_sp=0;
        
        TSINSTR ip=0; // instruction pointer
        size_t sp=0; // stack pointer
        
        gc.reset(roots_ary, &csp);
        
        static const void* jump_table[]{
            // Variables
            
            &&STORE_N, &&LOAD_N, &&STORE_R, &&LOAD_CST, &&LOAD_NAT, &&DUP, &&POP,
            
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
            
            // Functions operations
            
            &&MK_FUNC, &&CALL_FUNC, &&CALL_NAT, &&CALL_NAT_V, &&RET, &&RET_V,
            
            // Class operations
            
            &&MK_CLASS, &&NEW, &&NEW_SCOPED,
            
            // Instance operations
            
            &&GET_F, &&SET_F, &&SET_F_R, &&GET_V,
            
            // String operations
            
            &&MK_STRING, &&PRINT_STR,
            
            // Array operations
            
            &&NEW_ARRAY, &&MK_ARRAY, &&GET_AT, &&SET_AT, &&SET_AT_R,
            
            // Tuple operations
            
            &&NEW_TUPLE, &&MK_TUPLE, &&GET_TE, &&SET_TE, &&SET_TE_R,
            
            // Other control flow operations
            
            &&DEL_SCOPED,
            &&GOTO, &&EXIT, &&RET_STACK_0
        };
        
        while (true) {
        begin:
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
            if (USE_GC) {
                roots_ary[csp]._array[locRef_tables[csp]._array[p[ip]]]=stack[sp]; /* use the table to set the value 
                                                                                    of the reference in the roots variables array*/
                gc.store((objects::Object*)stack[sp].Ref);
            }
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
            std::cout.precision(50);
            std::cout<<stack[sp].Real;
            std::cout.precision(6);
            std::cout.flush();
            goto INC_NEXT_INSTR;

            ///////////////////////////////////////
            // FUNCTIONS OPERATIONS
            ///////////////////////////////////////
            
        MK_FUNC:
            {
                size_t max_sz=p[++ip];
                size_t nb_locals=p[++ip];
                size_t nb_captures=p[++ip];
                size_t func_index=p[++ip];
                size_t toIgnore=p[++ip];
                
                std::vector<TSDATA> captures(nb_captures);
                
                for (size_t i=0; i<nb_captures; ++i)
                    captures[i]=stack[--sp];
                
                stack[sp++].Ref=new objects::Function(captures, locrefs[func_index], Procedure(max_sz, nb_locals, p+ip+1));
                
                if(USE_GC)
                    gc.trace((objects::Object*)stack[sp-1].Ref);
                
                ip+=toIgnore+1;
            }
            goto INC_NEXT_INSTR;
            
        CALL_FUNC:
            {
                size_t nbArg=p[++ip]; // get the number of argument
                objects::Function* func=(objects::Function*)stack[--sp].Ref; // get the referenced function
                
                ++csp; // increment the call stack pointer 
                
                const std::vector<TSDATA>& captures(func->captures()); // get the captures stored by the function
                p_ary[csp]=func->getProc().byteCode(); // get the bytecode of the function
                func->allocateResources(stack_ary[csp], locals_ary[csp], roots_ary[csp]); // allocate the stack and the local variables array
                locRef_tables[csp]=func->getLocRefTable(); // set the function index for the current call
                
                sp-=nbArg;
                
                for (size_t i=0; i<captures.size(); ++i) // copy the captures in the local variable array of the function
                    locals_ary[csp][i]=captures[i];
                for (size_t i=0; i<nbArg; ++i) // copy the arguments of the stack on the local variable array of the function
                    locals_ary[csp][i+captures.size()]=stack[sp+i];
                
                if(USE_GC){
                    for (size_t i=0; i<locRef_tables[csp]._count; ++i) {
                        size_t pos=locRef_tables[csp]._array[i];
                        if (pos>=captures.size() && pos<nbArg+captures.size()) {
                            if(locals_ary[csp][pos].Ref){
                                roots_ary[csp]._array[pos]=locals_ary[csp][pos];
                                gc.store((objects::Object*)locals_ary[csp][pos].Ref);
                            }
                        }
                    }
                }
                
                ip_ary[csp-1]=ip; // save the state of the instruction pointer
                sp_ary[csp-1]=sp; // save the state of the stack pointer
                ip=0; // reset instruction pointer to the beginning of the function's bytecode
                sp=1; /* reset the stack pointer to 1 
                       (to position 0 is assigned a pointer to the function so that we can free resources later on)*/
        
                p=p_ary[csp]; // adjust the 'current' bytecode
                stack=stack_ary[csp]; // adjust the 'current' stack array
                locals=locals_ary[csp]; // adjust the 'current' locals array
                
                stack[0].Ref=func; // set the element at position 0 of the stack as a reference to the function
                
            }
            goto NEXT_INSTR;
            
        CALL_NAT:
            {
                size_t nbArg=p[++ip]; // get the number of argument
                objects::NativeFunction* func=(objects::NativeFunction*)stack[--sp].Ref; // get the referenced native function
                sp-=nbArg;
                stack[sp]=func->call(ctx, stack+sp);
                ++sp;
            }
            goto INC_NEXT_INSTR;
            
        CALL_NAT_V:
            {
                size_t nbArg=p[++ip]; // get the number of argument
                objects::NativeFunction* func=(objects::NativeFunction*)stack[--sp].Ref; // get the referenced native function
                sp-=nbArg;
                func->call(ctx, stack+sp);
            }
            goto INC_NEXT_INSTR;
            
        RET:
            
            --csp;
            stack_ary[csp][sp_ary[csp]]=stack[--sp]; // put the last element of the stack on the stack of the caller
            
            ((objects::Function*)stack[0].Ref)->freeResources(); // free the stack array and the locals array
            
            stack=stack_ary[csp]; // reset the currentstack variable to the stack of the original caller
            locals=locals_ary[csp]; // idem for the locals
            
            p=p_ary[csp]; // reset the bytecode  state
            ip=ip_ary[csp]; // reset the instruction pointer state
            sp=sp_ary[csp]+1; // reset the stack pointer state, +1 because of the returned value.
            
            goto INC_NEXT_INSTR;
            
        RET_V:
            
            if (AS_THREAD) {
                if (csp==0) {
                    goto EXIT;
                }
            }
            
            --csp;
            
            ((objects::Function*)stack[0].Ref)->freeResources(); // free the stack array and the locals array
            
            stack=stack_ary[csp]; // reset the currentstack variable to the stack of the original caller
            locals=locals_ary[csp]; // idem for the locals
            
            p=p_ary[csp]; // reset the bytecode  state
            ip=ip_ary[csp]; // reset the instruction pointer state
            sp=sp_ary[csp]; // reset the stack pointer state, +1 because of the returned value.
            
            goto INC_NEXT_INSTR;
            
            ///////////////////////////////////////
            // CLASS OPERATIONS
            ///////////////////////////////////////
            
        MK_CLASS:
            {
                size_t nb_attrs=p[++ip];
                size_t nb_defs=p[++ip];
                size_t destructorIndex=p[++ip];
                size_t nb_refs=p[++ip];
                
                TSDATA* defs=new TSDATA[nb_defs];
                for (size_t i=nb_defs; i!=0; --i)
                    defs[i-1]=stack[--sp];
                
                std::vector<size_t> refs(nb_refs);
                for (size_t i=0; i<nb_refs; ++i)
                    refs[i]=p[++ip];
                
                stack[sp++].Ref=new objects::Class(nb_attrs, defs, refs, destructorIndex);
                
                if(USE_GC)
                    gc.trace((objects::Object*)stack[sp-1].Ref);
                
            }
            goto INC_NEXT_INSTR;
            
        NEW:
            stack[sp-1]=((objects::Class*)stack[sp-1].Ref)->newInstance();
            if (USE_GC)
                gc.trace(stack[sp-1].Instance);
            goto INC_NEXT_INSTR;
            
        NEW_SCOPED:
            {
                objects::Instance* inst=new (data_stack+data_sp) objects::Instance((objects::Class*)stack[sp-1].Ref,
                                                                                   data_stack+data_sp+sizeof(objects::Instance));
                inst->unmark();
                stack[sp-1].Ref=inst;
                data_sp+=sizeof(objects::Instance)+sizeof(TSDATA)*inst->getClass()->numberOfAttributes();
                new (data_stack+data_sp) size_t(sizeof(objects::Instance)+sizeof(TSDATA)*inst->getClass()->numberOfAttributes());
                data_sp+=sizeof(size_t);
            }
            goto INC_NEXT_INSTR;
            
            ///////////////////////////////////////
            // INSTANCE OPERATIONS
            ///////////////////////////////////////
            
        GET_F:
            stack[sp-1]=((objects::Instance*)stack[sp-1].Ref)->getAttr(p[++ip]);
            goto INC_NEXT_INSTR;
        SET_F:
            ((objects::Instance*)stack[--sp].Ref)->setAttr(p[++ip], stack[--sp]);
            goto INC_NEXT_INSTR;
            
        SET_F_R:
            ((objects::Instance*)stack[--sp].Ref)->setAttr(p[++ip], stack[--sp]);
            if (USE_GC)
                gc.store((objects::Object*)stack[sp].Ref);
            goto INC_NEXT_INSTR;
            
        GET_V:
            stack[sp-1]=((objects::Instance*)stack[sp-1].Ref)->getVirtual(p[++ip]);
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
            std::cerr<<*((std::string*)stack[--sp].Instance->getAttr(0).Ref);
            goto INC_NEXT_INSTR;
            
            ///////////////////////////////////////
            // ARRAY OPERATIONS
            ///////////////////////////////////////
            
        NEW_ARRAY:
            stack[sp-1].Ref=new objects::Array(stack[sp-1].Int, p[++ip]);
            if (USE_GC)
                gc.trace(((objects::Object*)stack[sp-1].Ref));
            goto INC_NEXT_INSTR;
            
        MK_ARRAY:
            {
                objects::Array* obj=new objects::Array((TSINT)p[++ip], p[++ip]);
                if (USE_GC)
                    gc.trace(obj);
                
                for (TSINT i=obj->length()-1; (TSINT)i>-1; --i)
                    obj->array()[i]=stack[--sp];
                
                if (obj->areRefs())
                    for(TSINT i=0; i<obj->length(); ++i)
                        gc.store((objects::Object*)obj->array()[i].Ref);
                
                stack[sp++].Ref=obj;
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
            gc.store((objects::Object*)stack[sp].Ref);
            goto INC_NEXT_INSTR;
            
            ///////////////////////////////////////
            // TUPLE OPERATIONS
            ///////////////////////////////////////
            
        NEW_TUPLE:
            stack[sp++].Ref=new objects::Tuple(p[++ip]);
            goto INC_NEXT_INSTR;
            
        MK_TUPLE:
            {
                size_t nbElem=p[++ip];
                TSDATA* elems=new TSDATA[nbElem];
                
                for (size_t i=0; i<nbElem; ++i)
                    elems[nbElem-i-1]=stack[--sp];
                
                objects::Tuple* obj=new objects::Tuple(elems);
                if (USE_GC)
                    gc.trace(obj);
                
                stack[sp++].Ref=obj;
            }
            goto INC_NEXT_INSTR;
            
        GET_TE:
            stack[sp-1]=((objects::Tuple*)stack[sp-1].Ref)->getAttr(p[++ip]);
            goto INC_NEXT_INSTR;
        SET_TE:
            ((objects::Tuple*)stack[--sp].Ref)->setAttr(p[++ip], stack[--sp]);
            goto INC_NEXT_INSTR;
            
        SET_TE_R:
            ((objects::Tuple*)stack[--sp].Ref)->setAttr(p[++ip], stack[--sp]);
            if (USE_GC)
                gc.store((objects::Tuple*)stack[sp].Ref);
            goto INC_NEXT_INSTR;
            
            ///////////////////////////////////////
            // CONTROL FLOW
            ///////////////////////////////////////
            
        DEL_SCOPED:
            {
                size_t nbToDelete=p[++ip];
                for (size_t i=0; i<nbToDelete; ++i) {
                    size_t size=*((size_t*)(data_stack+data_sp-sizeof(size_t)));
                    data_sp-=size+sizeof(size_t);
                    ((objects::Object*)(data_stack+data_sp))->~Object();
                }
            }
            goto INC_NEXT_INSTR;
            
        GOTO:
            ip=p[ip+1];
            goto NEXT_INSTR;
            
        EXIT:
            free(stack);
            free(locals);
            return TSDATA();
            
        RET_STACK_0:
            {
                TSDATA toRet=stack[0];
                free(stack);
                free(locals);
                return toRet;
            }
            
        out:
            ++ip;
        }
        
        free(stack);
        free(locals);
        return TSDATA();
    }
    
    // because the template function is implemented in the cpp
    EXPLICIT_INSTANTIATION
}