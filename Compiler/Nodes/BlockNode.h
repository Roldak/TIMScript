//
//  BlockNode.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__BlockNode__
#define __TIMScript__BlockNode__

#include <iostream>
#include <vector>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class BlockNode : public AbstractNode {
		public:

			BlockNode(size_t pos, size_t length, bool debugMode) : AbstractNode(pos, length), _content(), _generateDebugInstructions(debugMode) {}
			virtual ~BlockNode();

			inline void addNode(AbstractNode* n) {
				_content.push_back(n);
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();
		private:

			std::vector<AbstractNode*> _content;
			bool _generateDebugInstructions;
		};

	}
}

#endif /* defined(__TIMScript__BlockNode__) */
