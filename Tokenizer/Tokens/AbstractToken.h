//
//  Token.h
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#ifndef __TimScript_5__Token__
#define __TimScript_5__Token__

#include <iostream>

namespace ts {
    namespace tok{
        enum TOKEN_TYPE{
            TK_KEYWORD, TK_OPERATOR, TK_IDENTIFIER, TK_BOOLEAN, TK_INTEGER, TK_REAL, TK_STRING, TK_UNDEFINED
        };

        class AbstractToken{
        public:
            AbstractToken(size_t pos, size_t len) : _position(pos), _length(len) {}
            virtual ~AbstractToken(){}
            
            inline size_t position(){return _position;}
            inline size_t length(){return _length;}
            
            virtual TOKEN_TYPE getType()=0;
            virtual std::string toString()=0;
            
            static std::string typeToString(TOKEN_TYPE t);
            
        private:
            size_t _position;
            size_t _length;
        };
    }
}

#endif /* defined(__TimScript_5__Token__) */
