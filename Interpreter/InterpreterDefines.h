//
//  InterpreterDefines.h
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef TIMScript_InterpreterDefines_h
#define TIMScript_InterpreterDefines_h

#define NEXT_INSTR *jump_table[p[ip]]
#define INC_NEXT_INSTR *jump_table[p[++ip]];

#define BINARY_OPER_CODE(opCode, operator, type)  opCode:\
                                                --sp; \
                                                stack[sp-1].type=stack[sp-1].type operator stack[sp].type; \
                                                goto INC_NEXT_INSTR;

#define BINARY_BOOLOPER_CODE(opCode, operator, type)  opCode:\
                                                --sp; \
                                                stack[sp-1].Int=(TSINT)(stack[sp-1].type operator stack[sp].type); \
                                                goto INC_NEXT_INSTR;


#define STORE_IF_TEMP(DATA)  \
                            if(USE_GC)\
                                if (DATA.Instance->marked()==objects::TEMPORARY) \
                                    ctx->gc->fastStore(DATA.Instance);

#define SAFE_STORE_IF_TEMP(DATA)  \
                            if(USE_GC)\
                                if (DATA.Instance && DATA.Instance->marked()==objects::TEMPORARY) \
                                    ctx->gc->fastStore(DATA.Instance);

#define NAT_FUNC_CALL_OPER(RET_EXP)  \
                            size_t nbArg=p[++ip]; \
                            const objects::NativeFunction* func=((objects::Function*)stack[--sp].Ref)->getNative();\
                            sp-=nbArg; \
                            TSDATA* oldStack=ctx->stack; \
                            ctx->stack=stack+sp; \
                            ctx->caller=proc; \
                            RET_EXP func->call(ctx, stack+sp); \
                            ctx->stack=oldStack; \
                            \
                            size_t nbRef=p[++ip]; \
                            \
                            for(size_t i=0; i<nbRef; ++i, ++ip) \
                                SAFE_STORE_IF_TEMP(stack[sp+p[ip+1]])

#define NAT_FUNC_CALL_OPER_NO_RET NAT_FUNC_CALL_OPER( )
#define NAT_FUNC_CALL_OPER_RET NAT_FUNC_CALL_OPER(TSDATA ret=)

#define EXPLICIT_INSTANTIATION void _____(){ \
    Interpreter::exec<true, true>(NULL, NULL, NULL, NULL); \
}

/*
template<>
TSDATA ts::Interpreter::exec<false, false>(ExecutionContext& ctx, const Procedure* proc);
*/
#endif
