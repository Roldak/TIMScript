//
//  Utils.cpp
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Utils.h"
#include "Bytecode.h"

size_t nbOfArgs(TSINSTR instr, const TSINSTR* rest){
    using namespace ts;
    
    switch (instr) {
            
        // VARIABLES
            
        case STORE_N:   return 1;
        case LOAD_N:    return 1;
        case STORE_R:   return 1;
        case GC_STORE:  return 0;
        case LOAD_CST:  return 1;
        case LOAD_NAT:  return 1;
        case DUP:       return 1;
        case POP:       return 0;
        case POP_R:     return 0;
            
        // BOOLEANS
            
        case PUSH_FALSE:return 0;
        case PUSH_TRUE: return 0;
            
        case AND_B:     return 0;
        case OR_B:      return 0;
            
        case PRINT_B:   return 0;
            
        // INTEGERS
            
        case PUSH_I:    return 1;
            
        case ADD_I:     return 0;
        case SUB_I:     return 0;
        case MUL_I:     return 0;
        case DIV_I:     return 0;
        case MOD_I:     return 0;
        case POW_I:     return 0;
        case CMP_I_L:   return 0;
        case CMP_I_G:   return 0;
        case CMP_I_EQ:  return 0;
        case CMP_I_NEQ: return 0;
            
        case INC_I:     return 2;
        case INC_I_1:   return 1;
        case DEC_I:     return 2;
        case DEC_I_1:   return 1;
        case NEG_I:     return 0;
            
        case IF_I_TRUE: return 1;
        case IF_I_FALSE:return 1;
        case IFG_I:     return 1;
            
        case I_TO_R:    return 0;
            
        case PRINT_I:   return 0;
            
        // REALS
            
        case PUSH_R:    return 1;
            
        case ADD_R:     return 0;
        case SUB_R:     return 0;
        case MUL_R:     return 0;
        case DIV_R:     return 0;
        case MOD_R:     return 0;
        case POW_R:     return 0;
        case CMP_R_L:   return 0;
        case CMP_R_G:   return 0;
        case CMP_R_EQ:  return 0;
        case CMP_R_NEQ: return 0;
            
        case INC_R:     return 2;
        case INC_R_1:   return 1;
        case DEC_R:     return 2;
        case DEC_R_1:   return 1;
        case NEG_R:     return 0;
            
        case IFG_R:     return 1;
            
        case R_TO_I:    return 0;
            
        case PRINT_R:   return 0;
            
        // RAW DATAS
            
        case RAW_ARRAY: return 1+rest[0];
            
        // FUNCTIONS OPERATIONS
            
        case MK_FUNC:   return 6;
        case CALL_FUNC: //return 2+rest[1];
        case CALL_FUNC_R://return 2+rest[1];
        case CALL_FUNC_V:return 2+rest[1];
        case RET_R:     return 0;
        case RET:       return 0;
        case RET_V:     return 0;
            
        // CLASS OPERATIONS
            
        case MK_CLASS:  {
            size_t length=4;
            size_t refAmongAttrNb=rest[length++];
            length+=refAmongAttrNb;
            size_t nbInterfaces=rest[length++];
            for(size_t i=0; i<nbInterfaces; ++i){
                ++length; // interfaceID
                size_t nbFunc=rest[length++];
                length+=nbFunc;
            }
            return length;
        }
        case NEW:       return 0;
        case NEW_SCOPED:return 0;
            
        // INSTANCE OPERATIONS
            
        case GET_F:     return 1;
        case SET_F:     return 1;
        case SET_F_R:   return 1;
        case GET_V:     return 1;
        case GET_I:     return 2;
        case LD_0_GET_F:return 1;
        case LD_0_SET_F:return 1;
        case LD_0_SET_F_R:return 1;
            
        case LD_CALL_V: return 3;
        case INSTANCEOF:return 0;
            
        // STRING OPERATIONS
            
        case MK_STRING: return 1+rest[0];
        case PRINT_STR: return 0;
            
        // ARRAY OPERATIONS
            
        case NEW_ARRAY: return 1;
        case MK_ARRAY:  return 2;
        case GET_AT:    return 0;
        case SET_AT:    return 0;
        case SET_AT_R:  return 0;
            
        // TUPLE OPERATIONS
            
        case NEW_TUPLE: return 2;
        case MK_TUPLE:  return 2;
        case GET_TE:    return 1;
        case SET_TE:    return 1;
        case SET_TE_R:  return 1;
            
        // SCOPE ALLOCATIONS
            
        case INIT_SCOPE:return 0;
        case CLEAN_SCOPE:return 0;
            
        // JUMP/LOOKUP TABLE
            
        case LU_TABLE:  return 4+2*rest[3];
        case J_TABLE:   return 3+rest[2];
            
        // EXCEPTIONS

        case MK_EX_TABLE:{
            size_t length=0;
            size_t nb_try=rest[length++];
            for(size_t i=0; i<nb_try; ++i){
                length+=2; // ip start, ip end of try block
                size_t nb_catch=rest[length++];
                for(size_t j=0; j<nb_catch; ++j){
                    ++length; // index in the local variable array of the exception
                    ++length; // index in constant pool of exception class
                    ++length; // ip of the catch block
                }
            }
            return length;
        }

        case THROW:     return 0;

        // CONTROL FLOW
            
        case GOTO:      return 1;
        case EXIT:      return 0;
            
        // DEBUG INSTRUCTIONS

        case MK_DBG_INFO:return 1;

        case DBG_EOS:   return 1;
        case DBG_CALL:  return 1;
        case DBG_RET:   return 0;

        default:
            break;
    }
    
    return 0;
}

bool isBranch(TSINSTR instr){
    using namespace ts;
    
    switch (instr) {
        case IF_I_TRUE:
        case IF_I_FALSE:
        case IFG_I:
        case IFG_R:
        case GOTO:
            return true;
            
        default:
            return false;
    }
}
