//
//  InterfaceNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 13.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "InterfaceNode.h"
#include "FunctionNode.h"
#include "Scope.h"
#include "CompilationExceptions.h"
#include "CastUtils.h"

namespace ts {
	namespace nodes {

		InterfaceNode::InterfaceNode(size_t pos, size_t length, const std::string& n, size_t id) : AbstractNode(pos, length), _name(n), _ID(id) {
			_interfaceType = new type::InterfaceType(this);
		}

		InterfaceNode::~InterfaceNode() {

		}

		void InterfaceNode::addDefinitions(const std::map<std::string, cmplr::Definition*>& definitions) {
			for (const auto& def : definitions) {

				addDefinition(def.first, def.second);

			}
		}

		void InterfaceNode::semanticTraverse() {

		}

		void InterfaceNode::pushBytecode(std::vector<TSINSTR>& program) {

		}

		std::string InterfaceNode::toString() {
			return "";
		}

	}
}
