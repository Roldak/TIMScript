//
//  Keyword.h
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#ifndef __TimScript_5__Keyword__
#define __TimScript_5__Keyword__

#include <iostream>
#include <unordered_map>

#include "AbstractToken.h"

namespace ts {
    namespace tok{

        enum KEYWORD_TYPE{
            BEGIN, LET, DEF, TYPEDEF, PRINT, IF, ELSE, WHILE, FOR, FOREACH, IN, CLASS, INTERFACE, STATIC, EXTENDS, ABSTRACT,
            INCLUDE, IMPORT, RETURN, SCOPE, ASYNC, MATCH, CASE, DEFAULT, TRY, KTHROW, CATCH, STRUCT};

        class Keyword : public AbstractToken{
        public:
            Keyword(size_t pos, size_t len, KEYWORD_TYPE type) : AbstractToken(pos, len), _kwType(type) {}
            virtual ~Keyword() {}
            
            inline KEYWORD_TYPE kwType() {return _kwType;}

            virtual TOKEN_TYPE getType(){return TK_KEYWORD;}
            virtual std::string toString();

            static Keyword* getKeyWordFromString(size_t pos, size_t len, const std::string& str);
            static bool isValidKeyWord(const std::string& str);
            static std::unordered_map<std::string, KEYWORD_TYPE> keywords;
            
        private:
            KEYWORD_TYPE _kwType;
        };

    }
}

#endif /* defined(__TimScript_5__Keyword__) */
