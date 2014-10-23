//
//  ClassNode.h
//  TIMScript
//
//  Created by Romain Beguet on 13.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ClassNode__
#define __TIMScript__ClassNode__

#include <iostream>
#include <map>
#include "AbstractNode.h"
#include "DefinitionReferenceNode.h"

namespace ts {
	namespace cmplr {
		class Definition;
	}

	namespace nodes {
		class Argument;
		class InterfaceNode;

		class ClassNode : public AbstractNode {
		public:

			ClassNode(size_t pos, size_t length, const std::string& n, ClassNode* superClass);
			virtual ~ClassNode();

			inline void resetNodeLocation(size_t pos, size_t length) {
				_pos = pos;
				_length = length;
			}

			inline const std::string& name() {
				return _name;
			}
			inline type::ClassType* typeClass() {
				return _classType;
			}

			const std::vector<DefinitionReferenceNode*>& defs() {
				return _defs;
			}
			void setDefs(const std::map<std::string, cmplr::Definition*> defs);
			ClassNode* setDef(DefinitionReferenceNode* ref);
			size_t getDefinitionIndex(const std::string& identifier);

			ClassNode* setAttributes(const std::vector<Argument*>& a);
			inline const std::vector<Argument*>& getAttributes() {
				return _attrs;
			}
			size_t getAttributeIndex(const std::string& identifier);

			inline void setAbstract(bool a) {
				_isAbstract = a;
			}

			void extends(ClassNode* clss);
			inline const ClassNode* superClass() const {
				return _superClass;
			}

			inline void setDebugInfoNodeIndex(size_t index) {
				_debugInfoIndex = index;
			}

			void addInterface(InterfaceNode* interface);
			inline const std::vector<InterfaceNode*>& getInterfaces() const {
				return _interfaces;
			}

			virtual void semanticTraverse();

			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_CLASS;
			}

		private:

			bool overload_or_override(DefinitionReferenceNode* base, DefinitionReferenceNode* derive);

			const std::string _name;
			std::vector<Argument*> _attrs;
			std::vector<DefinitionReferenceNode*> _defs;
			ClassNode* _superClass;

			size_t _debugInfoIndex;

			std::vector<InterfaceNode*> _interfaces;
			std::vector<std::vector<size_t>> _interfacesMap;

			type::ClassType* _classType;

			bool _isAbstract;
		};

	}
}


#endif /* defined(__TIMScript__ClassNode__) */
