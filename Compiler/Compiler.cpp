//
//  Compiler.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Compiler.h"
#include "Optimizer.h"
#include <fstream>

#define CAST(type) ((type*)(*_currentToken))

#define IS(type) (_currentToken!=_lastToken ? (*_currentToken)->getType()==type : false)

#define NEXT_TOKEN delete *_currentToken++;//if(_currentToken==_lastToken) return false;

#define PARSE_UNR_PRIMARY parseBinaryExp(parsePrimary(), 60)

#define EXPECT_SEMICOLON    if (!(IS(TK_OPERATOR) && CAST(Operator)->opType()==PAREN_C) && !accept(SEMICOLON)) { \
		ERROR("missing ';'") \
	}

#define CUR_POSLENGTH (*_currentToken)->position(), (*_currentToken)->length()

#define ERROR(string) {throw exception::ParsingError(CUR_POSLENGTH, string);}

#define CUSTOMERROR(pos, len, string) {throw exception::ParsingError(pos, len, string);}

#include "CompilationExceptions.h"
#include "CompilationContext.h"

#include "BeginNode.h"
#include "BlockNode.h"
#include "ConstNodes.h"
#include "VariableNode.h"
#include "BinaryOperationNodes.h"
#include "UnaryOperationNodes.h"
#include "CastNode.h"
#include "PassNode.h"
#include "FunctionNode.h"
#include "CallNode.h"
#include "InterfaceNode.h"
#include "ClassNode.h"
#include "DotOperation.h"
#include "InstanceNodes.h"
#include "ArrayNodes.h"
#include "TupleNodes.h"
#include "InitializerListNode.h"
#include "AsyncNode.h"

#include "PrintNode.h"
#include "ConditionnalNodes.h"
#include "ExceptionNodes.h"
#include "DebugInfoNode.h"
#include "MatchNode.h"

#include "Interpreter.h"

#include "BytecodePrinter.h"
#include "CastUtils.h"
#include "RefMapTupleNode.h"

namespace ts {
	using namespace tok;
	using namespace nodes;

	namespace cmplr {

		using namespace tok;

		Compiler::Compiler(std::string p, bool debugMode)
			: _debug(debugMode), _usrDatas(), _baseScope(new Scope(NULL)), _defNumber(0), _interfaceCount(0) {
			_currentScope = _baseScope;
			_includePaths.push_back("");
			_includePaths.push_back(getWorkingDirectory(p));
		}

		Program* Compiler::compile(const std::string& source) {
			std::vector<tok::AbstractToken*> tokens = Tokenizer::tokenize(source);
			AbstractNode* root = parse(tokens);

			CompilationContext::enable(this);

			// produce the code for each constants

			std::vector<Procedure*> compiledCP(_constantPool.size() + 2);

			for (size_t i = 0; i < _constantPool.size(); ++i) {
				try {
					compiledCP[i] = getProc(_constantPool[i]->value(), _constantPool[i]->name(), true);
#ifdef DEBUG
					std::cerr << "@" << i << " '" << _constantPool[i]->name() << "' : " << std::endl << BytecodePrinter::translate(compiledCP[i]) << std::endl << std::endl;
#endif
				} catch (exception::CompilationException& ex) {
					std::cerr << ex.what() << std::endl;
				}
			}

			// add the definition of the reference map of the main procedure

			size_t mainProcRefMapIndex = createCurrentFunctionRefMapTupleNode();
			size_t mainProcExTableIndex = createCurrentFunctionExTableNode();
			_constantPool.push_back(_baseScope->def("@" + T_toString(mainProcRefMapIndex)));
			_constantPool.push_back(_baseScope->def("!" + T_toString(mainProcExTableIndex)));

			// produce the code of the main procedure

			Procedure* main = NULL;

			try {
				main = getProc(root, "main");

				// compile the reference map
				compiledCP[mainProcRefMapIndex] = getProc(_constantPool[mainProcRefMapIndex]->value(), _constantPool[mainProcRefMapIndex]->name(), true);
				// compile the exception table
				compiledCP[mainProcExTableIndex] = getProc(_constantPool[mainProcExTableIndex]->value(), _constantPool[mainProcExTableIndex]->name(), true);
#ifdef DEBUG
				std::cerr << "@" << mainProcRefMapIndex << " : " << BytecodePrinter::translate(compiledCP[mainProcRefMapIndex]) << std::endl << std::endl;
				std::cerr << "@" << mainProcExTableIndex << " : " << BytecodePrinter::translate(compiledCP[mainProcExTableIndex]) << std::endl << std::endl;
#endif
			} catch (exception::CompilationException& ex) {
				std::cerr << ex.what() << std::endl;
			}

			// create the array that contains the native definitions addresses

			TSDATA* nativesPool = new TSDATA[_usrDatas.size()];
			for (auto it = _usrDatas.begin(); it != _usrDatas.end(); ++it)
				nativesPool[it->second._index].Ref = new objects::Function((objects::NativeFunction*)it->second._data,
						"native");


			delete root;

			// then create the Program object and return it

			CompilationContext::disable();

			return new Program(compiledCP, nativesPool, mainProcRefMapIndex, mainProcExTableIndex, main);
		}


		AbstractNode* Compiler::parse(std::vector<AbstractToken*> tokens) {
			_currentToken = tokens.begin();
			_lastToken = tokens.end();
			_currentScope = _baseScope;
			_currentClass = NULL;
			_currentScopeNode = NULL;
			_currentFunctionExTableNode = new ExTableNode(CUR_POSLENGTH);
			AbstractNode* ret = parseStatement();
			addDefsToConstantPool(); // for the base scope
			checkUnresolvedLinks(true);
			return ret;
		}

		Procedure* Compiler::getProc(AbstractNode* exp, const std::string& name, bool forConstantPool) {
			std::vector<TSINSTR> code;
			exp->semanticTraverse();
			exp->pushBytecode(code);

			if (forConstantPool)
				code.push_back(RET_V);
			//else
			//    opt::Optimizer(code).optimize();

			TSINSTR* copy = new TSINSTR[code.size()];

			for (size_t i = 0; i < code.size(); ++i)
				copy[i] = code[i];

			return new Procedure(25, _currentFunctionLocalVariables.size(), copy, code.size(), NULL, NULL, NULL, name);
		}

		std::string Compiler::getWorkingDirectory(std::string path) {
			for (size_t i = path.length() - 1; i != 0; --i)
				if (path[i] == '/' || path[i] == '\\')
					return path.substr(0, i + 1);

			return "";
		}

		bool Compiler::isMoreOrElse(const std::string &msg) {

		}

		void Compiler::accept() {
			if (_currentToken == _lastToken) {
				tok::AbstractToken* blast = *(_currentToken - 1);
				CUSTOMERROR(blast->position(), blast->length(), "Incomplete code")
			}
			delete *_currentToken++;
		}

		bool Compiler::accept(TOKEN_TYPE t) {
			if (_currentToken == _lastToken)
				return false;
			if ((*_currentToken)->getType() == t) {
				NEXT_TOKEN
				return true;
			}

			return false;
		}

		bool Compiler::accept(OPERATOR_TYPE t) {
			if (_currentToken == _lastToken)
				return false;

			if ((*_currentToken)->getType() == TK_OPERATOR)
				if (CAST(Operator)->opType() == t) {
					NEXT_TOKEN
					return true;
				}

			return false;
		}

