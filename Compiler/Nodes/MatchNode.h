//
//  MatchNode.h
//  TIMScript
//
//  Created by Romain Beguet on 09.04.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__MatchNode__
#define __TIMScript__MatchNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class MatchNode : public AbstractNode {
		public:

			MatchNode(size_t pos, size_t length,
					  AbstractNode* e, const std::vector<TSINT>& c, const std::vector<AbstractNode*>& b, AbstractNode* d);

			virtual ~MatchNode();

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();
		private:

			AbstractNode* _exp;
			std::vector<TSINT> _cases;
			std::vector<AbstractNode*> _blocks;
			AbstractNode* _default;

			// table info

			bool _isJumpTable;
			TSINT min, max;
		};

	}
}

#endif /* defined(__TIMScript__MatchNode__) */
