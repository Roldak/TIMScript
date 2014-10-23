//
//  MatchNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 09.04.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "MatchNode.h"
#include "CastNode.h"
#include "CompilationExceptions.h"

namespace ts {
	namespace nodes {

		MatchNode::MatchNode(size_t pos, size_t length, AbstractNode* e,
							 const std::vector<TSINT>& c, const std::vector<AbstractNode*>& b, AbstractNode* d)
			: AbstractNode(pos, length), _exp(e), _cases(c), _blocks(b), _default(d) {
		}

		MatchNode::~MatchNode() {
			delete _exp;
			for (size_t i = 0; i < _blocks.size(); ++i) {
				delete _blocks[i];
			}
		}

		void MatchNode::semanticTraverse() {
			if (_cases.size() == 0)
				return;

			_exp->semanticTraverse();

			if (_exp->type() != type::BasicType::Int) {
				_exp = new CastNode(_exp, type::BasicType::Int);
				_exp->semanticTraverse();
			}

			_cachedType = NULL;

			for (size_t i = 0; i < _blocks.size(); ++i) {
				_blocks[i]->semanticTraverse();
				if (_cachedType == NULL) {
					_cachedType = _blocks[i]->type();

					if (!_default && _cachedType != type::BasicType::Void)
						throw exception::MissingDefaultCase(this, _cachedType);
				} else if (_blocks[i]->type() != _cachedType) {
					_blocks[i] = new CastNode(_blocks[i], _cachedType);
					try {
						_blocks[i]->semanticTraverse();
					} catch (exception::IllegalCast& ex) {
						throw exception::CasesHaveDifferentTypes(this, ((CastNode*)_blocks[i])->innerExp()->type(), _cachedType);
					}
				}
			}

			if (_default) {
				_default->semanticTraverse();
				if (_default->type() != _cachedType) {
					_default = new CastNode(_default, _cachedType);

					try {
						_default->semanticTraverse();
					} catch (exception::IllegalCast& ex) {
						throw exception::CasesHaveDifferentTypes(this, ((CastNode*)_default)->innerExp()->type(), _cachedType);
					}
				}
			}

			min = max = _cases[0];

			for (size_t i = 1; i < _cases.size(); ++i) {
				if (_cases[i] < min)  min = _cases[i];
				if (_cases[i] > max)  max = _cases[i];
			}

			_isJumpTable = (max - min + 1 == _cases.size());
		}

		void MatchNode::pushBytecode(std::vector<TSINSTR>& program) {
			_exp->pushBytecode(program);

			std::vector<size_t> casesEnds(_cases.size());
			size_t caseBegin;

			if (_isJumpTable) {
				program.push_back(J_TABLE);
				program.push_back((TSINSTR)min);
				program.push_back(0);
				program.push_back(_cases.size());

				caseBegin = program.size();
				program.resize(program.size() + _cases.size());

				for (size_t i = 0; i < _cases.size(); ++i) {
					program[caseBegin + i] = program.size();

					_blocks[i]->pushBytecode(program);

					program.push_back(GOTO);
					casesEnds[i] = program.size();
					program.push_back(0);
				}
			} else {
				program.push_back(LU_TABLE);
				program.push_back((TSINSTR)min);
				program.push_back((TSINSTR)max);
				program.push_back(0); // End of table
				program.push_back(_cases.size());

				caseBegin = program.size();
				program.resize(program.size() + 2 * _cases.size());

				for (size_t i = 0; i < _cases.size(); ++i) {
					program[caseBegin + 2 * i] = (size_t)_cases[i];
					program[caseBegin + 2 * i + 1] = program.size();

					_blocks[i]->pushBytecode(program);

					program.push_back(GOTO);
					casesEnds[i] = program.size();
					program.push_back(0);
				}
			}

			program[caseBegin - 2] = program.size();

			if (_default)
				_default->pushBytecode(program);

			for (size_t i = 0; i < casesEnds.size(); ++i)
				program[casesEnds[i]] = program.size();
		}

		std::string MatchNode::toString() {
			std::string toRet = "match(" + _exp->toString() + "){";
			for (size_t i = 0; i < _cases.size(); ++i)
				toRet += "case " + T_toString(_cases[i]) + "=>" + _blocks[i]->toString() + "\n";
			return toRet + "}";
		}

	}
}