		bool Compiler::accept(KEYWORD_TYPE t) {
			if (_currentToken == _lastToken)
				return false;

			if ((*_currentToken)->getType() == TK_KEYWORD)
				if (CAST(Keyword)->kwType() == t) {
					NEXT_TOKEN
					return true;
				}

			return false;
		}

		std::string Compiler::expectIdentifier(const std::string& msg) {
			if (!IS(TK_IDENTIFIER)) {
				ERROR(msg)
			}

			std::string name(CAST(Identifier)->name());
			accept();
			return name;
		}

		void Compiler::startScope() {
			_currentScope = new Scope(_currentScope);
		}

		void Compiler::startScope(const std::vector<nodes::LocalVariableNode*>& captures, const std::vector<Argument*>& args) {
			startScope();
			_currentScope->setIndependant();
			for (size_t i = 0; i < captures.size(); ++i)
				makeVar(captures[i]->name(), captures[i]->var());
			for (size_t i = 0; i < args.size(); ++i)
				makeVar(args[i]->name(), args[i]->type());
		}

		void Compiler::endScope() {
			checkUnresolvedLinks();
			addDefsToConstantPool();

			Scope* tmp = _currentScope;
			_currentScope = _currentScope->parent();
			delete tmp;
		}

		AbstractNode* Compiler::parseStatement() {
			size_t init_pos = (*_currentToken)->position();
			size_t init_len = (*_currentToken)->length();

			if (IS(TK_KEYWORD)) {
				KEYWORD_TYPE t = CAST(Keyword)->kwType();
				accept();
				switch (t) {
					case BEGIN:
						return new BeginNode(CUR_POSLENGTH, parseBlock());
					case LET:
						return parseVariableDeclaration(init_pos, init_len);
					case DEF:
						return parseDefinition();
					case TYPEDEF:
						return parseTypedef();
					case PRINT:
						return parsePrintNode(init_pos, init_len);
					case IF:
						return parseIfNode(init_pos, init_len);
					case WHILE:
						return parseWhileNode(init_pos, init_len);
					case FOR:
						return parseForNode(init_pos, init_len);
					case FOREACH:
						return parseForeachNode(init_pos, init_len);
					case CLASS:
						return parseClassDeclaration(false);
					case INTERFACE:
						return parseInterfaceDeclaration();
					case ABSTRACT:
						if (accept(CLASS)) return parseClassDeclaration(true);
					case RETURN:
						return new ExplicitReturnNode(CUR_POSLENGTH, parseExpression());
					case SCOPE:
						return new ScopeNode(CUR_POSLENGTH, parseBlock());
					case ASYNC:
						return parseAsyncNode();
					case MATCH:
						return parseMatchNode();
					case INCLUDE:
						return parseInclude();
					case IMPORT:
						return parseImport();
					case TRY:
						return parseTryCatchNode();
					case KTHROW:
						return new ThrowNode(CUR_POSLENGTH, parseExpression());
					case STRUCT:
						return parseStructureDeclaration();
					default:
						break;
				}
			} else
				return parseExpression();

			ERROR("Expected keyword")
		}

		AbstractNode* Compiler::parseBlock() {

			BlockNode* ret = new BlockNode(CUR_POSLENGTH, _debug);

			startScope();

			if (accept(BRACE_O)) {

				while (!accept(BRACE_C)) {
					if (_currentToken == _lastToken) {
						CUSTOMERROR(0, 1, "Block left unclosed")
					}

					AbstractNode* n = parseStatement();

					if (_currentToken == _lastToken) {
						CUSTOMERROR(0, 1, "Block left unclosed")
					}

					if (n == NULL)
						break;

					if (n->getNodeType() != N_PASS)
						ret->addNode(n);
				}

			} else
				ret->addNode(parseStatement());

			endScope();
			return ret;
		}

		AbstractNode* Compiler::parseExpression() {
			AbstractNode* exp = parseBinaryExp(parsePrimary(), 0);
			accept(SEMICOLON);
			return exp;
		}

		AbstractNode* Compiler::parseBinaryExp(AbstractNode* left, int precedence) {
			while (IS(TK_OPERATOR)) {
				if (CAST(Operator)->getOperatorPrecedence() >= precedence) {
					OPERATOR_TYPE op = CAST(Operator)->opType();
					int prec = CAST(Operator)->getOperatorPrecedence();

					accept(); //eat operator

					if (AbstractNode* tmp = parseSpecialBinaryExp(left, op)) {
						left = tmp;
						continue;
					}

					AbstractNode* right = parsePrimary();
					if (right == NULL)
						return left;

					while (IS(TK_OPERATOR))
						if (CAST(Operator)->getOperatorPrecedence() > prec)
							right = parseBinaryExp(right, CAST(Operator)->getOperatorPrecedence());
						else
							break;

					left = makeBinaryExp(left, op, right);
				} else
					break;
			}

			return left;
		}

		AbstractNode* Compiler::parseSpecialBinaryExp(AbstractNode* left, tok::OPERATOR_TYPE op) {
			using namespace tok;
			switch (op) {
				case COLON:
					return new HardCastNode(left->pos(), left->len(), left, parseTypeDeclaration());
				case PAREN_O:
					return new CallNode(left->pos(), left->len(), left, parseCallArguments());
				case DOT:
					return new DotOperationNode(left->pos(), left->len(), left, expectIdentifier("Expected Identifier"));
				case BRACKET_O: {
					AbstractNode* exp = parseExpression();
					if (!accept(BRACKET_C)) {
						ERROR("Expected ']'")
					}
					return new IndexAccessOperation(left->pos(), left->len(), left, exp);
				}
				default:
					return NULL;
			}
		}

		AbstractNode* Compiler::makeBinaryExp(AbstractNode* l, tok::OPERATOR_TYPE op, AbstractNode* r) {
			using namespace tok;

			size_t pos = l->pos();
			size_t len = l->len();

			switch (op) {
				case PLUS:
					return new BinaryAdd(pos, len, l, r);
				case MINUS:
					return new BinarySub(pos, len, l, r);
				case TIMES:
					return new BinaryMul(pos, len, l, r);
				case SLASH:
					return new BinaryDiv(pos, len, l, r);
				case MODULO:
					return new BinaryMod(pos, len, l, r);
				case POWER:
					return new BinaryPow(pos, len, l, r);
				case INF:
					return new BinaryLessThan(pos, len, l, r);
				case SUP:
					return new BinaryGreaterThan(pos, len, l, r);
				case EQUAL_EQUAL:
					return new BinaryEqualEqual(pos, len, l, r);
				case NOT_EQUAL:
					return new BinaryNotEqual(pos, len, l, r);
				case AND:
					return new BinaryAnd(pos, len, l, r);
				case OR:
					return new BinaryOr(pos, len, l, r);
				case ISTYPE:
					return new BinaryIs(pos, len, l, r);
				case EQUAL:
					return new AssignementNode(pos, len, (AssignableNode*)l, r);

				default:
					ERROR("Unknown binary operator " + Operator(0, 0, op).toString())
			}
		}

