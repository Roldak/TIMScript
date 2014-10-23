//
//  ConditionnalNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 31.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ConditionnalNodes__
#define __TIMScript__ConditionnalNodes__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class IfNode : public AbstractNode {
		public:

			IfNode(size_t pos, size_t length, AbstractNode* c, AbstractNode* t, AbstractNode* e)
				: AbstractNode(pos, length), _cond(c), _then(t), _else(e) {}

			virtual ~IfNode();

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			AbstractNode* _cond;
			AbstractNode* _then;
			AbstractNode* _else;

		};

		class WhileNode : public AbstractNode {
		public:

			WhileNode(size_t pos, size_t length, AbstractNode* c, AbstractNode* t)
				: AbstractNode(pos, length), _cond(c), _toLoop(t) {}

			virtual ~WhileNode();

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			AbstractNode* _cond;
			AbstractNode* _toLoop;

		};

		class ForNode : public AbstractNode {
		public:

			ForNode(size_t pos, size_t length, AbstractNode* init, AbstractNode* cond, AbstractNode* incr, AbstractNode* t)
				: AbstractNode(pos, length), _init(init), _cond(cond), _incr(incr), _toLoop(t) {}

			virtual ~ForNode();

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			AbstractNode* _init;
			AbstractNode* _cond;
			AbstractNode* _incr;
			AbstractNode* _toLoop;

		};

	}
}


#endif /* defined(__TIMScript__ConditionnalNodes__) */
