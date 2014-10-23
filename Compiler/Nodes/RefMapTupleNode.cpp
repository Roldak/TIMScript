//
//  RefMapTupleNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "RefMapTupleNode.h"
#include "Scope.h"

namespace ts {
	namespace nodes {

		void RefMapTupleNode::semanticTraverse() {}

		void RefMapTupleNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(RAW_ARRAY);
			program.push_back(_vars.size());

			size_t number = 0;

			for (size_t i = 0; i < _vars.size(); ++i) {
				if (!type::BasicType::isBasicType(_vars[i].second->type()))
					program.push_back(number++);
				else
					program.push_back(std::string::npos);
			}
		}

		std::string RefMapTupleNode::toString() {
			return "";
		}

		void TupleTypeRefMapTupleNode::semanticTraverse() {}

		void TupleTypeRefMapTupleNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(RAW_ARRAY);
			program.push_back(_types.size());

			size_t number = 0;

			for (size_t i = 0; i < _types.size(); ++i) {
				if (!type::BasicType::isBasicType(_types[i]))
					program.push_back(number++);
				else
					program.push_back(std::string::npos);
			}
		}

		std::string TupleTypeRefMapTupleNode::toString() {
			return "";
		}

	}
}
