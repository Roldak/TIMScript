//
//  Procedure.cpp
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Procedure.h"

namespace ts {

	Procedure::Procedure(const std::string& name)
		:   _stack_size(0), _local_var_nb(0), _bytecode(NULL), _bytecode_length(0), _refsLocations(NULL), _exTable(NULL), _name(name) {

	}

	Procedure::Procedure(const size_t st_sz,
						 const size_t lcl_sz,
						 const TSINSTR* bytecode,
						 const size_t bytecodeLength,
						 size_t_Array** refsLoc,
						 ExceptionTable** exTable,
						 DebugInfos** dbgInfo,
						 const std::string& name)

		:
		_stack_size(st_sz),
		_local_var_nb(lcl_sz),
		_bytecode(bytecode), _bytecode_length(bytecodeLength),
		_refsLocations(refsLoc),
		_exTable(exTable),
		_dbgInfo(dbgInfo),
		_name(name) {

	}

}
