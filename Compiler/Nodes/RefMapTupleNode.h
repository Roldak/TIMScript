//
//  RefMapTupleNode.h
//  TIMScript
//
//  Created by Romain Beguet on 29.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__RefMapTupleNode__
#define __TIMScript__RefMapTupleNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {

	namespace cmplr {
		class Variable;
	}

	namespace nodes {

		class RefMapTupleNode : public AbstractNode {
		public:

			RefMapTupleNode(const std::vector<std::pair<std::string, cmplr::Variable*>>& vars) : AbstractNode(0, 0),  _vars(vars) {}
			virtual ~RefMapTupleNode() {}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			const std::vector<std::pair<std::string, cmplr::Variable*>> _vars;

		};

		class TupleTypeRefMapTupleNode : public AbstractNode {
		public:

			TupleTypeRefMapTupleNode(const std::vector<type::Type*>& types) : AbstractNode(0, 0),  _types(types) {}
			virtual ~TupleTypeRefMapTupleNode() {}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		private:

			const std::vector<type::Type*> _types;

		};

	}
}

#endif /* defined(__TIMScript__RefMapTupleNode__) */
