//
//  StructureNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "StructureNodes.h"
#include "ClassNode.h"
#include "DefinitionReferenceNode.h"
#include "Scope.h"
#include "FunctionNode.h"
#include "ConstNodes.h"
#include "CastUtils.h"
#include "CompilationExceptions.h"
#include "CastNode.h"

namespace ts {
	namespace nodes {

		// MAKE STRUCTURE NODE

		void MakeStructureNode::semanticTraverse() {
			if (_structType->fields().size() != _nodes.size()) {
				throw exception::NumberOfElementsMismatchInitializerList(this, _nodes.size(), (size_t)_structType->fields().size());
			}
			for (size_t i = 0; i < _nodes.size(); ++i) {
				if (_nodes[i]->type() != _structType->fields()[i]->type())
					_nodes[i] = new CastNode(_nodes[i], _structType->fields()[i]->type());
			}
			_cachedType = _structType;
		}

		void MakeStructureNode::pushBytecode(std::vector<TSINSTR>& program) {
			for (size_t i = 0; i < _nodes.size(); ++i)
				_nodes[i]->pushBytecode(program);
		}

		std::string MakeStructureNode::toString() {
			std::string toRet = "[";
			for (size_t i = 0; i < _nodes.size(); ++i) {
				toRet += _nodes[i]->toString();
				if (i != _nodes.size() - 1) {
					toRet += ", ";
				}
			}
			return toRet + "]";
		}

		// STRUCTURE FIELD ACCESS NODE

		StructureFieldAccessNode::~StructureFieldAccessNode() {
			if (_node) {
				if (LocalVariableNode* lv = getIf<LocalVariableNode*>(_node)) {
					delete lv->var();
				}
				delete _node;
			}
		}

		void StructureFieldAccessNode::semanticTraverse() {
			_left->semanticTraverse();
			if (type::StructType* st = type::getIf<type::StructType*>(_left->type())) {
				size_t fieldIndex = st->fieldIndex(_identifier);
				_fieldOffset = st->fieldOffset(_identifier);

				if (fieldIndex == std::string::npos) {
					throw exception::UnknownMember(this, "structure", st->name(), _identifier);
				} else {

					_cachedType = st->fields()[fieldIndex]->type();

					AbstractNode* tmp = _left;

					if (DotOperationNode* dot = getIf<DotOperationNode*>(tmp)) {
						if (StructureFieldAccessNode* sfa = getIf<StructureFieldAccessNode*>(dot->getResultingNode())) {
							tmp = sfa->_node;
						}
					}

					if (LocalVariableNode* lv = getIf<LocalVariableNode*>(tmp)) {
						_node = new LocalVariableNode(pos(), len(), "", new cmplr::Variable(_cachedType, lv->var()->index() + _fieldOffset));
						_node->semanticTraverse();
					}

				}
			}
		}

		void StructureFieldAccessNode::pushBytecode(std::vector<TSINSTR>& program) {
			AbstractNode* tmp = _left;

			if (DotOperationNode* dot = getIf<DotOperationNode*>(tmp)) {
				if (StructureFieldAccessNode* sfa = getIf<StructureFieldAccessNode*>(dot->getResultingNode())) {
					tmp = sfa->_node;
				}
			}

			if (getIf<LocalVariableNode*>(tmp)) {
				_node->pushBytecode(program);
			}
		}

		void StructureFieldAccessNode::pushAssignementBytecode(std::vector<TSINSTR>& program) {
			AbstractNode* tmp = _left;

			if (DotOperationNode* dot = getIf<DotOperationNode*>(tmp)) {
				if (StructureFieldAccessNode* sfa = getIf<StructureFieldAccessNode*>(dot->getResultingNode())) {
					tmp = sfa->_node;
				}
			}

			if (getIf<LocalVariableNode*>(tmp)) {
				_node->pushAssignementBytecode(program);
			}
		}

		void StructureFieldAccessNode::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program) {

		}

		void StructureFieldAccessNode::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program) {

		}

		void StructureFieldAccessNode::setInferedType(type::Type* t) {

		}

		std::string StructureFieldAccessNode::toString() {
			return _left->toString() + "." + _identifier;
		}

	}
}
