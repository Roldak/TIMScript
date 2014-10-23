//
//  Keyword.cpp
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#include "Keyword.h"

namespace ts {
	namespace tok {

		std::unordered_map<std::string, KEYWORD_TYPE> createKeywordsMap() {
			std::unordered_map<std::string, KEYWORD_TYPE> map;
			map["begin"] = BEGIN;
			map["let"] = LET;
			map["def"] = DEF;
			map["typedef"] = TYPEDEF;
			map["print"] = PRINT;
			map["if"] = IF;
			map["else"] = ELSE;
			map["while"] = WHILE;
			map["for"] = FOR;
			map["foreach"] = FOREACH;
			map["in"] = IN;
			map["class"] = CLASS;
			map["interface"] = INTERFACE;
			map["static"] = STATIC;
			map["extends"] = EXTENDS;
			map["abstract"] = ABSTRACT;
			map["include"] = INCLUDE;
			map["import"] = IMPORT;
			map["return"] = RETURN;
			map["scope"] = SCOPE;
			map["async"] = ASYNC;
			map["match"] = MATCH;
			map["case"] = CASE;
			map["default"] = DEFAULT;
			map["try"] = TRY;
			map["throw"] = KTHROW;
			map["catch"] = CATCH;
			map["struct"] = STRUCT;
			return map;
		}

		std::unordered_map<std::string, KEYWORD_TYPE> Keyword::keywords = createKeywordsMap();

		std::string Keyword::toString() {
			switch (_kwType) {
				case BEGIN:
					return "begin";
				case LET:
					return "let";
				case DEF:
					return "def";
				case TYPEDEF:
					return "typedef";
				case PRINT:
					return "print";
				case IF:
					return "if";
				case ELSE:
					return "else";
				case WHILE:
					return "while";
				case FOR:
					return "for";
				case FOREACH:
					return "foreach";
				case IN:
					return "in";
				case CLASS:
					return "class";
				case INTERFACE:
					return "interface";
				case STATIC:
					return "static";
				case EXTENDS:
					return "extends";
				case ABSTRACT:
					return "abstract";
				case INCLUDE:
					return "include";
				case IMPORT:
					return "import";
				case RETURN:
					return "return";
				case SCOPE:
					return "scope";
				case ASYNC:
					return "async";
				case MATCH:
					return "match";
				case CASE:
					return "case";
				case DEFAULT:
					return "default";
				case TRY:
					return "try";
				case KTHROW:
					return "throw";
				case CATCH:
					return "catch";
				case STRUCT:
					return "struct";
			}
			return "";
		}

		Keyword* Keyword::getKeyWordFromString(size_t pos, size_t len, const std::string& str) {
			if (Keyword::keywords.count(str))
				return new Keyword(pos, len, Keyword::keywords[str]);
			return NULL;
		}

		bool Keyword::isValidKeyWord(const std::string& str) {
			return Keyword::keywords.count(str);
		}

	}
}
