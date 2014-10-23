//
//  TSType.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__TSType__
#define __TIMScript__TSType__

#include <iostream>
#include <vector>
#include "Utils.h"

namespace ts {
	namespace nodes {
		class Argument;
		class ClassNode;
		class InterfaceNode;
	}

	enum TYPE {TBASIC, TFUNCTION, TARRAY, TCLASS, TINTERFACE, TCOMPOSED, TTUPLE, TSTRUCT};

	namespace type {

		class Type {
		public:
			virtual size_t size() {
				return 1;
			}
			virtual bool equals(Type* other) = 0;
			virtual TYPE type() = 0;
			virtual std::string toString() = 0;
		};

		class ComposedType;
		class InterfaceType;

		class BasicType : public Type {
		public:
			static Type* Bool;
			static Type* Int;
			static Type* Real;
			static Type* Void;
			static Type* Class;
			static Type* NotYetDefined;
			static Type* Any;

			static bool isBasicType(type::Type* t);

			virtual bool equals(Type* other) {
				return this == other;
			}
			virtual TYPE type() {
				return TYPE::TBASIC;
			}
			virtual std::string toString() {
				return _name;
			}

		protected:

			BasicType(const std::string& name) : _name(name) {}
			const std::string _name;
		};

		class FunctionType : public Type {
		public:

			FunctionType(const std::vector<Type*>& at, Type* rt, bool isN = false);

			inline const std::vector<Type*>& argsTypes() {
				return _argsTypes;
			}
			inline Type* returnType() {
				return _returnType;
			}

			inline void setReturnType(Type* t) {
				_returnType = t;
			}

			inline void setNative(bool r) {
				_isNative = r;
			}
			inline bool isNative() {
				return _isNative;
			}
			inline void setAbstract(bool a) {
				_isAbstract = a;
			}
			inline bool isAbstract() {
				return _isAbstract;
			}

			bool isOverrideOf(FunctionType* other);

			size_t totalArgsSize();
			std::vector<Type*> flattenTypes();

			virtual bool equals(Type* other);
			virtual TYPE type() {
				return TYPE::TFUNCTION;
			}
			virtual std::string toString();

			static FunctionType* RealToReal;
			static FunctionType* IntToInt;

		private:

			const std::vector<Type*> _argsTypes;
			Type* _returnType;
			bool _isNative;
			bool _isAbstract;
		};

		class ArrayType : public Type {
		public:
			ArrayType(Type* elemTypes, TSINT n) : _elementsType(elemTypes), _nbElems(n) {}

			inline Type* elementsType() {
				return _elementsType;
			}
			inline TSINT nbElements() {
				return _nbElems;
			}

			virtual bool equals(Type* other);
			virtual TYPE type() {
				return TYPE::TARRAY;
			}
			virtual std::string toString();

		private:
			Type* _elementsType;
			TSINT _nbElems;
		};

		class ClassType : public BasicType {
		public:

			ClassType(nodes::ClassNode* c);

			bool inheritsFrom(ClassType* c);
			bool inheritsFrom(InterfaceType* i);
			bool inheritsFrom(ComposedType* c);
			bool inheritsFrom(Type* t);

			inline nodes::ClassNode* getClass() {
				return _class;
			}
			inline void setClassDefIndex(size_t index) {
				_classDefIndex = index;
			}
			inline size_t getClassDefIndex() {
				return _classDefIndex;
			}

			virtual TYPE type() {
				return TYPE::TCLASS;
			}

		private:

			nodes::ClassNode* _class;
			size_t _classDefIndex;
		};

		class InterfaceType : public  BasicType {
		public:

			InterfaceType(nodes::InterfaceNode* i);

			inline nodes::InterfaceNode* getInterface() {
				return _interface;
			}

			bool inheritsFrom(ClassType* c);
			bool inheritsFrom(InterfaceType* i);
			bool inheritsFrom(ComposedType* c);
			bool inheritsFrom(Type* t);

			virtual TYPE type() {
				return TYPE::TINTERFACE;
			}

			virtual std::string toString();

		private:

			nodes::InterfaceNode* _interface;
		};

		class ComposedType : public Type {
		public:

			ComposedType(ClassType* ct, const std::vector<InterfaceType*>& interfaces);

			bool inheritsFrom(ClassType* c);
			bool inheritsFrom(InterfaceType* i);
			bool inheritsFrom(ComposedType* c);
			bool inheritsFrom(Type* t);

			bool implements(InterfaceType* i);

			ImplementationLocation getInterfaceOwning(const std::string& identifier);

			inline void setClass(ClassType* c) {
				_class = c;
			}
			inline ClassType* getClass() {
				return _class;
			}
			inline const std::vector<InterfaceType*>& getInterfaces() {
				return _interfaces;
			}

			virtual bool equals(Type* other);
			virtual TYPE type() {
				return TCOMPOSED;
			}
			virtual std::string toString();

		private:

			ClassType* _class;
			std::vector<InterfaceType*> _interfaces;
		};

		class TupleType : public Type {
		public:

			TupleType(const std::vector<Type*>& types, size_t refMapIndex) : _types(types), _refMapIndex(refMapIndex) {}

			inline const std::vector<Type*>& types() {
				return _types;
			}
			inline const size_t refMapIndex() {
				return _refMapIndex;
			}

			virtual bool equals(Type* other);
			virtual TYPE type() {
				return TYPE::TTUPLE;
			}
			virtual std::string toString();

		private:

			const std::vector<Type*> _types;
			const size_t _refMapIndex;
		};
		class StructType : public BasicType {
		public:

			StructType(const std::string& name, const std::vector<nodes::Argument*> fields);

			virtual size_t size() {
				return _size;
			}
			virtual TYPE type() {
				return TYPE::TSTRUCT;
			}

			inline const std::string& name() {
				return _name;
			}
			inline const std::vector<nodes::Argument*>& fields() {
				return _fields;
			}

			size_t fieldIndex(const std::string& fieldName);
			size_t fieldOffset(const std::string& fieldName);

			void addFlatten(std::vector<Type*>& types);

		private:

			const std::vector<nodes::Argument*> _fields;
			size_t _size;

		};

		template<class T>
		static T getIf(Type* t) {
			return NULL;
		}

		template<>
		BasicType* getIf<BasicType*>(Type* t) {
			return t->type() == TBASIC ? (BasicType*)t : NULL;
		}

		template<>
		FunctionType* getIf<FunctionType*>(Type* t) {
			return t->type() == TFUNCTION ? (FunctionType*)t : NULL;
		}

		template<>
		ArrayType* getIf<ArrayType*>(Type* t) {
			return t->type() == TARRAY ? (ArrayType*)t : NULL;
		}

		template<>
		ClassType* getIf<ClassType*>(Type* t) {
			return t->type() == TCLASS ? (ClassType*)t : NULL;
		}

		template<>
		InterfaceType* getIf<InterfaceType*>(Type* t) {
			return t->type() == TINTERFACE ? (InterfaceType*)t : NULL;
		}

		template<>
		ComposedType* getIf<ComposedType*>(Type* t) {
			return t->type() == TCOMPOSED ? (ComposedType*)t : NULL;
		}

		template<>
		TupleType* getIf<TupleType*>(Type* t) {
			return t->type() == TTUPLE ? (TupleType*)t : NULL;
		}

		template<>
		StructType* getIf<StructType*>(Type* t) {
			return t->type() == TSTRUCT ? (StructType*)t : NULL;
		}
	}
}

#endif /* defined(__TIMScript__TSType__) */
