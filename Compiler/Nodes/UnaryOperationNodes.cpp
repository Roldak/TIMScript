//
//  UnaryOperationNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 04.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "UnaryOperationNodes.h"

namespace ts {
	namespace nodes {

		// UNARY OPERATION

		template<OP_CODE onInt, OP_CODE onReal>
		void UnaryOperation<onInt, onReal>::semanticTraverse() {
			_exp->semanticTraverse();
			_cachedType = _exp->type();
		}

		template<OP_CODE onInt, OP_CODE onReal>
		void UnaryOperation<onInt, onReal>::pushBytecode(std::vector<TSINSTR>& program) {
			_exp->pushBytecode(program);

			if (_cachedType == type::BasicType::Int)
				program.push_back(onInt);
			else if (_cachedType == type::BasicType::Real)
				program.push_back(onReal);
		}

		template<OP_CODE onInt, OP_CODE onReal>
		std::string UnaryOperation<onInt, onReal>::toString() {
			return getUNROPCODElitteral<onInt>() + _exp->toString();
		}

		// UNARY MODIFIER OPERATION

		template<OP_CODE onInt, OP_CODE onReal>
		void UnaryModifierOperation<onInt, onReal>::semanticTraverse() {
			UnaryOperation<onInt, onReal>::_exp->semanticTraverse();
			UnaryOperation<onInt, onReal>::_cachedType = UnaryOperation<onInt, onReal>::_exp->type();
		}

		template<OP_CODE onInt, OP_CODE onReal>
		void UnaryModifierOperation<onInt, onReal>::pushBytecode(std::vector<TSINSTR>& program) {
			if (onInt == INC_I_1)
				((AssignableNode*)UnaryOperation<onInt, onReal>::_exp)->pushPlusPlusAssignementBytecode(program);
			else if (onInt == DEC_I_1)
				((AssignableNode*)UnaryOperation<onInt, onReal>::_exp)->pushMinusMinusAssignementBytecode(program);
		}

		template class UnaryOperation<NEG_I, NEG_R>;
		template class UnaryModifierOperation<INC_I_1, INC_R_1>;
		template class UnaryModifierOperation<DEC_I_1, DEC_R_1>;

	}
}