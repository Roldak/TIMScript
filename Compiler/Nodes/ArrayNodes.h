//
//  ArrayNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 27.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ArrayNodes__
#define __TIMScript__ArrayNodes__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class NewArrayNode : public AbstractNode {
		public:

			NewArrayNode(size_t pos, size_t length, type::ArrayType* type);
			virtual ~NewArrayNode() {}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_NEW_ARRAY;
			}
		};

		class MakeArrayNode : public AbstractNode {
		public:

			MakeArrayNode(size_t pos, size_t length, std::vector<AbstractNode*>& nodes, type::ArrayType* at)
				: AbstractNode(pos, length), _nodes(nodes), _arytype(at) {}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:
			std::vector<AbstractNode*>& _nodes;
			type::ArrayType* _arytype;
		};

	}
}

#endif /* defined(__TIMScript__ArrayNodes__) */