		AbstractNode* Compiler::parseUnaryExp() {
			OPERATOR_TYPE op = CAST(Operator)->opType();
			accept();
			switch (op) {
				case MINUS:
					return new UnaryMinus(CUR_POSLENGTH, PARSE_UNR_PRIMARY);
				case PLUS_PLUS:
					return new UnaryPlusPlus(CUR_POSLENGTH, (AssignableNode*)PARSE_UNR_PRIMARY);
				case MINUS_MINUS:
					return new UnaryMinusMinus(CUR_POSLENGTH, (AssignableNode*)PARSE_UNR_PRIMARY);
				default:
					ERROR("Unknow unary operator '" + tok::Operator(0, 0, op).toString() + "'")
					return NULL;
			}
		}

		AbstractNode* Compiler::parsePrimary() {
			AbstractNode* n = NULL;
			switch ((*_currentToken)->getType()) {
				case TK_BOOLEAN:
					n = new BooleanNode(CUR_POSLENGTH, CAST(BooleanConstToken)->value());
					accept();
					return n;
				case TK_INTEGER:
					n = new IntegerNode(CUR_POSLENGTH, CAST(IntegerConstToken)->value());
					accept();
					return n;
				case TK_REAL:
					n = new RealNode(CUR_POSLENGTH, CAST(RealConstToken)->value());
					accept();
					return n;
				case TK_STRING:
					n = parseStringConstant();
					accept();
					return n;
				case TK_IDENTIFIER:
					return parseIdentifier();
				case TK_KEYWORD:
					return parsePrimaryKeyword();
				case TK_OPERATOR:
					return parsePrimaryOperator();

				default:
					ERROR("Unexpected token : " + (*_currentToken)->toString());
					return n;
			}
		}

		AbstractNode* Compiler::parseIdentifier() {
			size_t init_pos = (*_currentToken)->position();
			size_t init_len = (*_currentToken)->length();

			std::string name(CAST(Identifier)->name());
			accept();

			if (name == "null")
				return new NullReferenceNode(init_pos, init_len);
			else if (Variable* v = _currentScope->var(name))
				return new LocalVariableNode(init_pos, init_len, name, v);
			else if (_currentClass && _currentClass->getAttributeIndex(name) != std::string::npos)
				return new DotOperationNode(init_pos, init_len, new LocalVariableNode(CUR_POSLENGTH, "this", getThis()), name);
			else if (Definition* d = _currentScope->def(name))
				return new DefinitionReferenceNode(init_pos, init_len, name, d);
			else if (NativeDefinitionReferenceNode* nr = getNativeReference(name))
				return nr;
			else {
				DefinitionReferenceNode* r = new DefinitionReferenceNode(init_pos, init_len, name, NULL);
				_unresolvedReferences.push_back(r);
				return r;
			}
		}

		AbstractNode* Compiler::parsePrimaryOperator() {
			if (CAST(Operator)->opType() == BRACE_O)
				return parseBlock();
			else if (accept(PAREN_O)) {
				AbstractNode* exp = parseExpression();
				if (!accept(PAREN_C)) {
					ERROR("Expected ')'")
				}
				return exp;
			} else if (CAST(Operator)->opType() == BRACKET_O) {
				return parseFunctionDeclaration(false);
			} else {
				bool scoped = false;
				if (accept(tok::SCOPED))
					scoped = true;
				else if (!accept(tok::NEW))
					return parseUnaryExp();

				type::Type* t = parseTypeDeclaration();

				if (type::ClassType* classT = type::getIf<type::ClassType*>(t))
					return new NewInstanceNode(CUR_POSLENGTH, classT->getClass(), scoped);
				else if (type::ArrayType* arrayT = type::getIf<type::ArrayType*>(t))
					return new NewArrayNode(CUR_POSLENGTH, arrayT);
				else if (type::TupleType* tupleT = type::getIf<type::TupleType*>(t))
					return new NewTupleNode(CUR_POSLENGTH, tupleT);
			}
			return parseUnaryExp();
		}

		AbstractNode* Compiler::parsePrimaryKeyword() {
			size_t init_pos = (*_currentToken)->position();
			size_t init_len = (*_currentToken)->length();

			if (accept(IF))
				return parseIfNode(init_pos, init_len);
			else if (accept(SCOPE))
				return new ScopeNode(CUR_POSLENGTH, parseBlock());
			else if (accept(ASYNC))
				return parseAsyncNode();
			else if (accept(MATCH))
				return parseMatchNode();
			else if (accept(TRY))
				return parseTryCatchNode();
			else {
				ERROR("Keyword '" + (*_currentToken)->toString() + "' cannot be used as an expression")
			}
		}

		AbstractNode* Compiler::parseFunctionDeclaration(bool asMember, InterfaceNode* interface) {
			std::vector<LocalVariableNode*> captures;

			if (accept(BRACKET_O)) {
				size_t pos = (*_currentToken)->position();

				std::vector<AbstractNode*> nodes(parseCallArguments(BRACKET_C));

				bool isNextTokenBRACEO = IS(TK_OPERATOR) && CAST(Operator)->opType() == BRACE_O;

				if (!accept(PAREN_O) && !isNextTokenBRACEO) {
					return new InitializerListNode(pos, (*_currentToken)->length(), nodes);
				}

				for (AbstractNode* node : nodes) {
					if (node->getNodeType() != NODE_TYPE::N_LOCAL_VARIABLE_NODE) {
						CUSTOMERROR(node->pos(), node->len(), "Expected capture of local variable")
					}
				}

				captures = *((std::vector<LocalVariableNode*>*)&nodes);

				if (isNextTokenBRACEO) {
					return parseFunctionDeclarationContent(captures, {}, type::BasicType::NotYetDefined);
				}
			}

			std::vector<Argument*> args(parseArguments());

			if (_currentClass) {
				if (asMember)
					args.insert(args.begin(), new Argument("this", _currentClass->typeClass()));
				else
					captures.insert(captures.begin(), new LocalVariableNode(CUR_POSLENGTH, "this", _currentScope->var("this")));
			} else if (interface) {
				args.insert(args.begin(), new Argument("this", interface->typeInterface()));
			}

			type::Type* returnType = type::BasicType::NotYetDefined;

			if (accept(TINY_RIGHT_ARROW))
				returnType = parseTypeDeclaration();

			if (!accept(RIGHT_ARROW)) {
				ERROR("Expected '=>'")
			}

			return parseFunctionDeclarationContent(captures, args, returnType);
		}

		AbstractNode* Compiler::parseFunctionDeclarationContent(const std::vector<LocalVariableNode*>& captures,
				const std::vector<Argument*>& args,
				type::Type* returnType) {

			std::vector<std::pair<std::string, cmplr::Variable*>> lastFuncRefMap = _currentFunctionLocalVariables;
			ExTableNode* lastExTableNode = _currentFunctionExTableNode;

			_currentFunctionLocalVariables = std::vector<std::pair<std::string, cmplr::Variable*>>();
			_currentFunctionExTableNode = new ExTableNode(CUR_POSLENGTH);

			startScope(captures, args);
			AbstractNode* body = parseBlock();
			endScope();

			size_t refMapTuplePos = createCurrentFunctionRefMapTupleNode();
			size_t exTableNodePos = createCurrentFunctionExTableNode();
			size_t debugInfoPos = _debug ? createCurrentFunctionDebugInfoNode() : std::string::npos;
			size_t nbLocals = _currentFunctionLocalVariables.size();

			_currentFunctionExTableNode = lastExTableNode;
			_currentFunctionLocalVariables = lastFuncRefMap;

			return new FunctionNode(CUR_POSLENGTH, args, captures, returnType,
									new ReturnNode(CUR_POSLENGTH, body), refMapTuplePos, exTableNodePos, nbLocals,
									debugInfoPos);
		}

