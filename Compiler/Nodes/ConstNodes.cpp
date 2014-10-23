//
//  ConstNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include <sstream>
#include "ConstNodes.h"
#include "ClassNode.h"
#include "InstanceNodes.h"
#include "CallNode.h"

namespace ts {
	namespace nodes {

		// BOOLEAN

		void BooleanNode::semanticTraverse() {
			_cachedType = type::BasicType::Bool;
		}

		void BooleanNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(_value ? PUSH_TRUE : PUSH_FALSE);
		}

		std::string BooleanNode::toString() {
			return T_toString(_value);
		}

		// INTEGER

		void IntegerNode::semanticTraverse() {
			_cachedType = type::BasicType::Int;
		}

		void IntegerNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(PUSH_I);
			program.push_back((TSINSTR)_value);
		}

		std::string IntegerNode::toString() {
			return T_toString(_value);
		}

		// REAL

		void RealNode::semanticTraverse() {
			_cachedType = type::BasicType::Real;
		}

		void RealNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(PUSH_R);
			program.push_back(r_to_instr(_value));
		}

		std::string RealNode::toString() {
			return T_toString(_value);
		}

		// STRING

		StringNode::~StringNode() {
			delete _resultingNode;
		}

		void StringNode::semanticTraverse() {
			if (!_resultingNode) {
				_resultingNode = new NewInstanceNode(_pos, _length, _stringClass, false);
				_resultingNode = new CallNode(_pos, _length, _resultingNode, {new MakeStringNode(_pos, _length, _value)});
				_resultingNode->semanticTraverse();
				_cachedType = _resultingNode->type();
			}
		}

		void StringNode::pushBytecode(std::vector<TSINSTR>& program) {
			_resultingNode->pushBytecode(program);
		}

		std::string StringNode::toString() {
			return "\"" + _value + "\"";
		}

		// MAKE STRING NODE

		void MakeStringNode::semanticTraverse() {
			_cachedType = type::BasicType::Int;
		}

		void MakeStringNode::pushBytecode(std::vector<TSINSTR>& program) {
			program.push_back(MK_STRING);
			program.push_back(_value.size());
			for (size_t i = 0; i < _value.size(); ++i) {
				program.push_back((TSINSTR)_value[i]);
			}
		}

		std::string MakeStringNode::toString() {
			return "\"" + _value + "\"";
		}

		// NULL REFERENCE NODE

		void NullReferenceNode::useAs(type::Type *t) {
			_cachedType = t;
		}

		void NullReferenceNode::semanticTraverse() {
			if (type::BasicType::isBasicType(_cachedType) || type::getIf<type::StructType*>(_cachedType)) {
				// ERROR
			}
		}

		void NullReferenceNode::pushBytecode(std::vector<TSINSTR> &program) {
			program.push_back(PUSH_I);
			program.push_back(0);
		}

		std::string NullReferenceNode::toString() {
			return "null";
		}

	}
}
