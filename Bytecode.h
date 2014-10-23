//
//  Bytecode.h
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef TIMScript_Bytecode_h
#define TIMScript_Bytecode_h

namespace ts {
	enum OP_CODE {

		STORE_N, LOAD_N, // store number, load number (no need to distinguish int and real)
		STORE_R, // store reference (special case)
		GC_STORE, // trace object in gc
		LOAD_CST, // load constant from the constant pool
		LOAD_NAT, // load native definition from the native definitions pool

		// Stack operations

		DUP, // duplicate the value on top of the stack
		POP, // decrement stack pointer
		POP_R, // decrement stack pointer, while a reference lays at the top

		// Booleans operations

		PUSH_FALSE, PUSH_TRUE, // Constants
		AND_B, OR_B, // Binary operations
		// Unary operations
		// Branches
		PRINT_B,  // Others

		// Integers operations

		PUSH_I, // Constants
		ADD_I, SUB_I, MUL_I, DIV_I, MOD_I, POW_I, // Binary operations
		CMP_I_L, CMP_I_G, CMP_I_EQ, CMP_I_NEQ, // Comparisons
		INC_I, INC_I_1, DEC_I, DEC_I_1, NEG_I, // Unary operations
		IF_I_TRUE, IF_I_FALSE, IFG_I, // Branches
		I_TO_R, // Casts
		PRINT_I, // Others

		// Reals operations

		PUSH_R, // Constants
		ADD_R, SUB_R, MUL_R, DIV_R, MOD_R, POW_R, // Binary operations
		CMP_R_L, CMP_R_G, CMP_R_EQ, CMP_R_NEQ, // Comparisons
		INC_R, INC_R_1, DEC_R, DEC_R_1, NEG_R, // Unary operations
		IFG_R, // Branches
		R_TO_I, // Casts
		PRINT_R, // Others

		// Raw datas

		RAW_ARRAY,

		// Functions operations

		MK_FUNC, CALL_FUNC, CALL_FUNC_R, CALL_FUNC_V, RET_R, RET, RET_V,

		// Class operations

		MK_CLASS, NEW, NEW_SCOPED,

		// Instance operations

		GET_F, SET_F, SET_F_R, GET_V, GET_I,
		LD_0_GET_F, LD_0_SET_F, LD_0_SET_F_R,
		LD_CALL_V,
		INSTANCEOF,

		// String instances operations

		MK_STRING, PRINT_STR,

		// Array operations

		NEW_ARRAY, MK_ARRAY, GET_AT, SET_AT, SET_AT_R,

		// Tuple operations

		NEW_TUPLE, MK_TUPLE, GET_TE, SET_TE, SET_TE_R,

		// Scope allocations

		INIT_SCOPE, CLEAN_SCOPE,

		// Jump/lookup table

		LU_TABLE, J_TABLE,

		// Exceptions

		MK_EX_TABLE, THROW,

		// Other control flow operations

		GOTO, EXIT,

		// Debug instructions

		MK_DBG_INFO, DBG_EOS, DBG_CALL, DBG_RET, DBG_NEW
	};
}

#endif