		AbstractNode* Compiler::parseClassDeclaration(bool isAbstract) {
			const std::string name(expectIdentifier("Expected class Name"));

			ClassNode* clss;
			Definition* classDef;

			if ((classDef = _currentScope->def(name))) {
				clss = (ClassNode*)classDef->value();
				clss->resetNodeLocation(CUR_POSLENGTH);
			} else {
				clss = new ClassNode(CUR_POSLENGTH, name, NULL);
				_currentScope->setType(name, clss->typeClass());

				/* because a class needs to reference its method (accessed via load_cst) at its creation (mk_class)
				 we need its definition index to be higher than the definition it contains.
				 we will therefore set it to 0 initially, and correct it once we know the
				 last definition number of the definitons contained in the class */

				classDef = _currentScope->setDef(name, 0, type::BasicType::Class, clss, _currentClass);
			}

			clss->setAbstract(isAbstract);

			if (!accept(BRACKET_O)) {
				ERROR("Expected '['")
			}

			const std::vector<Argument*>& attrs(parseArguments(BRACKET_C));

			if (accept(EXTENDS)) {
				bool classFound = false;
				do {

					type::Type* t = parseTypeDeclaration();

					if (type::getIf<type::ClassType*>(t)) {
						if (classFound) {
							ERROR("Cannot inherit from multiple classes")
						}

						classFound = true;
						clss->extends(((type::ClassType*)t)->getClass());
					} else if (type::getIf<type::InterfaceType*>(t)) {
						clss->addInterface(((type::InterfaceType*)t)->getInterface());
					} else {
						ERROR("Expected class or interface to inherit from")
					}

				} while (accept(COMMA));

			}

			clss->setAttributes(attrs);

			if (!accept(BRACE_O)) {
				ERROR("Expected '{'")
			}

			startScope();
			ClassNode* lastClass = _currentClass;
			_currentClass = clss;

			while (!accept(BRACE_C)) {
				ClassNode* lastClass = _currentClass;
				if (accept(CLASS))
					delete parseClassDeclaration(false);
				else if (IS(TK_OPERATOR) && CAST(Operator)->opType() == tok::NEW)
					delete parseDefinition();
				else if (IS(TK_IDENTIFIER) && CAST(Identifier)->name() == "delete")
					delete parseDefinition();
				else {
					if (accept(ABSTRACT)) {
						if (accept(CLASS))
							delete parseClassDeclaration(true);
						else
							parseAbstractFieldDeclaration();
						continue;
					}

					if (accept(STATIC))
						_currentClass = NULL;

					if (accept(DEF))
						delete parseDefinition();
					else {
						ERROR("Expected definition statement")
					}
				}

				_currentClass = lastClass;
			}

			_currentClass = lastClass;

			clss->setDefs(_currentScope->getDefs());
			clss->typeClass()->setClassDefIndex(_defNumber);
			classDef->setIndex(_defNumber++); // set the definition index of the class to the right one
			clss->setDebugInfoNodeIndex(createCurrentClassDebugInfoNode(clss));

			endScope();

			return new PassNode(CUR_POSLENGTH);
		}

		AbstractNode* Compiler::parseInterfaceDeclaration() {
			const std::string name(expectIdentifier("Expected interface Name"));

			InterfaceNode* interface;
			type::Type* tryType = _currentScope->getType(name);
			type::InterfaceType* t = (tryType == NULL ? NULL : type::getIf<type::InterfaceType*>(tryType));

			if (t) {
				interface = t->getInterface();
				interface->resetNodeLocation(CUR_POSLENGTH);
			} else {
				interface = new InterfaceNode(CUR_POSLENGTH, name, _interfaceCount++);
				_currentScope->setType(name, interface->typeInterface());
			}

			if (accept(EXTENDS)) {
				do {
					type::Type* t = parseTypeDeclaration();
					if (type::InterfaceType* it = type::getIf<type::InterfaceType*>(t))
						interface->addInterface(it->getInterface());
					else
						ERROR("Expected interface to extend")

					} while (accept(COMMA));
			}

			if (!accept(BRACE_O)) {
				ERROR("Expected '{'")
			}

			startScope();

			while (!accept(BRACE_C)) {
				if (accept(ABSTRACT)) {
					const std::string defName(expectIdentifier("Expected abstract field name"));

					accept(COLON);

					if (type::FunctionType* func = type::getIf<type::FunctionType*>(parseTypeDeclaration(interface->typeInterface()))) {
						func->setAbstract(true);
						interface->addDefinition(defName, new Definition(defName, 0, func, NULL));
					} else {
						ERROR("Expected function type")
					}

				} else if (accept(DEF)) {
					delete parseDefinition(interface);
				} else {
					ERROR("Expected 'abstract' keyword")
				}
			}

			interface->addDefinitions(_currentScope->getDefs());

			endScope();

			return new PassNode(CUR_POSLENGTH);
		}

		AbstractNode* Compiler::parseStructureDeclaration() {
			std::string name = expectIdentifier("Expected struct name");

			if (!accept(BRACKET_O)) {
				ERROR("Expected '['")
			}

			const std::vector<Argument*>& fields(parseArguments(BRACKET_C));

			if (fields.size() == 0) {
				ERROR("Structures should have at least one field")
			}

			_currentScope->setType(name, new type::StructType(name, fields));

			return new PassNode(CUR_POSLENGTH);
		}

		AbstractNode* Compiler::parseStringConstant() {
			const std::string val = CAST(StringConstToken)->value();
			AbstractNode* n = new StringNode(CUR_POSLENGTH, findClass("String"), val);
			++_defNumber;
			return new DefinitionReferenceNode(CUR_POSLENGTH, val,
											   _currentScope->setDef("$" + val + T_toString(_defNumber - 1),
													   _defNumber - 1,
													   type::BasicType::NotYetDefined, n));
		}

		AbstractNode* Compiler::parsePrintNode(size_t init_pos, size_t init_len) {
			std::vector<AbstractNode*> nodes;

			do {
				nodes.push_back(parseExpression());
			} while (accept(COMMA));

			return new PrintNode(init_pos, init_len, nodes);
		}

		// Statement nodes

		AbstractNode* Compiler::parseIfNode(size_t init_pos, size_t init_len) {
			if (!accept(PAREN_O)) {
				ERROR("expected '('")
			}

			AbstractNode* cond = parseExpression();

			if (!accept(PAREN_C)) {
				ERROR("expected ')'")
			}

			AbstractNode* then = parseBlock();
			AbstractNode* elseN = NULL;

			if (accept(ELSE))
				elseN = parseBlock();

			return new IfNode(init_pos, init_len, cond, then, elseN);
		}

		AbstractNode* Compiler::parseWhileNode(size_t init_pos, size_t init_len) {
			if (!accept(PAREN_O)) {
				ERROR("expected '('")
			}

			AbstractNode* cond = parseExpression();

			if (!accept(PAREN_C)) {
				ERROR("expected ')'")
			}

			AbstractNode* toLoop = parseBlock();

			return new WhileNode(init_pos, init_len, cond, toLoop);
		}

