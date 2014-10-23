//
//  BinaryOperationNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__BinaryOperationNodes__
#define __TIMScript__BinaryOperationNodes__

#include <iostream>
#include "AssignableNode.h"
#include "Bytecode.h"

namespace ts {
	namespace nodes {

		template<OP_CODE code>
		std::string getOPCODElitteral() {
			switch (code) {
				case ADD_I:
					return "+";
				case SUB_I:
					return "-";
				case MUL_I:
					return "*";
				case DIV_I:
					return "/";
				case MOD_I:
					return "%";
				case POW_I:
					return "^";

				case CMP_I_G:
					return ">";
				case CMP_I_L:
					return "<";
				case CMP_I_EQ:
					return "==";
				case CMP_I_NEQ:
					return "!=";

				case AND_B:
					return "&&";
				case OR_B:
					return "||";

				default:
					break;
			}
			return "";
		}

		template<OP_CODE onInt, OP_CODE onReal, bool returnsBoolean>
		class BinaryOperation : public AbstractNode {
		public:

			BinaryOperation(size_t pos, size_t length, AbstractNode* l, AbstractNode* r)
				: AbstractNode(pos, length), _l(l), _r(r), _isMethodCall(NULL), _alrdyComputed(false) {}

			virtual ~BinaryOperation() {
				delete _l;
				if (!_isMethodCall)
					delete _r;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		protected:
			AbstractNode* _l;
			AbstractNode* _r;
			bool _isMethodCall;
			type::Type* _castType;
			bool _alrdyComputed;
		};

		template<OP_CODE code>
		class BinaryBoolOperation : public AbstractNode {
		public:

			BinaryBoolOperation(size_t pos, size_t length, AbstractNode* l, AbstractNode* r)
				: AbstractNode(pos, length), _l(l), _r(r), _isMethodCall(NULL), _alrdyComputed(false) {}

			virtual ~BinaryBoolOperation() {
				delete _l;
				if (!_isMethodCall)
					delete _r;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		protected:
			AbstractNode* _l;
			AbstractNode* _r;
			bool _isMethodCall;
			bool _alrdyComputed;
		};

		class IndexAccessOperation : public AssignableNode {
		public:

			IndexAccessOperation(size_t pos, size_t length, AbstractNode* a, AbstractNode* i)
				: AssignableNode(pos, length), _accessed(a), _index(i), _LType(NULL), _alrdyComputed(false) {}

			virtual ~IndexAccessOperation() {
				delete _accessed;
				//delete _index;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
			virtual void setInferedType(type::Type* t);
			virtual std::string toString();

			virtual NODE_TYPE getNodeType() {
				return N_ASSIGNABLE;
			}

		protected:
			AbstractNode* _accessed;
			AbstractNode* _index;
			type::ClassType* _LType; // initially null, takes a value if the operation involves calling a >method<
			bool _alrdyComputed;
		};

		class BinaryIs : public AbstractNode {
		public:

			BinaryIs(size_t pos, size_t length, AbstractNode* left, AbstractNode* right)
				: AbstractNode(pos, length), _l(left), _r(right) {}

			virtual ~BinaryIs() {
				delete _l;
				delete _r;
			}

			virtual void semanticTraverse();
			virtual void pushBytecode(std::vector<TSINSTR>& program);
			virtual std::string toString();

		protected:
			AbstractNode* _l;
			AbstractNode* _r;
		};

		typedef BinaryOperation<ADD_I, ADD_R, false> BinaryAdd;
		typedef BinaryOperation<SUB_I, SUB_R, false> BinarySub;
		typedef BinaryOperation<MUL_I, MUL_R, false> BinaryMul;
		typedef BinaryOperation<DIV_I, DIV_R, false> BinaryDiv;
		typedef BinaryOperation<MOD_I, MOD_R, false> BinaryMod;
		typedef BinaryOperation<POW_I, POW_R, false> BinaryPow;
		typedef BinaryOperation<CMP_I_L, CMP_R_L, true> BinaryLessThan;
		typedef BinaryOperation<CMP_I_G, CMP_R_G, true> BinaryGreaterThan;
		typedef BinaryOperation<CMP_I_EQ, CMP_R_EQ, true> BinaryEqualEqual;
		typedef BinaryOperation<CMP_I_NEQ, CMP_R_NEQ, true> BinaryNotEqual;
		typedef BinaryBoolOperation<AND_B> BinaryAnd;
		typedef BinaryBoolOperation<OR_B> BinaryOr;
	}
}

#endif /* defined(__TIMScript__BinaryOperationNodes__) */
