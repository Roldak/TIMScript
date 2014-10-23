//
//  Tokenizer.cpp
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#include "Tokenizer.h"
#include <sstream>

#define ADDTOKEN addToken(tokens, currentState, currentToken, pos);
#define ADDTOKEN_CLEAR ADDTOKEN currentToken="";
#define INSERT_SEMICOLON tokens.push_back(new Operator(SEMICOLON));

namespace ts {
	using namespace tok;

	std::vector<AbstractToken*> Tokenizer::tokenize(const std::string& source) {
		std::vector<AbstractToken*> tokens;

		TOKEN_TYPE currentState = TK_UNDEFINED;
		std::string currentToken;
		COMMENT_TYPE isCommentating = NONE;

		for (size_t pos = 0; pos < source.size(); ++pos) {
			char c = source[pos];
			CHAR_TYPE cType = getCharType(c);

			if (isCommentating != NONE) {
				if (isCommentating == SINGLE_LINE && c == '\n') {
					isCommentating = NONE;
					continue;
				} else if (isCommentating == MULTI_LINE && pos < source.size() - 1) {
					if (c == '*' && source[pos + 1] == '/') {
						++pos;
						isCommentating = NONE;
						continue;
					}
				}
				continue;
			} else {
				if (c == '/' && pos < source.size() - 1) {
					if (source[pos + 1] == '*') {
						isCommentating = MULTI_LINE;
						++pos;
						continue;
					} else if (source[pos + 1] == '/') {
						isCommentating = SINGLE_LINE;
						++pos;
						continue;
					}
				}
			}

			switch (cType) {
				case C_DIGIT:
				begDigit:
					switch (currentState) {
						case TK_INTEGER:
							if (c == '.')
								currentState = TK_REAL;
						case TK_REAL:
						case TK_STRING:
							currentToken += c;
							break;
						case TK_IDENTIFIER:
							if (c == '.') {
								ADDTOKEN_CLEAR
								currentState = TK_OPERATOR;
							}
							currentToken += c;
							break;
						case TK_OPERATOR:
							ADDTOKEN_CLEAR
							if (c == '.') {
								currentToken += c;
								break;
							}

						default:
							currentState = TK_INTEGER;
							goto begDigit;
					}

					break;

				case C_OPERATOR:

					switch (currentState) {
						case TK_INTEGER:
						case TK_REAL:
						case TK_IDENTIFIER:
							ADDTOKEN_CLEAR
							currentState = TK_OPERATOR;

						case TK_STRING:
							currentToken += c;
							break;
						case TK_OPERATOR:
							if (Operator::isValidOperator(currentToken + c))
								currentToken += c;
							else {
								ADDTOKEN
								currentToken = c;
							}
							break;
						default:
							currentState = TK_OPERATOR;
							currentToken += c;
							break;
					}

					break;

				case C_CHARACTER:
				begCharacter:
					switch (currentState) {
						case TK_INTEGER:
						case TK_REAL:
							ADDTOKEN_CLEAR
							currentState = TK_UNDEFINED;
							break;

						case TK_IDENTIFIER:
							if (c == ' ' || c == '\t' || c == '\n') {
								if (currentToken != "") ADDTOKEN_CLEAR
									currentState = TK_UNDEFINED;
								break;
							}
						case TK_STRING:
							if (c == '\"') {
								if (currentToken == "") {
									currentState = TK_STRING;
									if (pos < source.size() - 1) {
										if (source[pos + 1] == '\"') {
											pos++;
											ADDTOKEN_CLEAR
											currentState = TK_UNDEFINED;
										}
									}
								} else {
									ADDTOKEN_CLEAR
									currentState = TK_UNDEFINED;
								}
								break;
							} else if (c == '\'') {
								if (currentToken == "") {
									currentState = TK_INTEGER;
									if (pos < source.size() - 2) {

										std::ostringstream stream;
										if (source[++pos] == '\\')
											stream << (int)readEscapeChar(source[++pos]);
										else
											stream << (int)source[pos];

										currentToken += stream.str();

										if (source[pos + 1] == '\'') {
											pos++;
											ADDTOKEN_CLEAR
										} else {
											// error
										}
									}
								} else {
									// error
								}
								currentState = TK_UNDEFINED;
								break;
							}

							currentToken += c;
							break;
						case TK_OPERATOR:
							ADDTOKEN_CLEAR
						default:
							currentState = TK_IDENTIFIER;
							goto begCharacter;
					}

					break;

				case C_OTHER:
					switch (currentState) {
						case TK_STRING:
							if (c == '\\' && pos < source.size() - 1) {
								++pos;
								c = readEscapeChar(source[pos]);
							}
							currentToken += c;
							break;

						default:
							//Err::out<<"Unexpected token "<<c<<" after "<<currentToken<<Err::endl;
							break;
					}
					break;
			}

			if (pos == source.size() - 1)
				ADDTOKEN;
		}

		return tokens;
	}

