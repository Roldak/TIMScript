//
//  PassNode.h
//  TIMScript
//
//  Created by Romain Beguet on 04.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__PassNode__
#define __TIMScript__PassNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class PassNode : public AbstractNode {
		public:

			PassNode(size_t pos, size_t length) : AbstractNode(pos, length) {}
			virtual ~PassNode() {}

			virtual void semanticTraverse() {
				_cachedType = type::BasicType::Void;
			}

			virtual void pushBytecode(std::vector<TSINSTR>& program) {}
			virtual std::string toString() {
				return "pass";
			}

			virtual NODE_TYPE getNodeType() {
				return N_PASS;
			}
		};

	}
}


#endif /* defined(__TIMScript__PassNode__) */
