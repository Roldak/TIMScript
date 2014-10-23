//
//  DotOperation.cpp
//  TIMScript
//
//  Created by Romain Beguet on 13.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "DotOperation.h"
#include "ClassNode.h"
#include "FunctionNode.h"
#include "DefinitionReferenceNode.h"
#include "Scope.h"
#include "InstanceNodes.h"
#include "StructureNodes.h"
#include "CastUtils.h"
#include "CompilationExceptions.h"

namespace ts {
	namespace nodes {

		DotOperationNode::~DotOperationNode() {
			delete _left;
			if (_node)
				delete _node;
		}

		void DotOperationNode::findAppropriateAccessNode(type::ComposedType* ct) {
			if (type::ClassType* cls = ct->getClass()) {
				if (cls->getClass()->getAttributeIndex(_identifier) != std::string::npos) {
					_node = new AttributeAccessNode(_left->pos(), _left->len(), _left, _identifier);
					return;
				} else if (cls->getClass()->getDefinitionIndex(_identifier) != std::string::npos) {
					_node = new VirtualAccessNode(_left->pos(), _left->len(), _left, _identifier);
					return;
				}
			}
			_node = new ImplementationAccessNode(_left->pos(), _left->len(), _left, _identifier);
		}

		void DotOperationNode::semanticTraverse() {
			_left->semanticTraverse();
			type::Type* t = _left->type();

			if (t == type::BasicType::Class) {
				ClassNode* clss;

				/*
				 Refering to a class can only happen through a direct reference by a DefinitionReferenceNode,
				 or by a DotOperationNode, for example (Class.InnerClass).method()
				 */

				if (DefinitionReferenceNode* defref = getIf<DefinitionReferenceNode*>(_left)) {
					clss = ((ClassNode*)defref->definition()->value());
				} else {
					clss = ((ClassNode*)((DefinitionReferenceNode*)((DotOperationNode*)_left)->getResultingNode())->definition()->value());
				}

				size_t index = clss->getDefinitionIndex(_identifier);
				if (index != std::string::npos) {
					_node = clss->defs()[index];
				} else {
					throw exception::UnknownStaticMember(this, clss->name(), _identifier);
				}
			} else if (type::ClassType* ct = type::getIf<type::ClassType*>(t)) {
				if (ct->getClass()->getAttributeIndex(_identifier) == std::string::npos)
					_node = new VirtualAccessNode(_left->pos(), _left->len(), _left, _identifier);
				else
					_node = new AttributeAccessNode(_left->pos(), _left->len(), _left, _identifier);
			} else if (type::getIf<type::InterfaceType*>(t)) {
				_node = new ImplementationAccessNode(_left->pos(), _left->len(), _left, _identifier);
			} else if (type::getIf<type::TupleType*>(t)) {
				_node = new TupleElementAccessNode(_left->pos(), _left->len(), _left, _identifier);
			} else if (type::ComposedType* ct = type::getIf<type::ComposedType*>(t)) {
				findAppropriateAccessNode(ct);
			} else if (type::getIf<type::StructType*>(t)) {
				_node = new StructureFieldAccessNode(_left->pos(), _left->len(), _left, _identifier);
			} else if (type::getIf<type::ArrayType*>(t)) {

			} else
				throw exception::InvalidDotOperation(this, t, _identifier);

			_node->semanticTraverse();
			_cachedType = _node->type();
		}

		void DotOperationNode::pushBytecode(std::vector<TSINSTR>& program) {
			_node->pushBytecode(program);
		}

		void DotOperationNode::pushAssignementBytecode(std::vector<TSINSTR>& program) {
			if (AssignableNode* an = getIf<AssignableNode*>(_node)) {
				an->pushAssignementBytecode(program);
			}
		}

		void DotOperationNode::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program) {
			if (AssignableNode* an = getIf<AssignableNode*>(_node)) {
				an->pushPlusPlusAssignementBytecode(program);
			}
		}

		void DotOperationNode::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program) {
			if (AssignableNode* an = getIf<AssignableNode*>(_node)) {
				an->pushMinusMinusAssignementBytecode(program);
			}
		}

		void DotOperationNode::setInferedType(type::Type* t) {
			if (AssignableNode* an = getIf<AssignableNode*>(_node)) {
				an->setInferedType(t);
			}
		}

		std::string DotOperationNode::toString() {
			return "(" + _left->toString() + "." + _identifier + ")";
		}

	}
}
