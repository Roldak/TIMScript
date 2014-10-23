//
//  DotOperation.h
//  TIMScript
//
//  Created by Romain Beguet on 13.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__DotOperation__
#define __TIMScript__DotOperation__

#include <iostream>
#include "AssignableNode.h"

namespace ts {
	namespace nodes {

		class DotOperationNode : public AssignableNode {
		public:

			DotOperationNode(size_t pos, size_t length, AbstractNode* l, std::string i)
				: AssignableNode(pos, length), _left(l), _identifier(i) {}

			virtual ~DotOperationNode();

			inline AbstractNode* left() {
				return _left;
			}
			inline AbstractNode* getResultingNode() {
				return _node;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void setInferedType(type::Type* t);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_DOT_OPERATION;
			}

		protected:

			void findAppropriateAccessNode(type::ComposedType* ct);

			AbstractNode* _left;
			AbstractNode* _node;
			const std::string _identifier;
		};

	}
}

#endif /* defined(__TIMScript__DotOperation__) */
