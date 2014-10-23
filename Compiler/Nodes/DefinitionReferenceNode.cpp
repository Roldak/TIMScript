//
//  DefinitionReferenceNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 04.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "DefinitionReferenceNode.h"
#include "AssignableNode.h"

namespace ts {
	namespace nodes {

		// DEFINITION REFERENCE NODE

		DefinitionReferenceNode::DefinitionReferenceNode(const std::string& n, cmplr::Definition* r)
			: DefinitionReferenceNode(r->value()->pos(), r->value()->len(), n, r) {

		}

		void DefinitionReferenceNode::semanticTraverse() {
			_cachedType = _def->type();
			if (_cachedType == type::BasicType::NotYetDefined) {
				_def->value()->semanticTraverse();
				_def->setType(_def->value()->type());
				_cachedType = _def->value()->type();
			}
		}

		void DefinitionReferenceNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(LOAD_CST);
			program.push_back(_def->index());
		}

		void DefinitionReferenceNode::pushAssignementBytecode(std::vector<TSINSTR>& program) {
			program.push_back(STORE_N);
			program.push_back(0);
		}

		void DefinitionReferenceNode::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program) {

		}

		void DefinitionReferenceNode::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program) {

		}

		void DefinitionReferenceNode::setInferedType(type::Type* t) {
			_def->setType(t);
			_cachedType = _def->type();
		}

		std::string DefinitionReferenceNode::toString() {
			return _name;
		}

		// NATIVE DEFINITION REFERENCE NODE

		void NativeDefinitionReferenceNode::semanticTraverse() {
			_cachedType = _data._type;
		}

		void NativeDefinitionReferenceNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(LOAD_NAT);
			program.push_back(_data._index);
		}

	}
}