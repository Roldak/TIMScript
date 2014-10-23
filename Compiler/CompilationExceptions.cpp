//
//  CompilationExceptions.cpp
//  TIMScript
//
//  Created by Romain Beguet on 05.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "CompilationExceptions.h"
#include "AbstractNode.h"
#include "Utils.h"

namespace ts {
	namespace exception {

		CompilationException::CompilationException(nodes::AbstractNode* n) : _pos(n->pos()), _len(n->len()) {

		}

		const char* CompilationException::what() {
			get();
			_msg = "Pos " + T_toString(_pos) + " : " + _msg;
			return _msg.c_str();
		}

		void ParsingError::get() {

		}

		// ILLEGAL CAST

		void IllegalCast::get() {
			_msg = "Illegal cast. Cannot cast from " + _found->toString() + " to " + _expected->toString();
		}

		// ARGUMENT TYPE MISMATCH

		void ArgumentTypeMismatch::get() {
			_msg = "Argument #" + T_toString(_index) + " Type mismatch. Found : " + _found->toString() + ", expected : " + _expected->toString();
		}

		// ARGUMENT NUMBER MISMATCH

		void ArgumentNumberMismatch::get() {
			_msg = "Argument number mismatch. Found : " + T_toString(_found) + ", expected : " + T_toString(_expected);
		}

		// NON-VOID IF WITHOUT ELSE PART

		void NonVoidIfWithoutElsePart::get() {
			_msg = "Expected else part of if when then-part is not of void type (" + _expType->toString() + " found)";
		}

		// ...

		void ThenAndElseStatementHaveNotTheSameReturnType::get() {
			_msg = "Then part and Else part should have the same type. (found : " + _thenType->toString() + " and " + _elseType->toString() + ")";
		}

		// NOT REDEFINED ABSTRACT FUNCTION

		void NotRedefinedAbstractFunction::get() {
			_msg = "Abstract function '" + _funcName + "' was not redefined in non-abstract class " + _className;
		}

		// UNKNOWN STATIC MEMBER

		void UnknownStaticMember::get() {
			_msg = "Unknown static member '" + _member + "' in class " + _className;
		}

		// UNKNOWN MEMBER

		void UnknownMember::get() {
			_msg = "Unknown member '" + _member + "' in " + _class_interface + " " + _name;
		}

		// REFERING TO MISSING CONSTRUCTOR

		void ReferingMissingConstructor::get() {
			_msg = "Cannot instantiate class '" + _className + "', because there is no constructor";
		}

		// NUMBER OF ELEMENTS MISMATCH IN INITIALIZER LIST

		void NumberOfElementsMismatchInitializerList::get() {
			_msg = "Number of elements in initializer list mismatch. Found : " + T_toString(_found) + ", expected : " + T_toString(_expected);
		}

		// UNKNOWN TUPLE ELEMENT

		void UnknownTupleElement::get() {
			_msg = "No element '" + _element + "' in tuple " + _tupleType->toString();
		}

		// INVALID DOT OPERATION

		void InvalidDotOperation::get() {
			_msg = "Cannot perform dot operation ('." + _element + "') on type " + _type->toString();
		}

		// CASES HAVE DIFFERENT TYPES

		void CasesHaveDifferentTypes::get() {
			_msg = "Cases must be of the same type. Found : " + _found->toString() + ", expected : " + _expected->toString();
		}

		// MISSING DEFAULT CASE

		void MissingDefaultCase::get() {
			_msg = "Match statement needs a default case when its type is not void. (found : " + _type->toString() + ")";
		}

		void InvalidOperandBinaryIs::get() {
			_msg = "Invalid " + _left_right + " operand in binary is expression. Found : " + _found->toString() + ", expected : " + _expected;
		}

		// INVALID THROW VALUE

		void InvalidThrowValue::get() {
			_msg = "Invalid value thrown. Found : " + _found->toString() + ". Expected : Instance";
		}

		// UNSET STRUCTURE TYPE

		void UnsetStructureType::get() {
			_msg = "Type should be specified when expected type is a structure. (found : " + _found->toString() + ")";
		}

		void UndeducableType::get() {
			_msg = "Cannot deduce type of " + _node->toString();
		}

	}
}
