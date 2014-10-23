//
//  NewInstanceNode.h
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__NewInstanceNode__
#define __TIMScript__NewInstanceNode__

#include <iostream>
#include <map>
#include "AssignableNode.h"
#include "InterfaceNode.h"

namespace ts {

	namespace cmplr {
		class Definition;
	}

	namespace nodes {

		class DefinitionReferenceNode;

		class MakeObject : public AbstractNode {
		public:

			MakeObject(size_t pos, size_t length, ClassNode* c, bool scoped);
			virtual ~MakeObject() {}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		protected:

			ClassNode* _class;
			bool _scoped;
		};

		class NewInstanceNode : public AbstractNode {
		public:

			NewInstanceNode(size_t pos, size_t length, ClassNode* c, bool scoped);
			virtual ~NewInstanceNode() {}

			inline ClassNode* getClass() {
				return _class;
			}
			inline bool scoped() {
				return _scoped;
			}
			DefinitionReferenceNode* getConstructor();

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_NEW_INSTANCE;
			}

		protected:

			size_t _constrIndex;
			ClassNode* _class;
			bool _scoped;
		};

		class AttributeAccessNode : public AssignableNode {
		public:

			AttributeAccessNode(size_t pos, size_t length, AbstractNode* l, const std::string& i)
				: AssignableNode(pos, length), _left(l), _identifier(i) {}

			virtual ~AttributeAccessNode() {}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void setInferedType(type::Type* t);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_ATTRIBUTE_ACCESS;
			}

		protected:
			AbstractNode* _left;
			const std::string _identifier;

			size_t _attributeIndex;
		};

		class VirtualAccessNode : public AbstractNode {
		public:

			VirtualAccessNode(size_t pos, size_t length, AbstractNode* l, const std::string& i)
				: AbstractNode(pos, length), _forFunctionCall(false), _left(l), _identifier(i), _defIndex(0), _nbArgs(0) {}

			virtual ~VirtualAccessNode() {}

			DefinitionReferenceNode* definition();
			inline size_t definitionIndex() {
				return _defIndex;
			}
			void setDefinitionIndex(size_t defIndex);

			inline void setIsForFunctionCall(bool v) {
				_forFunctionCall = v;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_VIRTUAL_ACCESS;
			}

		protected:

			bool _forFunctionCall;

			AbstractNode* _left;
			const std::string _identifier;

			size_t _defIndex;
			size_t _nbArgs;
		};

		class ImplementationAccessNode : public AbstractNode {
		public:

			ImplementationAccessNode(size_t pos, size_t length, AbstractNode* l, const std::string& i)
				: AbstractNode(pos, length), _forFunctionCall(false), _left(l), _identifier(i), _interface(NULL) {}

			virtual ~ImplementationAccessNode() {}

			inline void setIsForFunctionCall(bool v) {
				_forFunctionCall = v;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_IMPLEMENTATION_ACCESS;
			}

		protected:

			bool _forFunctionCall;

			AbstractNode* _left;
			const std::string _identifier;

			InterfaceNode* _interface;
			ImplementationLocation _defLocation;
		};
	}
}

#endif /* defined(__TIMScript__NewInstanceNode__) */