		AbstractNode* Compiler::parseForNode(size_t init_pos, size_t init_len) {
			if (!accept(PAREN_O)) {
				ERROR("expected '('")
			}

			AbstractNode* init = parseStatement(); // parseVariableDeclaration();

			AbstractNode* cond = parseExpression();

			AbstractNode* incr = parseExpression();

			if (!accept(PAREN_C)) {
				ERROR("expected ')'")
			}

			AbstractNode* toLoop = parseBlock();

			return new ForNode(init_pos, init_len, init, cond, incr, toLoop);
		}

		AbstractNode* Compiler::parseForeachNode(size_t init_pos, size_t init_len) {
			if (!accept(PAREN_O)) {
				ERROR("expected '('")
			}

			std::string varName = expectIdentifier("Expected variable name");
			type::Type* varType = type::BasicType::NotYetDefined;

			if (accept(COLON)) {
				varType = parseTypeDeclaration();
			}

			if (!accept(IN)) {
				ERROR("expected 'in'")
			}

			startScope();

			cmplr::Variable* iterator = makeVar("_it_", type::BasicType::NotYetDefined);

			BlockNode* n = new BlockNode(init_pos, init_len, _debug);
			n->addNode(
				new AssignementNode(CUR_POSLENGTH,
									new LocalVariableNode(CUR_POSLENGTH, "_it_", iterator),
									new CallNode(CUR_POSLENGTH,
												 new DotOperationNode(CUR_POSLENGTH, parseExpression(), "iterator"),
												 std::vector<AbstractNode*>())));


			if (!accept(PAREN_C)) {
				ERROR("expected ')'")
			}

			cmplr::Variable* var = makeVar(varName, varType);

			BlockNode* inner = new BlockNode(CUR_POSLENGTH, _debug);

			AbstractNode* nextCallNode = new CallNode(CUR_POSLENGTH,
					new DotOperationNode(CUR_POSLENGTH,
										 new LocalVariableNode(CUR_POSLENGTH, "_it_", iterator),
										 "next"),
					std::vector<AbstractNode*>());

			if (varType != type::BasicType::NotYetDefined) {
				nextCallNode = new HardCastNode(CUR_POSLENGTH, nextCallNode, varType);
			}

			inner->addNode(new AssignementNode(CUR_POSLENGTH,
											   new LocalVariableNode(CUR_POSLENGTH, varName, var),
											   nextCallNode));

			inner->addNode(parseBlock());

			n->addNode(new WhileNode(CUR_POSLENGTH,
									 new CallNode(CUR_POSLENGTH,
												  new DotOperationNode(CUR_POSLENGTH,
														  new LocalVariableNode(CUR_POSLENGTH, "_it_", iterator),
														  "hasNext"),
												  std::vector<AbstractNode*>()),
									 inner));



			endScope();

			return n;
		}

		AbstractNode* Compiler::parseDefinition(InterfaceNode* interface) {
			do {
				std::string name;
				if (IS(TK_OPERATOR) && _currentClass) {
					Operator* op = CAST(Operator);
					name = "_" + op->toString() + "_";
					OPERATOR_TYPE opType = op->opType();
					accept();
					if (opType == OPERATOR_TYPE::BRACKET_O) {
						if (!accept(BRACKET_C)) {
							ERROR("Expected ']'")
						}
					}
				} else
					name = expectIdentifier("Expected definition name");

				if (IS(TK_OPERATOR) && (CAST(Operator)->opType() == EQUAL || CAST(Operator)->opType() == COLON)) {

					type::Type* t = accept(COLON) ? parseTypeDeclaration() : type::BasicType::NotYetDefined;
					Definition* def = _currentScope->setDef(name, 0, t, NULL, _currentClass);
					DefinitionReferenceNode* defref = new DefinitionReferenceNode(CUR_POSLENGTH, name, def);
					AbstractNode* exp = parseBinaryExp(defref, 0);

					if (exp == defref) { // is we could not parse something, e.g. there wasn't an assigenement
						ERROR("Expected assignement")
					}

					if (t == type::BasicType::NotYetDefined)
						def->setValue(((AssignementNode*)exp)->value());
					else
						def->setValue(new CastNode(((AssignementNode*)exp)->value(), t));

					def->setIndex(_defNumber++);

				} else if (accept(PAREN_O)) {
					Definition* def = _currentScope->setDef(name, _defNumber++, type::BasicType::NotYetDefined, NULL, _currentClass);
					DefinitionReferenceNode* defref = new DefinitionReferenceNode(CUR_POSLENGTH, name, def);
					AssignementNode* exp = new AssignementNode(CUR_POSLENGTH, defref,
							interface == NULL ? parseFunctionDeclaration() :
							parseFunctionDeclaration(false, interface));

					if (_currentClass)
						adjustIfIsConstructor(exp->value(), name);

					def->setValue(exp->value());
				} else {
					ERROR("Expected type declaration or assignement")
				}
			} while (accept(COMMA));

			accept(SEMICOLON);

			return new PassNode(CUR_POSLENGTH);
		}

		AbstractNode* Compiler::parseTypedef() {
			std::string name(expectIdentifier("Expected name of new type"));

			type::Type* t = parseTypeDeclaration();
			_currentScope->setType(name, t);
			return new PassNode(CUR_POSLENGTH);
		}

		AbstractNode* Compiler::parseAsyncNode() {
			AbstractNode* function;

			if (!IS(TK_OPERATOR) || (IS(TK_OPERATOR) && CAST(Operator)->opType() != BRACKET_O))
				function = parseFunctionDeclarationContent({}, {}, type::BasicType::NotYetDefined);
			else
				function = parseFunctionDeclaration();

			DefinitionReferenceNode* ref;

			if (Definition* d = _currentScope->def("Thread"))
				ref = new DefinitionReferenceNode(CUR_POSLENGTH, "Thread", d);
			else {
				ref = new DefinitionReferenceNode(CUR_POSLENGTH, "Thread", NULL);
				_unresolvedReferences.push_back(ref);
			}

			return new AsyncNode(CUR_POSLENGTH, function, ref);
		}

		AbstractNode* Compiler::parseMatchNode() {

			size_t pos = (*_currentToken)->position();
			size_t len = (*_currentToken)->length();

			if (!accept(PAREN_O)) {
				ERROR("Expected '('");
			}

			AbstractNode* exp = parseExpression();

			if (!accept(PAREN_C)) {
				ERROR("Expected ')'");
			}

			if (!accept(BRACE_O)) {
				ERROR("Expected '{'");
			}

			std::vector<TSINT> cases;
			std::vector<AbstractNode*> blocks;

			AbstractNode* def = NULL;

			while (!accept(BRACE_C)) {
				if (accept(DEFAULT)) {
					if (def) {
						ERROR("Default case already specified.");
					}
					if (!accept(RIGHT_ARROW)) {
						ERROR("Expected '=>'")
					}

					def = parseExpression();
					continue;
				}

				if (!accept(CASE)) {
					ERROR("Expected case statement");
				}

				if (!IS(TK_INTEGER)) {
					ERROR("Expected integer litteral");
				}

				TSINT val = CAST(IntegerConstToken)->value();
				accept();

				if (!accept(RIGHT_ARROW)) {
					ERROR("Expected '=>'")
				}

				AbstractNode* block = parseExpression();

				cases.push_back(val);
				blocks.push_back(block);
			}

			return new MatchNode(pos, len, exp, cases, blocks, def);
		}

