//
//  AssignableNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "AssignableNode.h"
#include "VariableNode.h"
#include "CastNode.h"
#include "CompilationExceptions.h"

namespace ts {
	namespace nodes {

		AssignementNode::AssignementNode(size_t pos, size_t length, AssignableNode* an, AbstractNode* v)
			: AbstractNode(pos, length), _assignedNode(an), _value(v) {}

		AssignementNode::~AssignementNode() {
			delete _assignedNode;
			delete _value;
		}

		void AssignementNode::semanticTraverse() {

			_assignedNode->semanticTraverse();
			_value->semanticTraverse();

			type::Type* assignedNodeType = _assignedNode->type();
			type::Type* valueType = _value->type();

			if (assignedNodeType == type::BasicType::NotYetDefined) {
				if (valueType == type::BasicType::Any) {
					throw exception::UndeducableType(_assignedNode);
				}

				if (type::getIf<type::StructType*>(valueType)) {
					throw exception::UnsetStructureType(_assignedNode, valueType);
				}

				_assignedNode->setInferedType(valueType); // Type inference
			} else if (!assignedNodeType->equals(valueType)) {
				_value = new CastNode(_value, assignedNodeType);
				_value->semanticTraverse();
			}

			_cachedType = type::BasicType::Void;
		}

		void AssignementNode::pushBytecode(std::vector<TSINSTR>& program) {
			_value->pushBytecode(program);
			_assignedNode->pushAssignementBytecode(program);
		}

		std::string AssignementNode::toString() {
			return _assignedNode->toString() + "=" + _value->toString();
		}
	}
}
