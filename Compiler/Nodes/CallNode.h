//
//  CallNode.h
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__CallNode__
#define __TIMScript__CallNode__


#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class CallNode : public AbstractNode {
		public:

			CallNode(size_t pos, size_t length, AbstractNode* called, std::vector<AbstractNode*> args);
			virtual ~CallNode();

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			void checkArgumentTypes(const std::vector<type::Type*>& types);
			size_t findOverloadedDefinition(AbstractNode* def, bool resetAfter);

			AbstractNode* _called;
			std::vector<AbstractNode*> _args;

			bool alreadyComputed;
		};

	}
}

#endif /* defined(__TIMScript__CallNode__) */
