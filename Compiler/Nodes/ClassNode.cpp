//
//  ClassNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 13.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ClassNode.h"
#include "FunctionNode.h"
#include "Scope.h"
#include "CompilationExceptions.h"
#include "CastUtils.h"

namespace ts {
	namespace nodes {

		bool ClassNode::overload_or_override(DefinitionReferenceNode* base, DefinitionReferenceNode* derive) {
			type::FunctionType* ftbase = NULL;
			type::FunctionType* ftderive = NULL;

			if (!base->definition())
				ftbase = getIf<type::FunctionType*>(base->definitionType());
			else if (FunctionNode* fbase = dynamic_cast<FunctionNode*>(base->definition()->value()))
				ftbase = fbase->getSignature();
			else if (base->definition()->type())
				ftbase = getIf<type::FunctionType*>(base->definition()->type());

			if (!derive->definition())
				ftderive = getIf<type::FunctionType*>(derive->definitionType());
			else if (FunctionNode* fderive = dynamic_cast<FunctionNode*>(derive->definition()->value()))
				ftderive = fderive->getSignature();
			else if (derive->definition()->type())
				ftderive = getIf<type::FunctionType*>(derive->definition()->type());

			if (ftbase && ftderive)
				return ftderive->isOverrideOf(ftbase);

			return true;
		}

		ClassNode::ClassNode(size_t pos, size_t length, const std::string& n, ClassNode* superClass)
			: AbstractNode(pos, length), _name(n), _attrs(), _defs(), _superClass(superClass),
			  _debugInfoIndex(std::string::npos), _isAbstract(false) {
			_cachedType = type::BasicType::Class;
			_classType = new type::ClassType(this);

			if (superClass) {
				_interfaces = superClass->_interfaces;
				_attrs = superClass->getAttributes();
				_defs = superClass->defs();
			}
		}

		ClassNode::~ClassNode() {
			for (size_t i = (_superClass == NULL ? 0 : _superClass->defs().size()); i < _defs.size(); ++i)
				delete _defs[i];
			for (size_t i = (_superClass == NULL ? 0 : _superClass->getAttributes().size()) ; i < _attrs.size(); ++i)
				delete _attrs[i];
		}

		void ClassNode::setDefs(const std::map<std::string, cmplr::Definition*> defs) {
			_defs.reserve(_defs.size() + defs.size());
			for (auto it = defs.begin(); it != defs.end(); ++it) {
				cmplr::Definition* curDef = it->second;
				do {
					setDef(new DefinitionReferenceNode(it->first, curDef));
					curDef = curDef->getNextOverloadedDefinition();
				} while (curDef);
			}
		}

		ClassNode* ClassNode::setDef(DefinitionReferenceNode* ref) {
			size_t index = getDefinitionIndex(ref->name());

			if (index != std::string::npos) {
				while (index < _defs.size() && _defs[index]->name() == ref->name()) {
					if (overload_or_override(_defs[index], ref)) {
						_defs[index] = ref;
						return this;
					}
					++index;
				}
				_defs.insert(_defs.begin() + (long)index, ref);
			} else {
				_defs.push_back(ref);
			}

			return this;
		}

		size_t ClassNode::getDefinitionIndex(const std::string& identifier) {
			for (size_t i = 0; i < _defs.size(); ++i)
				if (_defs[i]->name() == identifier)
					return i;
			return std::string::npos;
		}

		ClassNode* ClassNode::setAttributes(const std::vector<Argument*>& a) {
			// should check if an attribute name already exists in superclass
			_attrs.resize(_superClass ? _superClass->getAttributes().size() : 0);
			_attrs.insert(_attrs.begin() + (long)_attrs.size(), a.begin(), a.end());
			return this;
		}

		size_t ClassNode::getAttributeIndex(const std::string& identifier) {
			for (size_t i = 0; i < _attrs.size(); ++i)
				if (_attrs[i]->name() == identifier)
					return i;
			return std::string::npos;
		}

		void ClassNode::extends(ClassNode* clss) {
			_superClass = clss;
			_interfaces = clss->_interfaces;
			_attrs = clss->getAttributes();
			_defs = clss->defs();
		}

		void getAllInterfaces(InterfaceNode* i, std::vector<InterfaceNode*>& vec) {
			for (InterfaceNode* p : i->getInterfaces()) {
				getAllInterfaces(p, vec);
			}
			vec.push_back(i);
		}

		void ClassNode::addInterface(InterfaceNode* interface) {
			const size_t untilNow = _interfaces.size();
			getAllInterfaces(interface, _interfaces);

			for (size_t i = untilNow; i < _interfaces.size(); ++i) {
				for (const InterfaceField& field : _interfaces[i]->defs()) {
					setDef(field.def);
				}
			}
		}

		void ClassNode::semanticTraverse() {
			if (_isAbstract)
				return;

			for (auto it = _defs.begin(); it != _defs.end(); ++it) {
				if (!(*it)->definition())
					continue;

				type::Type* t = (*it)->definition()->type();
				if (type::FunctionType* ft = type::getIf<type::FunctionType*>(t)) {
					if (ft->isAbstract()) {
						throw exception::NotRedefinedAbstractFunction(this, _name, (*it)->name() + " (" + ft->toString() + ")");
					}
				}
			}

			_interfacesMap.clear();
			for (InterfaceNode* interface : _interfaces) {
				const std::vector<InterfaceField>& defs(interface->defs());
				std::vector<size_t> positions;

				for (size_t i = 0; i < defs.size(); ++i) {
					positions.push_back(getDefinitionIndex(defs[i].name));
				}

				_interfacesMap.push_back(positions);
			}
		}

		void ClassNode::pushBytecode(std::vector<TSINSTR>& program) {
			for (size_t i = 0; i < _defs.size(); ++i)
				_defs[i]->pushBytecode(program);

			program.push_back(MK_CLASS);
			program.push_back(_attrs.size());
			program.push_back(_defs.size());

			// pass the definition index of the class debug info

			program.push_back(_debugInfoIndex);

			// pass the definition index of the superclass if any, otherwise max_size_t

			program.push_back(_superClass == NULL ? std::string::npos : _superClass->typeClass()->getClassDefIndex());

			// next we precise at which index in the vtable is the destructor, if there is one, otherwise at max_size_t
			program.push_back(getDefinitionIndex("delete"));

			size_t here = program.size();
			program.push_back(0);

			// precise which of the attributes are references (for the GC)

			size_t count = 0;
			for (size_t i = 0; i < _attrs.size(); ++i) {
				if (!type::BasicType::isBasicType(_attrs[i]->type())) {
					++count;
					program.push_back(i);
				}
			}

			program[here] = count;

			if (_isAbstract) {
				program.push_back(0);
				return;
			}

			program.push_back(_interfaces.size());
			for (size_t i = 0; i < _interfaces.size(); ++i) {
				program.push_back(_interfaces[i]->interfaceID());
				program.push_back(_interfacesMap[i].size());
				for (size_t j = 0; j < _interfacesMap[i].size(); ++j) {
					program.push_back(_interfacesMap[i][j]);
				}
			}
		}

		std::string ClassNode::toString() {
			return "class " + _name;
		}

	}
}
