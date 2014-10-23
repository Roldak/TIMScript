//
//  InitializerListNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 04.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "InitializerListNode.h"
#include "ArrayNodes.h"
#include "TupleNodes.h"
#include "StructureNodes.h"
#include "ClassNode.h"

namespace ts {
	namespace nodes {

		InitializerListNode::InitializerListNode(size_t pos, size_t length, std::vector<AbstractNode*>& nodes)
			: AbstractNode(pos, length), _nodes(nodes), _resultingNode(NULL) {
			_cachedType = type::BasicType::Any;
		}

		InitializerListNode::~InitializerListNode() {
			for (size_t i = 0; i < _nodes.size(); ++i)
				delete _nodes[i];

			if (_resultingNode)
				delete _resultingNode;
		}

		void InitializerListNode::semanticTraverse() {
			for (size_t i = 0; i < _nodes.size(); ++i)
				_nodes[i]->semanticTraverse();
		}

		void InitializerListNode::useAs(type::Type* t) {
			if (_resultingNode)
				return;

			if (type::ArrayType* at = type::getIf<type::ArrayType*>(t)) {
				_resultingNode = new MakeArrayNode(_pos, _length, _nodes, at);
				_resultingNode->semanticTraverse();
			} else if (type::TupleType* tt = type::getIf<type::TupleType*>(t)) {
				_resultingNode = new MakeTupleNode(_pos, _length, _nodes, tt);
				_resultingNode->semanticTraverse();
			} else if (type::StructType* st = type::getIf<type::StructType*>(t)) {
				_resultingNode = new MakeStructureNode(_pos, _length, _nodes, st);
				_resultingNode->semanticTraverse();
			} else if (type::ClassType* ct = type::getIf<type::ClassType*>(t)) {
				std::cerr << ct->getClass()->name() << std::endl;
			}
			_cachedType = t;
		}

		void InitializerListNode::pushBytecode(std::vector<TSINSTR>& program) {
			if (_resultingNode)
				_resultingNode->pushBytecode(program);
		}

		std::string InitializerListNode::toString() {
			std::string toRet = "[";
			for (size_t i = 0; i < _nodes.size(); ++i) {
				toRet += _nodes[i]->toString();
				if (i != _nodes.size() - 1) {
					toRet += ", ";
				}
			}
			return toRet + "]";
		}

	}
}
