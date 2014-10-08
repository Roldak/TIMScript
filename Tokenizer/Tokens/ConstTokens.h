//
//  BasicTokens.h
//  TimScript 5
//
//  Created by Romain Beguet on 06.07.13.
//  Copyright (c) 2013 none. All rights reserved.
//

#ifndef __TimScript_5__BasicTokens__
#define __TimScript_5__BasicTokens__

#include <iostream>
#include <sstream>

#include "AbstractToken.h"
#include "Utils.h"

namespace ts {
    namespace tok{
        
        class BooleanConstToken : public AbstractToken{
        public:
            
            BooleanConstToken(size_t pos, size_t len, TSBOOL v) : AbstractToken(pos, len), _value(v) {}
            virtual ~BooleanConstToken() {}
            
            inline TSBOOL value(){return _value;}
            
            virtual TOKEN_TYPE getType(){return TK_BOOLEAN;}
            virtual std::string toString(){return T_toString(_value);}
            
        private:
            
            TSBOOL _value;
        };
        
        class IntegerConstToken : public AbstractToken{
        public:
            
            IntegerConstToken(size_t pos, size_t len, TSINT v) : AbstractToken(pos, len), _value(v) {}
            virtual ~IntegerConstToken() {}
            
            inline TSINT value(){return _value;}
            
            virtual TOKEN_TYPE getType(){return TK_INTEGER;}
            virtual std::string toString(){return T_toString(_value);}
            
        private:
            
            TSINT _value;
        };
        
        class RealConstToken : public AbstractToken{
        public:
            
            RealConstToken(size_t pos, size_t len, TSREAL v) : AbstractToken(pos, len), _value(v) {}
            virtual ~RealConstToken() {}
            
            inline TSREAL value(){return _value;}
            
            virtual TOKEN_TYPE getType(){return TK_REAL;}
            virtual std::string toString(){return T_toString(_value);}
            
        private:
            
            TSREAL _value;
        };
        
        class StringConstToken : public AbstractToken{
        public:
            
            StringConstToken(size_t pos, size_t len, const std::string& v) : AbstractToken(pos, len), _value(v) {}
            virtual ~StringConstToken() {}
            
            inline const std::string& value(){return _value;}
            
            virtual TOKEN_TYPE getType(){return TK_STRING;}
            virtual std::string toString(){return _value;}
            
        private:
            
            const std::string _value;
        };
    }
}

#endif /* defined(__TimScript_5__BasicTokens__) */
