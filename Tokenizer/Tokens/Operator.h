//
//  Operator.h
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#ifndef __TimScript_5__Operator__
#define __TimScript_5__Operator__

#include <iostream>
#include <unordered_map>
#include <vector>

#include "AbstractToken.h"

namespace ts {
    namespace tok{
        
        enum OPERATOR_TYPE{ PLUS=0, MINUS, TIMES, SLASH, MODULO, POWER, EQUAL, NOT_EQUAL, EQUAL_EQUAL, SUP, INF, SUP_EQUAL, INF_EQUAL, ISTYPE,
                RIGHT_ARROW, TINY_RIGHT_ARROW, TERNARY, OR, AND, B_AND, B_OR, PLUS_PLUS, MINUS_MINUS, NOT, NEW, SCOPED, PAREN_O, PAREN_C,
                BRACKET_O, BRACKET_C, BRACE_O, BRACE_C, COMMA, DOT, COLON, DOUBLE_COLON, SEMICOLON, AT_SIGN, NUMBER_OF_OPERATORS};
        
        class Operator : public AbstractToken{
        public:
            Operator(size_t pos, size_t len, OPERATOR_TYPE type) : AbstractToken(pos, len), _opType(type) {}
            virtual ~Operator() {}
            
            inline OPERATOR_TYPE opType() {return _opType;}
            int getOperatorPrecedence();
            
            virtual TOKEN_TYPE getType(){return TK_OPERATOR;}
            virtual std::string toString();
            
            static Operator* getOperatorFromString(size_t pos, size_t len, const std::string& str);
            static bool isValidOperator(const std::string& str);
            
            static std::unordered_map<std::string, OPERATOR_TYPE> operators;
            static std::vector<int> operatorsPrecedence;
            
        private:
            OPERATOR_TYPE _opType;
        };
        
    }
}

#endif /* defined(__TimScript_5__Operator__) */
