//
//  AsyncNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 01.04.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "AsyncNode.h"
#include "DotOperation.h"
#include "CallNode.h"

namespace ts {
	namespace nodes {

		void AsyncNode::semanticTraverse() {
			if (_finalNode)
				return;

			_function->semanticTraverse();
			if (type::getIf<type::FunctionType*>(_function->type())->returnType() == type::BasicType::Void)
				_finalNode = new DotOperationNode(_pos, _length, _threadClassReference, "_async_void");
			else
				_finalNode = new DotOperationNode(_pos, _length, _threadClassReference, "_async");

			_finalNode = new CallNode(_pos, _length, _finalNode, {_function});
			_finalNode->semanticTraverse();
			_cachedType = _finalNode->type();
		}

		void AsyncNode::pushBytecode(std::vector<TSINSTR>& program) {
			if (_finalNode) {
				_finalNode->pushBytecode(program);
			}
		}

		std::string AsyncNode::toString() {
			return "async+" + _function->toString();
		}

	}
}