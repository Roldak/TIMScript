//
//  Scope.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Scope__
#define __TIMScript__Scope__

#include <iostream>
#include <vector>
#include <map>
#include "TSType.h"

namespace ts {

	namespace nodes {
		class AbstractNode;
	}

	namespace cmplr {

		class Variable {
		public:

			Variable() : _index(0), _type(NULL), _ref(NULL) {}
			Variable(Variable* other, size_t offset) : _index(offset), _type(NULL), _ref(other) {}
			Variable(type::Type* t, size_t i) :  _index(i), _type(t), _ref(NULL) {}

			inline size_t index() const {
				return _index;
			}
			inline type::Type* type() const {
				return _ref ? _ref->type() : _type;
			}
			inline void setType(type::Type* t) {
				_type = t;
			}

		private:

			// either this
			size_t _index;
			type::Type* _type;

			// or this
			Variable* _ref;
		};

		class Definition {
		public:

			Definition() : _index(0), _type(NULL), _value(NULL), _nextOverloadedDef(NULL) {}
			Definition(const std::string& n, size_t i, type::Type* t, nodes::AbstractNode* v)
				: _name(n), _index(i), _type(t), _value(v), _nextOverloadedDef(NULL) {}

			inline const std::string& name() {
				return _name;
			}
			inline size_t index() const {
				return _index;
			}
			inline nodes::AbstractNode* value() const {
				return _value;
			}
			inline type::Type* type() const {
				return _type;
			}

			inline void setIndex(size_t i) {
				_index = i;
			}
			inline void setValue(nodes::AbstractNode* val) {
				_value = val;
			}
			inline void setType(type::Type* t) {
				_type = t;
			}

			inline void addOverloadedDefinition(Definition* d) {
				if (_nextOverloadedDef)
					_nextOverloadedDef->addOverloadedDefinition(d);
				else
					_nextOverloadedDef = d;
			}

			inline Definition* getNextOverloadedDefinition() const {
				return _nextOverloadedDef;
			}

		private:

			std::string _name;
			size_t _index;
			type::Type* _type;
			nodes::AbstractNode* _value;
			Definition* _nextOverloadedDef;
		};

		class Scope {
		public:

			Scope(Scope* p);

			inline Scope* parent() const {
				return _parent;
			}
			inline void setIndependant() {
				_offset = 0;
				_independant = true;
			}

			Variable* var(const std::string& name);
			Variable* setVar(const std::string& name, Variable* var);
			Variable* setVar(const std::string& name, type::Type* t);
			Variable* setStructVar(const std::string& name, type::StructType* t);

			Definition* def(const std::string& name);
			Definition* setDef(const std::string& name, size_t i, type::Type* t, nodes::AbstractNode* n, nodes::ClassNode* curClass = NULL);

			type::Type* getType(const std::string& name);
			void setType(const std::string& name, type::Type* t);
			void setUnimplementedType(const std::string& name, type::Type* t);

			void setUnimplemented(const std::string& name, type::Type* t, nodes::AbstractNode* n);
			void recoverUnimplemented(type::Type*, nodes::AbstractNode*);

			inline const std::map<std::string, Variable*>& getLocals() {
				return _locals;
			}
			inline const std::map<std::string, Definition*>& getDefs() {
				return _defs;
			}

		private:

			Scope* _parent;
			size_t _offset;
			size_t _nbLocals;

			std::map<std::string, Variable*> _locals;
			std::map<std::string, Definition*> _defs;
			std::map<std::string, type::Type*> _types;

			bool _independant;

		};

	}
}

#endif /* defined(__TIMScript__Scope__) */
