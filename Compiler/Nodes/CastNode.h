//
//  CastNode.h
//  TIMScript
//
//  Created by Romain Beguet on 02.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__CastNode__
#define __TIMScript__CastNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
	namespace nodes {

		class CastNode : public AbstractNode {
		public:
			CastNode(size_t pos, size_t length, AbstractNode* e, type::Type* t);
			CastNode(AbstractNode* e, type::Type* t);
			virtual ~CastNode();

			inline AbstractNode* innerExp() {
				return _exp;
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			static bool isCastable(type::Type* from, type::Type* into);
			static bool isFunctionCastable(type::FunctionType* fta, type::FunctionType* ftb);

		protected:

			AbstractNode* _exp;
			type::Type* _type;
		};

		class HardCastNode : public CastNode {
		public:

			HardCastNode(size_t pos, size_t length, AbstractNode* e, type::Type* t);
			HardCastNode(AbstractNode* e, type::Type* t);

			virtual void semanticTraverse();
		};

	}
}

#endif /* defined(__TIMScript__CastNode__) */
