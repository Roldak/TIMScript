//
//  AssignableNode.h
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__AssignableNode__
#define __TIMScript__AssignableNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class AssignableNode : public AbstractNode {
		public:
			virtual void pushAssignementBytecode(std::vector<TSINSTR>& program) = 0;
			virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program) = 0;
			virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program) = 0;
			virtual void setInferedType(type::Type* t) = 0;

		protected:
			AssignableNode(size_t pos, size_t length) : AbstractNode(pos, length) {}
		};

		class AssignementNode : public AbstractNode {
		public:

			AssignementNode(size_t pos, size_t length, AssignableNode* an, AbstractNode* v);
			virtual ~AssignementNode();

			inline AbstractNode* value() {
				return _value;
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_ASSIGNABLE;
			}

		private:

			AssignableNode* _assignedNode;
			AbstractNode* _value;

		};

	}
}

#endif /* defined(__TIMScript__AssignableNode__) */
