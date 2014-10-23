//
//  Tokenizer.h
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#ifndef __TimScript_5__Tokenizer__
#define __TimScript_5__Tokenizer__

#include <iostream>
#include <vector>

#include "Utils.h"

#include "AbstractToken.h"
#include "ConstTokens.h"
#include "Operator.h"
#include "Keyword.h"
#include "Identifier.h"

namespace ts {

	enum CHAR_TYPE {C_OPERATOR, C_DIGIT, C_CHARACTER, C_OTHER};
	enum COMMENT_TYPE {NONE, SINGLE_LINE, MULTI_LINE};

	class Tokenizer {
	public:
		static std::vector<tok::AbstractToken*> tokenize(const std::string& source);

		static CHAR_TYPE getCharType(char c);
		static bool isOperator(char c);
		static bool isDigit(char c);
		static bool isCharacter(char c);
		static bool isEOS(tok::Operator* o);

	private:

		static void addToken(std::vector<tok::AbstractToken*>& tokens,
							 tok::TOKEN_TYPE type,
							 const std::string& str,
							 size_t pos);

		static char readEscapeChar(char c);
	};
}

#endif /* defined(__TimScript_5__Tokenizer__) */
