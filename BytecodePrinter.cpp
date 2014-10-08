//
//  BytecodePrinter.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "BytecodePrinter.h"
#include "Bytecode.h"

namespace ts {
    
    std::string BytecodePrinter::translate(const Procedure* proc){
        const TSINSTR* program=proc->byteCode();
        size_t ip=0;
        std::ostringstream oss;
        
        while (ip<proc->byteCodeLength()) {
            oss<<ip<<"\t"<<instrName(program[ip]);
            ++ip;
            
            size_t nb=ip+nbOfArgs(program[ip-1], program+ip);
            for (; ip<nb; ++ip) {
                oss<<"\t"<<program[ip];
            }
            
            oss<<"\n";
        }
        return oss.str();
    }
    
    std::string BytecodePrinter::instrName(TSINSTR instr){
        switch (instr) {
                
            // VARIABLES
                
            case STORE_N:   return "store_n";
            case LOAD_N:    return "load_n";
            case STORE_R:   return "store_r";
            case GC_STORE:  return "gc_trace";
            case LOAD_CST:  return "load_cst";
            case LOAD_NAT:  return "load_nat";
            case DUP:       return "dup";
            case POP:       return "pop";
            case POP_R:     return "pop_r";
                
            // BOOLEANS
                
            case PUSH_FALSE:return "push_false";
            case PUSH_TRUE: return "push_true";
                
            case AND_B:     return "and_b";
            case OR_B:      return "or_b";
                
            case PRINT_B:   return "print_b";
                
            // INTEGERS
                
            case PUSH_I:    return "push_i";
                
            case ADD_I:     return "add_i";
            case SUB_I:     return "sub_i";
            case MUL_I:     return "mul_i";
            case DIV_I:     return "div_i";
            case MOD_I:     return "mod_i";
            case POW_I:     return "pow_i";
            case CMP_I_L:   return "cmp_i_l";
            case CMP_I_G:   return "cmp_i_g";
            case CMP_I_EQ:  return "cmp_i_eq";
            case CMP_I_NEQ: return "cmp_i_neq";
                
            case INC_I:     return "inc_i";
            case INC_I_1:   return "inc_i_1";
            case DEC_I:     return "dec_i";
            case DEC_I_1:   return "dec_i_1";
            case NEG_I:     return "neg_i";
                
            case IF_I_TRUE: return "if_i_true";
            case IF_I_FALSE:return "if_i_false";
            case IFG_I:     return "ifg_i";
                
            case I_TO_R:    return "i_to_r";
                
            case PRINT_I:   return "print_i";
                
            // REALS
                
            case PUSH_R:    return "push_r";
                
            case ADD_R:     return "add_r";
            case SUB_R:     return "sub_r";
            case MUL_R:     return "mul_r";
            case DIV_R:     return "div_r";
            case MOD_R:     return "mod_r";
            case POW_R:     return "pow_r";
            case CMP_R_L:   return "cmp_r_l";
            case CMP_R_G:   return "cmp_r_g";
            case CMP_R_EQ:  return "cmp_r_eq";
            case CMP_R_NEQ: return "cmp_r_neq";
                
            case INC_R:     return "inc_r";
            case INC_R_1:   return "inc_r_1";
            case DEC_R:     return "dec_r";
            case DEC_R_1:   return "dec_r_1";
            case NEG_R:     return "neg_r";
                
            case IFG_R:     return "ifg_r";
                
            case R_TO_I:    return "r_to_i";
                
            case PRINT_R:   return "print_r";
                
            // RAW DATAS
                
            case RAW_ARRAY: return "raw_array";
                
            // FUNCTIONS OPERATIONS
                
            case MK_FUNC:   return "mk_func";
            case CALL_FUNC: return "call_func";
            case CALL_FUNC_R:return "call_func_r";
            case CALL_FUNC_V:return "call_func_v";
            case RET_R:     return "ret_r";
            case RET:       return "ret";
            case RET_V:     return "ret_v";
                
            // CLASS OPERATIONS
                
            case MK_CLASS:  return "mk_class";
            case NEW:       return "new";
            case NEW_SCOPED:return "new_scoped";
                
            // INSTANCE OPERATIONS
                
            case GET_F:     return "get_f";
            case SET_F:     return "set_f";
            case SET_F_R:   return "set_f_r";
            case GET_V:     return "get_v";
            case GET_I:     return "get_i";
                
            case LD_0_GET_F:return "ld_0_get_f";
            case LD_0_SET_F:return "ld_0_set_f";
            case LD_0_SET_F_R:return "ld_0_set_f_r";
                
            case LD_CALL_V: return "ld_call_v";
            case INSTANCEOF:return "instanceof";
                
            // STRING OPERATIONS
                
            case MK_STRING: return "mk_string";
            case PRINT_STR: return "print_str";
                
            // ARRAY OPERATIONS
                
            case NEW_ARRAY: return "new_array";
            case MK_ARRAY:  return "mk_array";
            case GET_AT:    return "get_at";
            case SET_AT:    return "set_at";
            case SET_AT_R:  return "set_at_r";
                
            // TUPLE OPERATIONS
                
            case NEW_TUPLE: return "new_tuple";
            case MK_TUPLE:  return "mk_tuple";
            case GET_TE:    return "get_te";
            case SET_TE:    return "set_te";
            case SET_TE_R:  return "set_te_r";
                
            // SCOPE ALLOCATIONS
                
            case INIT_SCOPE:return "init_scope";
            case CLEAN_SCOPE:return "clean_scope";
                
            // JUMP/LOOKUP TABLE
                
            case LU_TABLE:  return "lu_table";
            case J_TABLE:   return "j_table";

            // EXCEPTIONS

            case MK_EX_TABLE:return "mk_ex_table";
            case THROW:     return "throw";
                
            // CONTROL FLOW
                
            case GOTO:      return "goto";
            case EXIT:      return "exit";
                
            // DEBUG INSTRUCTIONS

            case MK_DBG_INFO:return "mk_dbg_info";

            case DBG_EOS:   return "dbg_eos";
            case DBG_CALL:  return "dbg_call";
            case DBG_RET:   return "dbg_ret";

            default:
                break;
        }
        
        return "";
    }
    
}
