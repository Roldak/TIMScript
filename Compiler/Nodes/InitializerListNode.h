//
//  InitializerListNode.h
//  TIMScript
//
//  Created by Romain Beguet on 04.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__InitializerListNode__
#define __TIMScript__InitializerListNode__

#include <iostream>
#include <vector>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class InitializerListNode : public AbstractNode {
		public:

			InitializerListNode(size_t pos, size_t length, std::vector<AbstractNode*>& nodes);

			virtual ~InitializerListNode();

			virtual void semanticTraverse();
			void useAs(type::Type* t);

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_INITIALIZERLIST;
			}

		private:

			std::vector<AbstractNode*> _nodes;
			AbstractNode* _resultingNode;
		};

	}
}

#endif /* defined(__TIMScript__InitializerListNode__) */
