//
//  Token.cpp
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#include "AbstractToken.h"

namespace ts {
	namespace tok {

		std::string AbstractToken::typeToString(TOKEN_TYPE t) {
			switch (t) {
				case TK_BOOLEAN:
					return "Boolean";
				case TK_INTEGER:
					return "Integer";
				case TK_REAL:
					return "Real";
				case TK_STRING:
					return "String";
				case TK_IDENTIFIER:
					return "Identifier";
				case TK_KEYWORD:
					return "Keyword";
				case TK_OPERATOR:
					return "Operator";
				case TK_UNDEFINED:
					return "Undefined";
			}
			return "";
		}

	}
}