		AbstractNode* Compiler::parseTryCatchNode() {
			AbstractNode* exp = parseStatement();

			if (!accept(CATCH)) {
				ERROR("Expected catch keyword")
			}

			TryCatchNode* trycatch = new TryCatchNode(CUR_POSLENGTH, exp);

			do {

				if (!accept(PAREN_O)) {
					ERROR("Expected '('")
				}

				std::string varName = expectIdentifier("Expected name of catched exception");

				if (!accept(ISTYPE)) {
					ERROR("Expected 'is'")
				}

				type::Type* varType = parseTypeDeclaration();

				if (!accept(PAREN_C)) {
					ERROR("Expected ')'")
				}

				Variable* exVar = makeVar(varName, varType);
				startScope();
				trycatch->addCatchNode(exVar, parseExpression());
				endScope();


			} while (accept(CATCH));

			_currentFunctionExTableNode->addTryCatchNode(trycatch);

			return trycatch;
		}

		// UTILITY

		type::Type* Compiler::parseTypeDeclaration(type::Type* forAbstractDeclaration) {

			type::Type* resultingType = NULL;

			if (IS(TK_IDENTIFIER)) {
				std::string typeName(CAST(Identifier)->name());
				accept();

				if (typeName == "bool") resultingType = type::BasicType::Bool;
				else if (typeName == "int") resultingType = type::BasicType::Int;
				else if (typeName == "real") resultingType = type::BasicType::Real;
				else if (typeName == "void") resultingType = type::BasicType::Void;
				else if (typeName == "Any") resultingType = type::BasicType::Any;
				else if (type::Type* t = _currentScope->getType(typeName)) {
					if (type::getIf<type::InterfaceType*>(t)) {
						ERROR("Missing '@' symbol before interface identifier")
					}
					resultingType = t;
				} else {
					ClassNode* clss = new ClassNode(0, 0, typeName, NULL);
					resultingType = clss->typeClass();
					_currentScope->setUnimplemented(typeName, resultingType, clss);
				}

				while (_currentToken != _lastToken && accept(DOT)) {
					if (type::ClassType* ct = type::getIf<type::ClassType*>(resultingType)) {

						std::string inner = expectIdentifier("Expected identifier");
						size_t defIndex = ct->getClass()->getDefinitionIndex(inner);

						if (defIndex != std::string::npos) {
							Definition* def = ct->getClass()->defs()[defIndex]->definition();

							if (def != NULL && def->type() == type::BasicType::Class) {
								resultingType = ((ClassNode*)def->value())->typeClass();
							} else {
								ERROR("Member '" + inner + "' of class " + resultingType->toString() + " is not a class")
							}
						} else {
							ERROR("No class named '" + inner + "' in class " + resultingType->toString())
						}

					} else {
						ERROR("Cannot access content of non-class " + resultingType->toString())
					}
				}

				if (_currentToken != _lastToken && accept(EXTENDS)) {
					if (type::ClassType* ct = type::getIf<type::ClassType*>(resultingType)) {

						type::Type* t = parseTypeDeclaration();

						if (type::InterfaceType* it = type::getIf<type::InterfaceType*>(t)) {
							resultingType = new type::ComposedType(ct, {it});
						} else if (type::ComposedType* cpt = type::getIf<type::ComposedType*>(t)) {
							cpt->setClass(ct);
							resultingType = cpt;
						}
					} else {
						ERROR("Composed type should be of the form : 'Class extends @Interface1 & @Interface2 & ...'")
					}
				}

			} else if (accept(PAREN_O)) {
				std::vector<type::Type*> types;
				if (forAbstractDeclaration)
					types.push_back(forAbstractDeclaration);

				if (!accept(PAREN_C)) {
					do {
						types.push_back(parseTypeDeclaration());
					} while (accept(COMMA));
					if (!accept(PAREN_C)) {
						ERROR("Expected ')'")
					}
				}
				if (!accept(TINY_RIGHT_ARROW))
					resultingType = new type::TupleType(types, createTupleTypeRefMapTupleNode(types));
				else
					resultingType = new type::FunctionType(types, parseTypeDeclaration());
			} else if (accept(AT_SIGN)) {
				std::string typeName(expectIdentifier("Expected interface name"));
				if (type::Type* t = _currentScope->getType(typeName)) {
					if (!getIf<type::InterfaceType*>(t))
						ERROR("Type '" + t->toString() + "' is not an interface")

						resultingType = t;
				} else {
					InterfaceNode* interface = new InterfaceNode(0, 0, typeName, _interfaceCount++);
					resultingType = interface->typeInterface();
					_currentScope->setUnimplementedType(typeName, resultingType);
				}

				if (_currentToken != _lastToken && accept(B_AND)) {
					std::vector<type::InterfaceType*> interfaces;
					interfaces.push_back((type::InterfaceType*)resultingType);
					do {
						if (type::InterfaceType* it = type::getIf<type::InterfaceType*>(parseTypeDeclaration()))
							interfaces.push_back(it);
						else
							ERROR("Expected interface identifier")
						} while (accept(B_AND));

					resultingType = new type::ComposedType(NULL, interfaces);
				}
			} else {
				ERROR("Unexpected token : '" + (*_currentToken)->toString() + "'")
			}

			while (_currentToken != _lastToken && accept(BRACKET_O)) {
				TSINT nbItems = -1;
				if (IS(TK_INTEGER)) {
					nbItems = CAST(tok::IntegerConstToken)->value();
					accept();
				}
				if (!accept(BRACKET_C)) {
					ERROR("Expected ']'");
				}
				resultingType = new type::ArrayType(resultingType, nbItems);
			}

			return resultingType;
		}

		AbstractNode* Compiler::parseVariableDeclaration(size_t init_pos, size_t init_len) {
			BlockNode* multipleDeclarations = new BlockNode(init_pos, init_len, false);
			do {
				std::string name(expectIdentifier("Expected variable name"));
				LocalVariableNode* var = NULL;

				if (_currentScope->var(name)) {
					ERROR("Redefinition of local variable '" + name + "'")
				}

				if (IS(TK_OPERATOR) && (CAST(Operator)->opType() == EQUAL || CAST(Operator)->opType() == COLON)) {

					type::Type* t = accept(COLON) ? parseTypeDeclaration() : type::BasicType::NotYetDefined;
					var = new LocalVariableNode(CUR_POSLENGTH, name, makeVar(name, t));

					AbstractNode* exp = parseBinaryExp(var, 0);

					if (exp != var) // is we could parse something, e.g. there was an assigenement
						multipleDeclarations->addNode(exp);

				} else if (accept(PAREN_O) || (IS(TK_OPERATOR) && CAST(Operator)->opType() == BRACKET_O)) {

					type::Type* t = type::BasicType::NotYetDefined;
					var = new LocalVariableNode(CUR_POSLENGTH, name, makeVar(name, t));

					AbstractNode* exp = new AssignementNode(CUR_POSLENGTH, var, parseFunctionDeclaration(false));
					multipleDeclarations->addNode(exp);
				} else {
					ERROR("Expected type declaration or assignement")
				}
			} while (accept(COMMA));

			accept(SEMICOLON);

			return multipleDeclarations;
		}

		void Compiler::parseAbstractFieldDeclaration() {
			std::string name(expectIdentifier("Expected abstract field name"));

			accept(COLON);

			type::FunctionType* t = type::getIf<type::FunctionType*>(parseTypeDeclaration(_currentClass->typeClass()));
			if (!t) {
				ERROR("Expected function type");
				return;
			}
			t->setAbstract(true);
			_currentClass->setDef(new DefinitionReferenceNode(CUR_POSLENGTH, name, new Definition(name, 0, t, NULL)));
		}

