//
//  DebugInfoNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "DebugInfoNode.h"
#include "Scope.h"
#include "FunctionNode.h"

namespace ts {
	namespace nodes {

		DEBUG_TYPE debugTypeFromVarType(type::Type* t) {
			if (t == type::BasicType::Bool)
				return T_BOOL;
			else if (t == type::BasicType::Int)
				return T_INT;
			else if (t == type::BasicType::Real)
				return T_REAL;
			else
				return T_REF;
		}

		FunctionDebugInfoNode::FunctionDebugInfoNode(const std::vector<std::pair<std::string, cmplr::Variable*>>& vars)
			: AbstractNode(0, 0),  _vars(vars) {

		}

		void FunctionDebugInfoNode::semanticTraverse() {}

		void FunctionDebugInfoNode::pushBytecode(std::vector<TSINSTR>& program) {

			for (size_t i = 0; i < _vars.size(); ++i) {

				// push variable's index

				program.push_back(PUSH_I);
				program.push_back(_vars[i].second->index());

				// push variable's name

				program.push_back(MK_STRING);

				std::string name = _vars[i].first;

				program.push_back(name.size());

				for (size_t j = 0; j < name.size(); ++j) {
					program.push_back(name[j]);
				}

				// push variable's debug type

				program.push_back(PUSH_I);
				program.push_back(debugTypeFromVarType(_vars[i].second->type()));
			}

			program.push_back(MK_DBG_INFO);
			program.push_back(_vars.size());
		}

		std::string FunctionDebugInfoNode::toString() {
			return "";
		}

		// CLASS DEBUG INFOS

		ClassDebugInfoNode::ClassDebugInfoNode(ClassNode* clss) : AbstractNode(0, 0), _class(clss) {

		}

		void ClassDebugInfoNode::semanticTraverse() {}

		void ClassDebugInfoNode::pushBytecode(std::vector<TSINSTR> &program) {

			const std::vector<Argument*>& attrs(_class->getAttributes());

			for (size_t i = 0; i < attrs.size(); ++i) {

				// push variable's index

				program.push_back(PUSH_I);
				program.push_back(i);

				// push variable's name

				program.push_back(MK_STRING);

				std::string name = attrs[i]->name();

				program.push_back(name.size());

				for (size_t j = 0; j < name.size(); ++j) {
					program.push_back(name[j]);
				}

				// push variable's debug type

				program.push_back(PUSH_I);
				program.push_back(debugTypeFromVarType(attrs[i]->type()));
			}

			program.push_back(MK_DBG_INFO);
			program.push_back(attrs.size());
		}

		std::string ClassDebugInfoNode::toString() {
			return "";
		}


	}
}
