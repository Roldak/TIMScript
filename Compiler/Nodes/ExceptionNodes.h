//
//  ExceptionNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 27.05.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ExceptionNodes__
#define __TIMScript__ExceptionNodes__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace cmplr {
		class Variable;
	}

	namespace nodes {

		class ThrowNode : public AbstractNode {
		public:

			ThrowNode(size_t pos, size_t length, AbstractNode* node) : AbstractNode(pos, length), _content(node) {}
			virtual ~ThrowNode() {
				delete _content;
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();
		private:

			AbstractNode* _content;
		};

		class TryCatchNode : public AbstractNode {
		public:

			TryCatchNode(size_t pos, size_t length, AbstractNode* tryNode);
			virtual ~TryCatchNode();

			inline void addCatchNode(cmplr::Variable* exType, AbstractNode* catchNode) {
				_catchNodes.push_back(std::pair<cmplr::Variable*, AbstractNode*>(exType, catchNode));
			}

			inline const std::vector<size_t>& getCatchEntryPoints() {
				return _catchEntryPoints;
			}
			inline size_t getTryBlockIPStart() {
				return _beginTryNodeIP;
			}
			inline size_t getTryBlockIPEnd() {
				return _endTryNodeIP;
			}

			inline std::vector<std::pair<cmplr::Variable*, AbstractNode*>>& getCatchNodes() {
				return _catchNodes;
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			AbstractNode* _tryNode;
			std::vector<std::pair<cmplr::Variable*, AbstractNode*>> _catchNodes;

			std::vector<size_t> _catchEntryPoints;
			size_t _beginTryNodeIP, _endTryNodeIP;
		};

		class ExTableNode : public AbstractNode {
		public:

			ExTableNode(size_t pos, size_t length) : AbstractNode(pos, length) {}
			virtual ~ExTableNode();

			inline void addTryCatchNode(TryCatchNode* tryCatchNode) {
				_tryCatchNodes.push_back(tryCatchNode);
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();
		private:

			std::vector<TryCatchNode*> _tryCatchNodes;

		};
	}
}

#endif /* defined(__TIMScript__ExceptionNodes__) */
