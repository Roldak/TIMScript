//
//  CallNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "CallNode.h"
#include "CastNode.h"
#include "InstanceNodes.h"
#include "ArrayNodes.h"
#include "DotOperation.h"
#include "DefinitionReferenceNode.h"
#include "Scope.h"
#include "CastUtils.h"
#include "CompilationExceptions.h"
#include "CompilationContext.h"

namespace ts {
	namespace nodes {

		void CallNode::checkArgumentTypes(const std::vector<type::Type*>& types) {

			if (_args.size() != types.size()) {
				throw exception::ArgumentNumberMismatch(this, _args.size(), types.size());
			}

			for (size_t i = 0; i < types.size(); ++i) {
				if (!_args[i]->type()->equals(types[i])) {
					_args[i] = new CastNode(_args[i], types[i]);
					try {
						_args[i]->semanticTraverse();
					} catch (const exception::IllegalCast& ex) {
						throw exception::ArgumentTypeMismatch(_args[i], i, ((CastNode*)_args[i])->innerExp()->type(), types[i]);
					}
				}
			}
		}

		size_t CallNode::findOverloadedDefinition(AbstractNode* def, bool resetAfter) {
			DefinitionReferenceNode* ref = (DefinitionReferenceNode*)def;
			cmplr::Definition* firstDef = ref->definition();
			size_t count = 0;

			do {
				// try the current definition

				type::FunctionType* func = (type::FunctionType*)ref->definitionType();
				const std::vector<type::Type*>& argTypes(func->argsTypes());

				if (_args.size() == argTypes.size()) {
					bool ok = true;

					for (size_t i = 0; i < argTypes.size(); ++i) {
						if (!_args[i]->type()->equals(argTypes[i])) {
							if (!CastNode::isCastable(_args[i]->type(), argTypes[i])) {
								ok = false;
								break;
							}
						}
					}

					if (ok) {
						if (resetAfter)
							ref->setDefinition(firstDef);
						return count;
					}
				}

				// it didnt work, try the next
				ref->nextOverloadedDefinition();
				++count;
			} while (ref->definition());

			ref->setDefinition(firstDef);
			return 0;
		}

		CallNode::CallNode(size_t pos, size_t length, AbstractNode* called, std::vector<AbstractNode*> args)
			: AbstractNode(pos, length), _called(called), _args(args), alreadyComputed(false) {
			_cachedType = NULL;
		}

		CallNode::~CallNode() {
			size_t i = 0;

			if (DotOperationNode* dop = getIf<DotOperationNode*>(_called)) {
				if (    getIf<VirtualAccessNode*>(dop->getResultingNode()) ||
						getIf<ImplementationAccessNode*>(dop->getResultingNode())) {
					i = 1;
				}
			}

			for (; i < _args.size(); ++i)
				delete _args[i];
			delete _called;
		}

		void CallNode::semanticTraverse() {
			if (alreadyComputed)
				return;

			_called->semanticTraverse();
			for (size_t i = 0; i < _args.size(); ++i) {
				_args[i]->semanticTraverse();
			}

			type::Type* t = _called->type();

			if (_cachedType != NULL)
				return;

			if (NewInstanceNode* n = getIf<NewInstanceNode*>(_called)) {
				n->getConstructor()->semanticTraverse();
				type::FunctionType* func = (type::FunctionType*)n->getConstructor()->type();
				const std::vector<type::Type*>& argsTypes(func->argsTypes());

				_args.insert(_args.begin(), new MakeObject(n->pos(), n->len(), n->getClass(), n->scoped()));

				checkArgumentTypes(argsTypes);

				_cachedType = func->returnType();
			} else if (NewArrayNode* an = getIf<NewArrayNode*>(_called)) {
				type::ArrayType* art = (type::ArrayType*)an->type();
				if (art->nbElements() != -1) {
					std::cerr << "Error" << std::endl;
				} else if (_args[0]->type() != type::BasicType::Int) {
					_args[0] = new CastNode(_args[0], type::BasicType::Int);
					_args[0]->semanticTraverse();
				}
				_cachedType = art;
			} else if (NewTupleNode* tn = getIf<NewTupleNode*>(_called)) {
				_cachedType = tn->type();
			} else {
				if (DotOperationNode* dop = getIf<DotOperationNode*>(_called)) {
					if (VirtualAccessNode* va = getIf<VirtualAccessNode*>(dop->getResultingNode())) {
						_args.insert(_args.begin(), dop->left());
						size_t nb = findOverloadedDefinition(va->definition(), true);
						va->setDefinitionIndex(va->definitionIndex() + nb);
						va->setIsForFunctionCall(true);
						t = va->type();
					} else if (ImplementationAccessNode* ia = getIf<ImplementationAccessNode*>(dop->getResultingNode())) {
						_args.insert(_args.begin(), dop->left());
						ia->setIsForFunctionCall(true);
						t = ia->type();
					}
				} else if (DefinitionReferenceNode* dt = getIf<DefinitionReferenceNode*>(_called)) {
					if (dt->definitionType() == type::BasicType::Class) {
						_called = new NewInstanceNode(_pos, _length, (ClassNode*)dt->definition()->value(), true);
						semanticTraverse();
					}
					findOverloadedDefinition(dt, false);
					t = dt->definitionType();
				}

				if (type::FunctionType* func = getIf<type::FunctionType*>(t)) {
					const std::vector<type::Type*>& argsTypes(func->argsTypes());

					checkArgumentTypes(argsTypes);

					_cachedType = func->returnType();
				}
			}

			alreadyComputed = true;
		}

		void CallNode::pushBytecode(std::vector<TSINSTR>& program) {
			for (size_t i = 0; i < _args.size(); ++i) {
				_args[i]->pushBytecode(program);
			}

			_called->pushBytecode(program);

			if (type::FunctionType* ft = type::getIf<type::FunctionType*>(_called->type())) {

				if (cmplr::CompilationContext::inDebugMode()) {
					program.push_back(DBG_CALL);
					program.push_back(0);
				}

				if (ft->returnType() == type::BasicType::Void)
					program.push_back(CALL_FUNC_V);
				else if (type::BasicType::isBasicType(ft->returnType()))
					program.push_back(CALL_FUNC);
				else
					program.push_back(CALL_FUNC_R);

				const std::vector<type::Type*>& flattenArgs = ft->flattenTypes();

				program.push_back(flattenArgs.size());

				size_t count = 0;
				size_t loc = program.size();
				program.push_back(0);

				for (size_t i = 0; i < flattenArgs.size(); ++i) {
					if (!type::BasicType::isBasicType(flattenArgs[i])) {
						++count;
						program.push_back(i);
					}
				}
				program[loc] = count;

				if (cmplr::CompilationContext::inDebugMode()) {
					program.push_back(DBG_RET);
					program.push_back(DBG_EOS);
					program.push_back(pos());
				}
			}
		}

		std::string CallNode::toString() {
			std::string toRet = _called->toString() + "(";
			for (size_t i = 0; i < _args.size(); ++i) {
				toRet += _args[i]->toString();
				if (i != _args.size() - 1)
					toRet += ", ";
			}
			return toRet + ")";
		}

	}
}
