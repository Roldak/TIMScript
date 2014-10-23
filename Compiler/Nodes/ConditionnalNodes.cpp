//
//  ConditionnalNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 31.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ConditionnalNodes.h"
#include "CastNode.h"
#include "BinaryOperationNodes.h"
#include "CompilationExceptions.h"

namespace ts {
	namespace nodes {

		// IF NODE

		IfNode::~IfNode() {
			delete _cond;
			delete _then;
			delete _else;
		}

		void IfNode::semanticTraverse() {
			_cond->semanticTraverse();
			_then->semanticTraverse();

			if (_cond->type() != type::BasicType::Int || _cond->type() != type::BasicType::Bool) {
				_cond = new CastNode(_cond, type::BasicType::Bool);
				_cond->semanticTraverse();
			}

			if (_else) {
				_else->semanticTraverse();
				if (!_then->type()->equals(_else->type())) {
					throw exception::ThenAndElseStatementHaveNotTheSameReturnType(_then, _then->type(), _else->type());
				}
			} else if (_then->type() != type::BasicType::Void)
				throw exception::NonVoidIfWithoutElsePart(_then, _then->type());

			_cachedType = _then->type();
		}

		void IfNode::pushBytecode(std::vector<TSINSTR>& program) {
			size_t toModifyLater_if, toModifyLater_else;

			_cond->pushBytecode(program);
			program.push_back(IF_I_FALSE);
			toModifyLater_if = program.size();
			program.push_back(0);

			_then->pushBytecode(program);

			if (_else) {
				program.push_back(GOTO);
				toModifyLater_else = program.size();
				program.push_back(0);
			}
			program[toModifyLater_if] = program.size();

			if (_else) {
				_else->pushBytecode(program);
				program[toModifyLater_else] = program.size();
			}
		}

		std::string IfNode::toString() {
			if (_else)
				return "if(" + _cond->toString() + ")" + _then->toString() + " else " + _else->toString();
			else
				return "if(" + _cond->toString() + ")" + _then->toString();
		}

		// WHILE NODE

		WhileNode::~WhileNode() {
			delete _cond;
			delete _toLoop;
		}

		void WhileNode::semanticTraverse() {
			_cond->semanticTraverse();
			_toLoop->semanticTraverse();

			if (_cond->type() != type::BasicType::Int && _cond->type() != type::BasicType::Bool)
				_cond = new CastNode( _cond, type::BasicType::Bool);

			if (_toLoop->type() != type::BasicType::Void)
				_toLoop = new CastNode(_toLoop, type::BasicType::Void);

			_cachedType = type::BasicType::Void;
		}

		void WhileNode::pushBytecode(std::vector<TSINSTR>& program) {
			size_t before, after;
			before = program.size();

			_cond->pushBytecode(program);
			program.push_back(IF_I_FALSE);
			after = program.size();
			program.push_back(0);

			_toLoop->pushBytecode(program);
			program.push_back(GOTO);
			program.push_back(before);

			program[after] = program.size();
		}

		std::string WhileNode::toString() {
			return "while(" + _cond->toString() + ")" + _toLoop->toString();
		}

		// FOR NODE

		ForNode::~ForNode() {
			delete _init;
			delete _cond;
			delete _incr;
			delete _toLoop;
		}

		void ForNode::semanticTraverse() {
			_init->semanticTraverse();
			_cond->semanticTraverse();
			_incr->semanticTraverse();
			_toLoop->semanticTraverse();

			if (_init->type() != type::BasicType::Void)
				_init = new CastNode(_init, type::BasicType::Void);

			if (_cond->type() != type::BasicType::Int || _cond->type() != type::BasicType::Bool)
				_cond = new CastNode(_cond, type::BasicType::Bool);

			if (_incr->type() != type::BasicType::Void)
				_incr = new CastNode(_incr, type::BasicType::Void);

			if (_toLoop->type() != type::BasicType::Void)
				_toLoop = new CastNode(_toLoop, type::BasicType::Void);

			_cachedType = type::BasicType::Void;
		}

		void ForNode::pushBytecode(std::vector<TSINSTR>& program) {
			size_t before, after;

			_init->pushBytecode(program);

			before = program.size();

			_cond->pushBytecode(program);
			program.push_back(IF_I_FALSE);
			after = program.size();
			program.push_back(0);

			_toLoop->pushBytecode(program);
			_incr->pushBytecode(program);
			program.push_back(GOTO);
			program.push_back(before);

			program[after] = program.size();
		}

		std::string ForNode::toString() {
			return "for(" + _init->toString() + "; " + _cond->toString() + "; " + _incr->toString() + ")" + _toLoop->toString();
		}

	}
}
