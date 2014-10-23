//
//  PrintNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "PrintNode.h"
#include "ClassNode.h"
#include "DotOperation.h"
#include "CallNode.h"

namespace ts {
	namespace nodes {

		void PrintNode::semanticTraverse() {
			for (AbstractNode*& n : _nodes) {
				n->semanticTraverse();

				type::Type* t = n->type();

				if (type::getIf<type::ClassType*>(t) || type::getIf<type::ComposedType*>(t)) {
					n = new DotOperationNode(_pos, _length, n, "toString");
					n = new CallNode(_pos, _length, n, {});
					n->semanticTraverse();
				}
			}

			_cachedType = type::BasicType::Void;
		}

		void PrintNode::pushBytecode(std::vector<TSINSTR>& program) {
			for (AbstractNode*& n : _nodes) {
				n->pushBytecode(program);

				type::Type* t = n->type();

				if (t == type::BasicType::Bool)
					program.push_back(PRINT_B);
				else if (t == type::BasicType::Int)
					program.push_back(PRINT_I);
				else if (t == type::BasicType::Real)
					program.push_back(PRINT_R);
				else if (type::getIf<type::ClassType*>(t)) {
					program.push_back(PRINT_STR);
				}
			}
		}

		std::string PrintNode::toString() {
			std::string toRet = "print " + _nodes[0]->toString();
			for (size_t i = 1; i < _nodes.size(); ++i) {
				toRet += ", " + _nodes[i]->toString();
			}
			return toRet;
		}

	}
}