		AbstractNode* Compiler::parseInclude() {
			if (!IS(TK_STRING)) {
				ERROR("Expected path to file to include")
			}

			std::string path = CAST(StringConstToken)->value();
			accept();

			char lastChar = path[path.size() - 1];
			if (lastChar != '/' || lastChar != '\\') {
				path += '/';
			}

			for (auto& p : _includePaths) {
				if (path == p) {
					return new PassNode(CUR_POSLENGTH);
				}
			}

			_includePaths.push_back(path);

			return new PassNode(CUR_POSLENGTH);
		}

		AbstractNode* Compiler::parseImport() {
			if (!IS(TK_STRING)) {
				ERROR("Expected path to file to include")
			}

			std::string path = CAST(StringConstToken)->value();
			accept();

			std::ifstream file;

			for (auto& p : _includePaths) {
				file.open(p + path);
				if (!file.good())
					file.close();
				else
					break;
			}

			if (!file.good()) {
				ERROR("File '" + path + "' does not exist")
			}

			std::string content;
			while (file.good())
				content += file.get();
			file.close();

			auto lastCur = _currentToken;
			auto lastLast = _lastToken;

			std::vector<AbstractToken*> toks(Tokenizer::tokenize(content));
			size_t p = toks[toks.size() - 1]->position();
			size_t l = toks[toks.size() - 1]->length();

			toks.push_back(new tok::IntegerConstToken(p, l, 0));

			_currentToken = toks.begin();
			_lastToken = toks.end() - 1;

			while (_currentToken != _lastToken) {
				parseStatement();
			}

			_currentToken = lastCur;
			_lastToken = lastLast;

			return new PassNode(CUR_POSLENGTH);
		}

		std::vector<nodes::Argument*> Compiler::parseArguments(tok::OPERATOR_TYPE closingOp) {
			std::vector<Argument*> args;
			if (accept(closingOp))
				return args;

			do {
				std::string name(expectIdentifier("Expected name of argument"));

				if (!accept(COLON)) {
					ERROR("Expected type declaration of " + name)
					return args;
				}

				type::Type* t = parseTypeDeclaration();

				args.push_back(new Argument(name, t));

			} while (accept(COMMA));

			if (!accept(closingOp))
				ERROR("Expected " + tok::Operator(0, 0, closingOp).toString() + " after arguments listing")

				return args;
		}

		std::vector<LocalVariableNode*> Compiler::parseCaptures() {
			std::vector<LocalVariableNode*> captures;
			if (accept(SUP))
				return captures;

			do {
				std::string name(expectIdentifier("Expected name of captured variable"));

				if (Variable* var = _currentScope->var(name))
					captures.push_back(new LocalVariableNode(CUR_POSLENGTH, name, var));
			} while (accept(COMMA));

			if (!accept(SUP))
				ERROR("Expected '>'")

				return captures;
		}

		std::vector<AbstractNode*> Compiler::parseCallArguments(tok::OPERATOR_TYPE closingOp) {
			std::vector<AbstractNode*> args;
			if (accept(closingOp))
				return args;

			do {
				args.push_back(parseExpression());
			} while (accept(COMMA));

			if (!accept(closingOp))
				ERROR("Expected '" + tok::Operator(0, 0, closingOp).toString() + "'")

				return args;
		}

		void Compiler::checkUnresolvedLinks(bool isError) {
			if (isError) {
				for (auto it = _unresolvedReferences.begin(); it != _unresolvedReferences.end(); ++it)
					CUSTOMERROR((*it)->pos(), (*it)->name().size(), "Undefined reference to symbol '" + (*it)->name() + "'")
					return;
			}

			for (auto it = _unresolvedReferences.begin(); it != _unresolvedReferences.end();) {
				std::string name((*it)->name());

				if (Definition* def = _currentScope->def(name)) {
					(*it)->setDefinition(def);
					it = _unresolvedReferences.erase(it);
				} else
					++it;
			}
		}

		size_t Compiler::createCurrentFunctionRefMapTupleNode() {
			_baseScope->setDef("@" + T_toString(_defNumber), _defNumber, NULL, new RefMapTupleNode(_currentFunctionLocalVariables));
			return _defNumber++;
		}

		size_t Compiler::createCurrentFunctionExTableNode() {
			_baseScope->setDef("!" + T_toString(_defNumber), _defNumber, NULL, _currentFunctionExTableNode);
			return _defNumber++;
		}

		size_t Compiler::createCurrentFunctionDebugInfoNode() {
			_baseScope->setDef("|" + T_toString(_defNumber), _defNumber, NULL, new FunctionDebugInfoNode(_currentFunctionLocalVariables));
			return _defNumber++;
		}

		size_t Compiler::createCurrentClassDebugInfoNode(ClassNode *clss) {
			_baseScope->setDef("|" + T_toString(_defNumber), _defNumber, NULL, new ClassDebugInfoNode(clss));
			return _defNumber++;
		}

		size_t Compiler::createTupleTypeRefMapTupleNode(const std::vector<type::Type*>& tuple) {
			_baseScope->setDef("@" + T_toString(_defNumber), _defNumber, NULL, new TupleTypeRefMapTupleNode(tuple));
			return _defNumber++;
		}

		void Compiler::adjustIfIsConstructor(AbstractNode* cstr, const std::string& name) {
			if (name != "_new_")
				return;

			FunctionNode* func = (FunctionNode*)cstr;
			BlockNode* block = new BlockNode(CUR_POSLENGTH, _debug);
			block->addNode(func->body());
			block->addNode(new LocalVariableNode(func->body()->pos(), func->body()->len(), "this", getThis()));

			func->setBody(block);
		}

		void Compiler::addDefsToConstantPool() {
			const std::map<std::string, Definition*>& defs(_currentScope->getDefs());
			_constantPool.resize(_defNumber);

			for (auto it = defs.begin(); it != defs.end(); ++it) {
				Definition* current = it->second;
				do {
					_constantPool[current->index()] = current;
				} while ((current = current->getNextOverloadedDefinition())); // while the definition is overloaded
			}
		}


		Variable* Compiler::makeStructVar(const std::string& name, type::StructType* t) {

			Variable* v = _currentScope->setStructVar(name, t);

			const std::vector<nodes::Argument*>& fields = t->fields();

			for (size_t i = 0; i < fields.size(); ++i) {
				//_currentFunctionLocalVariables.push_back(_currentScope->setVar(name+"."+fields[i]->name(), fields[i]->type()));
				makeVar(name + "." + fields[i]->name(), fields[i]->type());
			}

			return v;
		}

		Variable* Compiler::makeVar(const std::string& name, Variable* var) {
			if (var->type())
				if (type::StructType* st = type::getIf<type::StructType*>(var->type()))
					return makeStructVar(name, st);

			Variable* v = _currentScope->setVar(name, var);
			_currentFunctionLocalVariables.push_back({name, v});
			return v;
		}

		Variable* Compiler::makeVar(const std::string& name, type::Type* t) {
			if (t)
				if (type::StructType* st = type::getIf<type::StructType*>(t))
					return makeStructVar(name, st);

			Variable* v = _currentScope->setVar(name, t);
			_currentFunctionLocalVariables.push_back({name, v});
			return v;
		}

