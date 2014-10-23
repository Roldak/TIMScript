//
//  Compiler.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Compiler__
#define __TIMScript__Compiler__

#include <iostream>
#include "Tokenizer.h"
#include "AbstractNode.h"
#include "DefinitionReferenceNode.h"
#include "FunctionNode.h"
#include "ExceptionNodes.h"
#include "Scope.h"
#include "ScopeNode.h"
#include "TSProgram.h"

namespace ts {

	namespace cmplr {

		class Compiler {
		public:

			Compiler(std::string p = "", bool debugMode = false);

			Program* compile(const std::string& source);

			bool inDebugMode() {
				return _debug;
			}

			nodes::NativeDefinitionReferenceNode* setUserData(const std::string& name, const UserDataInfo& info);
			nodes::NativeDefinitionReferenceNode* getNativeReference(const std::string& name, size_t init_pos = 0, size_t init_len = 0);

			nodes::ClassNode* addClass(const std::string& name);
			nodes::ClassNode* findClass(const std::string& name);
			nodes::InterfaceNode* addInterface(const std::string& name);
			nodes::InterfaceNode* findInterface(const std::string& name);

			SymbolLocation getSymbolLocation(const std::string& name);

			void setClassData(nodes::ClassNode* clss, const std::string& name, const UserDataInfo& info);
			void updateClassData(nodes::ClassNode* clss, const std::string& name, objects::NativeData* data);

			void setInterfaceData(nodes::InterfaceNode* interface, const std::string& name, const UserDataInfo& info);
			void updateInterfaceData(nodes::InterfaceNode* interface, const std::string& name, objects::NativeData* data);
			void setAbstractInterfaceData(nodes::InterfaceNode* interface, const std::string& name, const UserDataInfo& info);

			type::Type* typeFromString(const std::string& tstr);

		private:

			nodes::AbstractNode* parse(std::vector<tok::AbstractToken*> tokens);

			std::string getWorkingDirectory(std::string filePath);
			Procedure* getProc(nodes::AbstractNode* exp, const std::string& name, bool forConstantPool = false);

			bool isMoreOrElse(const std::string& msg);
			void accept();
			bool accept(tok::TOKEN_TYPE t);
			bool accept(tok::OPERATOR_TYPE t);
			bool accept(tok::KEYWORD_TYPE t);
			std::string expectIdentifier(const std::string& msg);

			void startScope();
			void startScope(const std::vector<nodes::LocalVariableNode*>& captures, const std::vector<nodes::Argument*>& args);
			void endScope();

			nodes::AbstractNode* parseStatement();
			nodes::AbstractNode* parseBlock();
			nodes::AbstractNode* parseExpression();
			nodes::AbstractNode* parseBinaryExp(nodes::AbstractNode* left, int precedence);
			nodes::AbstractNode* parseSpecialBinaryExp(nodes::AbstractNode* left, tok::OPERATOR_TYPE op);
			nodes::AbstractNode* makeBinaryExp(nodes::AbstractNode* l, tok::OPERATOR_TYPE op, nodes::AbstractNode* r);
			nodes::AbstractNode* parseUnaryExp();
			nodes::AbstractNode* parsePrimary();
			nodes::AbstractNode* parseIdentifier();
			nodes::AbstractNode* parsePrimaryOperator();
			nodes::AbstractNode* parsePrimaryKeyword();
			nodes::AbstractNode* parseFunctionDeclaration(bool asMember = true, nodes::InterfaceNode* interface = NULL);
			nodes::AbstractNode* parseClassDeclaration(bool isAbstract);
			nodes::AbstractNode* parseInterfaceDeclaration();
			nodes::AbstractNode* parseStructureDeclaration();
			nodes::AbstractNode* parseStringConstant();

			// Statement nodes

			nodes::AbstractNode* parsePrintNode(size_t init_pos, size_t init_len);
			nodes::AbstractNode* parseIfNode(size_t init_pos, size_t init_len);
			nodes::AbstractNode* parseWhileNode(size_t init_pos, size_t init_len);
			nodes::AbstractNode* parseForNode(size_t init_pos, size_t init_len);
			nodes::AbstractNode* parseForeachNode(size_t init_pos, size_t init_len);
			nodes::AbstractNode* parseDefinition(nodes::InterfaceNode* interface = NULL);
			nodes::AbstractNode* parseTypedef();
			nodes::AbstractNode* parseAsyncNode();
			nodes::AbstractNode* parseMatchNode();
			nodes::AbstractNode* parseTryCatchNode();

			// Utility

			type::Type* parseTypeDeclaration(type::Type* forAbstractDeclaration = NULL);
			nodes::AbstractNode* parseVariableDeclaration(size_t init_pos, size_t init_len);
			nodes::AbstractNode* parseInclude();
			nodes::AbstractNode* parseImport();

			void parseAbstractFieldDeclaration();
			std::vector<nodes::Argument*> parseArguments(tok::OPERATOR_TYPE closingOp = tok::PAREN_C);
			std::vector<nodes::LocalVariableNode*> parseCaptures();
			std::vector<nodes::AbstractNode*> parseCallArguments(tok::OPERATOR_TYPE closingOp = tok::PAREN_C);

			nodes::AbstractNode* parseFunctionDeclarationContent(const std::vector<nodes::LocalVariableNode*>& captures,
					const std::vector<nodes::Argument*>& args,
					type::Type* returnType);

			void adjustIfIsConstructor(nodes::AbstractNode* cstr, const std::string& name);

			void addDefsToConstantPool();
			void checkUnresolvedLinks(bool isError = false);

			size_t createCurrentFunctionRefMapTupleNode();
			size_t createCurrentFunctionExTableNode();
			size_t createCurrentFunctionDebugInfoNode();
			size_t createCurrentClassDebugInfoNode(nodes::ClassNode* clss);
			size_t createTupleTypeRefMapTupleNode(const std::vector<type::Type*>& tuple);

			cmplr::Variable* makeStructVar(const std::string& name, type::StructType* t);
			cmplr::Variable* makeVar(const std::string& name, cmplr::Variable* var);
			cmplr::Variable* makeVar(const std::string& name, type::Type* t);

			cmplr::Variable* getThis();

			///////////////////
			// Attributes
			///////////////////

			bool _debug;

			std::vector<tok::AbstractToken*>::iterator _currentToken;
			std::vector<tok::AbstractToken*>::iterator _lastToken;
			nodes::ClassNode* _currentClass;

			// definitions
			size_t _defNumber;
			size_t _interfaceCount;
			std::vector<cmplr::Definition*> _constantPool;

			// unresolved references

			std::vector<nodes::DefinitionReferenceNode*> _unresolvedReferences;

			// local variables<->references maps (the gc needs to know at any given point which of the locals are references)
			std::vector<std::pair<std::string, cmplr::Variable*>> _currentFunctionLocalVariables;

			// exception table of the current function
			nodes::ExTableNode* _currentFunctionExTableNode;

			// scope
			Scope* _baseScope;
			Scope* _currentScope;
			nodes::ScopeNode* _currentScopeNode;

			// user datas

			std::map<std::string, UserData> _usrDatas;

			// includes paths

			std::vector<std::string> _includePaths;
		};
	}
}

#endif /* defined(__TIMScript__Compiler__) */
