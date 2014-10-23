//
//  BeginNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "BeginNode.h"

namespace ts {
	namespace nodes {

		void BeginNode::semanticTraverse() {
			_content->semanticTraverse();
			_cachedType = type::BasicType::Void;
		}

		void BeginNode::pushBytecode(std::vector<TSINSTR>& program) {
			_content->pushBytecode(program);
			program.push_back(RET_V);
		}

		std::string BeginNode::toString() {
			return "begin" + _content->toString();
		}

	}
}