		cmplr::Variable* Compiler::getThis() {
			return new Variable(_currentClass->typeClass(), 0);
		}

		ClassNode* Compiler::addClass(const std::string& name) {
			ClassNode* clss = NULL;
			if (Definition* def = _baseScope->def(name))
				if ((clss = getIf<ClassNode*>(def->value())))
					return clss;

			clss = new ClassNode(0, 0, name, NULL);
			clss->typeClass()->setClassDefIndex(_defNumber);
			_baseScope->setDef(name, _defNumber++, clss->type(), clss);
			_baseScope->setType(clss->name(), clss->typeClass());
			return clss;
		}

		ClassNode* Compiler::findClass(const std::string& name) {
			if (Definition* def = _currentScope->def(name))
				if (ClassNode* clss = getIf<ClassNode*>(def->value()))
					return clss;

			for (size_t i = 0; i < _constantPool.size(); ++i) {
				if (ClassNode* clss = getIf<ClassNode*>(_constantPool[i]->value())) {
					if (clss->name() == name) {
						return clss;
					}
				}
			}
			return NULL;
		}

		InterfaceNode* Compiler::addInterface(const std::string& name) {
			type::InterfaceType* interfaceType = NULL;
			if (type::Type* t = _baseScope->getType(name)) {
				if (interfaceType == type::getIf<type::InterfaceType*>(t)) {
					return interfaceType->getInterface();
				}
			} else {
				InterfaceNode* n = new InterfaceNode(0, 0, name, _interfaceCount++);
				_baseScope->setType(name, n->typeInterface());
				return n;
			}

			return NULL;
		}

		InterfaceNode* Compiler::findInterface(const std::string& name) {
			if (type::Type* t = _currentScope->getType(name)) {
				if (type::InterfaceType* it = type::getIf<type::InterfaceType*>(t)) {
					return it->getInterface();
				}
			}

			return NULL;
		}

		SymbolLocation Compiler::getSymbolLocation(const std::string& name) {

			// Search first in the current scope if still compiling

			if (Definition* def = _currentScope->def(name))
				return SymbolLocation(def->index(), SymbolLocation::Pool::K_POOL);

			// if not found, search in the constant pool

			for (size_t i = 0; i < _constantPool.size(); ++i) {
				if (_constantPool[i]->name() == name) {
					return SymbolLocation(i, SymbolLocation::Pool::K_POOL);
				}
			}

			// if still not found, search in the native pool

			auto it = _usrDatas.find(name);
			if (it != _usrDatas.end())
				return SymbolLocation(it->second._index, SymbolLocation::Pool::N_POOL);

			// not found

			return SymbolLocation(0, SymbolLocation::Pool::NOT_FOUND);
		}

		NativeDefinitionReferenceNode* Compiler::setUserData(const std::string& name, const UserDataInfo& info) {
			auto it = _usrDatas.find(name);
			if (it == _usrDatas.end()) {

				type::Type* dataType = info._type;

				if (info._type == NULL) { // it means that the type was given by a string instead of directly, so just parse it
					auto lastCur = _currentToken;
					auto lastLast = _lastToken;

					std::vector<AbstractToken*> toks(Tokenizer::tokenize(info._typestr));

					_currentToken = toks.begin();
					_lastToken = toks.end();

					dataType = parseTypeDeclaration();

					_currentToken = lastCur;
					_lastToken = lastLast;
				}

				if (type::FunctionType* ft = type::getIf<type::FunctionType*>(dataType))
					ft->setNative(true);

				UserData usrD(_usrDatas.size(), info._data, dataType);
				_usrDatas[name] = usrD;
				return new NativeDefinitionReferenceNode(0, 0, name, usrD);
			}
			return NULL;
		}

		void Compiler::setClassData(nodes::ClassNode* clss, const std::string& name, const UserDataInfo& info) {
			type::Type* dataType = info._type;

			if (dataType == NULL) { // it means that the type was given by a string instead of directly, so just parse it
				dataType = typeFromString(info._typestr);
			}

			if (type::FunctionType* ft = type::getIf<type::FunctionType*>(dataType))
				ft->setNative(true);

			UserData usrD(_usrDatas.size(), info._data, dataType);
			NativeDefinitionReferenceNode* ref = new NativeDefinitionReferenceNode(0, 0, name, usrD);
			clss->setDef(ref);

			_usrDatas[clss->name() + "." + name] = ref->getUserData();
		}

		void Compiler::updateClassData(nodes::ClassNode* clss, const std::string& name, objects::NativeData* data) {
			size_t pos = clss->getDefinitionIndex(name);
			if (pos != std::string::npos) {
				UserData& dat = ((NativeDefinitionReferenceNode*)clss->defs()[pos])->getUserData();
				dat._data = data;
				_usrDatas[clss->name() + "." + name] = dat;
			}
		}

		void Compiler::setInterfaceData(InterfaceNode *interface, const std::string &name, const UserDataInfo &info) {
			type::Type* dataType = info._type;

			if (dataType == NULL) { // it means that the type was given by a string instead of directly, so just parse it
				dataType = typeFromString(info._typestr);
			}

			if (type::FunctionType* ft = type::getIf<type::FunctionType*>(dataType))
				ft->setNative(true);

			UserData usrD(_usrDatas.size(), info._data, dataType);
			NativeDefinitionReferenceNode* ref = new NativeDefinitionReferenceNode(0, 0, name, usrD);

			interface->addDefinition(name, ref);

			_usrDatas[interface->name() + "." + name] = ref->getUserData();
		}

		void Compiler::updateInterfaceData(InterfaceNode *interface, const std::string &name, objects::NativeData *data) {
			ImplementationLocation pos = interface->getDefinitionIndex(name);
			if (!pos.Empty && pos.interface == interface) {
				UserData& dat = ((NativeDefinitionReferenceNode*)interface->defs()[pos.index].def)->getUserData();
				dat._data = data;
				_usrDatas[interface->name() + "." + name] = dat;
			}
		}

		void Compiler::setAbstractInterfaceData(InterfaceNode *interface, const std::string &name, const UserDataInfo &info) {
			type::Type* dataType = info._type;

			if (dataType == NULL) { // it means that the type was given by a string instead of directly, so just parse it
				dataType = typeFromString(info._typestr);
			}

			if (type::FunctionType* ft = type::getIf<type::FunctionType*>(dataType)) {
				ft->setNative(true);
				ft->setAbstract(true);
			}

			interface->addDefinition(name, new Definition(name, 0, dataType, NULL));
		}

		type::Type *Compiler::typeFromString(const std::string &tstr) {
			auto lastCur = _currentToken;
			auto lastLast = _lastToken;

			std::vector<AbstractToken*> toks(Tokenizer::tokenize(tstr));

			_currentToken = toks.begin();
			_lastToken = toks.end();

			type::Type* t = parseTypeDeclaration();

			_currentToken = lastCur;
			_lastToken = lastLast;

			return t;
		}

		NativeDefinitionReferenceNode* Compiler::getNativeReference(const std::string& name, size_t init_pos, size_t init_len) {
			auto it = _usrDatas.find(name);
			if (it == _usrDatas.end())
				return NULL;
			else
				return new NativeDefinitionReferenceNode(init_pos, init_len, name, it->second);
		}
	}
}
