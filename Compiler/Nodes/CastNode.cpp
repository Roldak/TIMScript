//
//  CastNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 02.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "CastNode.h"
#include "InitializerListNode.h"
#include "CastUtils.h"
#include "CompilationExceptions.h"

namespace ts {
	namespace nodes {

		CastNode::CastNode(size_t pos, size_t length, AbstractNode* e, type::Type* t)
			: AbstractNode(pos, length), _type(t), _exp(e) {
			_cachedType = _type;
			if (InitializerListNode* list = getIf<InitializerListNode*>(_exp)) {
				list->useAs(_type);
			}
		}

		CastNode::CastNode(AbstractNode* e, type::Type* t) : CastNode(e->pos(), e->len(), e, t) {

		}

		CastNode::~CastNode() {
			delete _exp;
		}

		void CastNode::semanticTraverse() {
			_exp->semanticTraverse();
			type::Type* expType = _exp->type();
			if (!isCastable(expType, _type))
				throw exception::IllegalCast(this, expType, _type);
		}

		void CastNode::pushBytecode(std::vector<TSINSTR>& program) {
			_exp->pushBytecode(program);

			type::Type* expType = _exp->type();
			if (expType->equals(_type))
				return;
			else if (expType == type::BasicType::Real && (_type == type::BasicType::Int || _type == type::BasicType::Bool))
				program.push_back(R_TO_I);
			else if ((expType == type::BasicType::Int || expType == type::BasicType::Bool) && _type == type::BasicType::Real)
				program.push_back(I_TO_R);
			else if (_type == type::BasicType::Void) {
				if (type::BasicType::isBasicType(expType))
					program.push_back(POP);
				else
					program.push_back(POP_R);
			}
		}

		std::string CastNode::toString() {
			return "((" + _type->toString() + ")" + _exp->toString() + ")";
		}

		bool CastNode::isCastable(type::Type* from, type::Type* into) {

			if (from->equals(into))
				return true;
			else if (from == type::BasicType::Bool && into == type::BasicType::Int)
				return true;
			else if (from == type::BasicType::Int && into == type::BasicType::Bool)
				return true;
			else if (from == type::BasicType::Real && (into == type::BasicType::Int || into == type::BasicType::Bool))
				return true;
			else if ((from == type::BasicType::Int || from == type::BasicType::Bool) && into == type::BasicType::Real)
				return true;
			else if (into == type::BasicType::Void || into == type::BasicType::Any || from == type::BasicType::Any)
				return true;
			else if (type::ClassType* currentType = type::getIf<type::ClassType*>(from))
				return currentType->inheritsFrom(into);
			else if (type::InterfaceType* currentType = type::getIf<type::InterfaceType*>(from))
				return currentType->inheritsFrom(into);
			else if (type::ComposedType* currentType = type::getIf<type::ComposedType*>(from))
				return currentType->inheritsFrom(into);
			else if (type::FunctionType* fta = type::getIf<type::FunctionType*>(from)) {
				if (type::FunctionType* ftb = type::getIf<type::FunctionType*>(into)) {
					return isFunctionCastable(fta, ftb);
				}
			}

			return false;
		}

		bool CastNode::isFunctionCastable(type::FunctionType *fta, type::FunctionType *ftb) {
			if (fta->argsTypes().size() != ftb->argsTypes().size())
				return false;
			else {
				const std::vector<type::Type*>& typesA(fta->argsTypes());
				const std::vector<type::Type*>& typesB(ftb->argsTypes());

				for (size_t i = 0; i < typesA.size(); ++i) {
					if (!typesA[i]->equals(typesB[i])) {
						return false;
					}
				}

				if (type::ClassType* cta = type::getIf<type::ClassType*>(fta->returnType())) {
					if (type::ClassType* ctb = type::getIf<type::ClassType*>(ftb->returnType()))
						return cta->inheritsFrom(ctb);
				}

				return fta->returnType()->equals(ftb->returnType());
			}
		}

		// HARD CAST NODE

		HardCastNode::HardCastNode(size_t pos, size_t length, AbstractNode* e, type::Type* t) : CastNode(pos, length, e, t) {

		}

		HardCastNode::HardCastNode(AbstractNode* e, type::Type* t) : CastNode(e, t) {

		}

		void HardCastNode::semanticTraverse() {
			try {
				CastNode::semanticTraverse();
			} catch (exception::IllegalCast& ex) {
				if (type::ClassType* wantedType = type::getIf<type::ClassType*>(_type)) {
					if (type::ClassType* currentType = type::getIf<type::ClassType*>(_exp->type())) {
						if (wantedType->inheritsFrom(currentType))
							return;
					}
				}
				throw ex;
			}
		}
	}
}