	char Tokenizer::readEscapeChar(char c) {
		switch (c) {
			case 'n':
				return '\n';
			case 't':
				return '\t';
			case '\"':
				return '\"';
			case '\\':
				return '\\';
			default:
				return '\n';
		}
	}

	CHAR_TYPE Tokenizer::getCharType(char c) {
		if (isDigit(c))           return C_DIGIT;
		else if (isCharacter(c))  return C_CHARACTER;
		else if (isOperator(c))   return C_OPERATOR;
		else                     return C_OTHER;
	}

	bool Tokenizer::isOperator(char c) {
		return (c == '=' || c == '>' || c == '<' || c == ',' || c == '&' || c == '|' ||
				c == '+' || c == '-' || c == '%' || c == '*' || c == '/' || c == '^' ||
				c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
				c == '!' || c == ':' || c == ';' || c == '?' || c == '@');
	}

	bool Tokenizer::isDigit(char c) {
		return ((c >= 48 && c <= 57) || c == '.');
	}

	bool Tokenizer::isCharacter(char c) {
		return ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '\"' || c == '\'' || c == ' ' || c == '_' || c == '\n' || c == '\t');
	}

	bool Tokenizer::isEOS(Operator* o) {
		OPERATOR_TYPE t = o->opType();
		return t == PAREN_C;
	}

	void Tokenizer::addToken(std::vector<tok::AbstractToken*>& tokens, tok::TOKEN_TYPE type, const std::string& str, size_t pos) {
		size_t len = str.length();
		pos = pos - len;

		switch (type) {
			case TK_BOOLEAN: // CANNOT HAPPEN
			case TK_INTEGER: {
				TSINT num;
				std::istringstream(str) >> num;
				tokens.push_back(new IntegerConstToken(pos, len, num));
				return;
			}
			case TK_REAL: {
				TSREAL num;
				std::istringstream(str) >> num;
				tokens.push_back(new RealConstToken(pos, len, num));
				return;
			}
			case TK_STRING:
				tokens.push_back(new StringConstToken(pos, len, str));
				return;
			case TK_IDENTIFIER:
				if (Keyword::isValidKeyWord(str))
					tokens.push_back(Keyword::getKeyWordFromString(pos, len, str));
				else if (str == "true")
					tokens.push_back(new BooleanConstToken(pos, len, 1));
				else if (str == "false")
					tokens.push_back(new BooleanConstToken(pos, len, 0));
				else if (str == "not")
					tokens.push_back(new Operator(pos, len, NOT));
				else if (str == "or")
					tokens.push_back(new Operator(pos, len, OR));
				else if (str == "and")
					tokens.push_back(new Operator(pos, len, AND));
				else if (str == "is")
					tokens.push_back(new Operator(pos, len, ISTYPE));
				else if (str == "new")
					tokens.push_back(new Operator(pos, len, tok::NEW));
				else if (str == "scoped")
					tokens.push_back(new Operator(pos, len, tok::SCOPED));
				else
					tokens.push_back(new Identifier(pos, len, str));
				return;
			case TK_OPERATOR:
				tokens.push_back(Operator::getOperatorFromString(pos, len, str));
				return;
			case TK_KEYWORD:
				tokens.push_back(Keyword::getKeyWordFromString(pos, len, str));
			case TK_UNDEFINED:
				return;
		}
	}

}
