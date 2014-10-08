//
//  CompilationExceptions.h
//  TIMScript
//
//  Created by Romain Beguet on 05.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__CompilationExceptions__
#define __TIMScript__CompilationExceptions__

#include <iostream>
#include "TSType.h"

namespace ts{
    namespace nodes{
        class AbstractNode;
    }
    
    namespace exception{
        
        class CompilationException : public std::exception{
        public:
            virtual const char* what();
            virtual void get()=0;
            
            inline size_t pos() const{return _pos;}
            inline size_t len() const{return _len;}
            
        protected:
            CompilationException(size_t pos, size_t len) : _pos(pos), _len(len) {}
            CompilationException(nodes::AbstractNode* n);
            
            size_t _pos;
            size_t _len;
            
            std::string _msg;
        };
        
        class ParsingError : public CompilationException{
        public:
            ParsingError(size_t pos, size_t len, std::string msg) : CompilationException(pos, len) {
                _msg=msg;
            }
            
            virtual void get();
        };
        
        class IllegalCast : public CompilationException{
        public:
            IllegalCast(nodes::AbstractNode* n, type::Type* found, type::Type* expected)
                : CompilationException(n), _found(found), _expected(expected) {}
            virtual void get();
        private:
            
            type::Type* _found;
            type::Type* _expected;
        };

        class ArgumentTypeMismatch : public CompilationException{
        public:
            ArgumentTypeMismatch(nodes::AbstractNode* n, size_t i, type::Type* found, type::Type* expected)
                : CompilationException(n), _index(i), _found(found), _expected(expected) {}
            virtual void get();
        private:
            
            size_t _index;
            type::Type* _found;
            type::Type* _expected;
        };
        
        class ArgumentNumberMismatch : public CompilationException{
        public:
            ArgumentNumberMismatch(nodes::AbstractNode* n, size_t found, size_t expected)
            : CompilationException(n), _found(found), _expected(expected) {}
            virtual void get();
        private:
            
            size_t _found;
            size_t _expected;
        };
        
        class NonVoidIfWithoutElsePart : public CompilationException{
        public:
            NonVoidIfWithoutElsePart(nodes::AbstractNode* n, type::Type* expType)
                : CompilationException(n), _expType(expType) {}
            virtual void get();
        private:
            
            type::Type* _expType;
        };
        
        class ThenAndElseStatementHaveNotTheSameReturnType : public CompilationException{
        public:
            ThenAndElseStatementHaveNotTheSameReturnType(nodes::AbstractNode* n, type::Type* thenType, type::Type* elseType)
                : CompilationException(n), _thenType(thenType), _elseType(elseType){}
            virtual void get();
        private:
            
            type::Type* _thenType;
            type::Type* _elseType;
        };
        
        class NotRedefinedAbstractFunction : public CompilationException{
        public:
            NotRedefinedAbstractFunction(nodes::AbstractNode* n, const std::string& className, const std::string& funcName)
                : CompilationException(n), _className(className), _funcName(funcName){}
            virtual void get();
        private:
            
            const std::string& _className;
            std::string _funcName;
        };
        
        class UnknownStaticMember : public CompilationException{
        public:
            UnknownStaticMember(nodes::AbstractNode* n, const std::string& className, const std::string& member)
                : CompilationException(n), _className(className), _member(member){}
            virtual void get();
        private:
            
            const std::string _className;
            std::string _member;
        };
        
        class UnknownMember : public CompilationException{
        public:
            UnknownMember(nodes::AbstractNode* n, const std::string& c_i, const std::string& name, const std::string& member)
                : CompilationException(n), _class_interface(c_i), _name(name), _member(member){}
            virtual void get();
        private:
            
            const std::string _class_interface;
            const std::string _name;
            std::string _member;
        };
        
        class ReferingMissingConstructor : public CompilationException{
        public:
            ReferingMissingConstructor(nodes::AbstractNode* n, const std::string& className)
                : CompilationException(n), _className(className){}
            virtual void get();
        private:
            
            const std::string& _className;
        };
        
        class NumberOfElementsMismatchInitializerList : public CompilationException{
        public:
            NumberOfElementsMismatchInitializerList(nodes::AbstractNode* n, size_t found, size_t expected)
                : CompilationException(n), _found(found), _expected(expected){}
            virtual void get();
        private:
            
            size_t _found;
            size_t _expected;
        };
        
        class UnknownTupleElement : public CompilationException{
        public:
            UnknownTupleElement(nodes::AbstractNode* n, type::Type* tupleType, const std::string& elem)
                : CompilationException(n), _tupleType(tupleType), _element(elem){}
            virtual void get();
        private:
            
            type::Type* _tupleType;
            std::string _element;
        };
        
        class InvalidDotOperation : public CompilationException{
        public:
            InvalidDotOperation(nodes::AbstractNode* n, type::Type* type, const std::string& elem)
                : CompilationException(n), _type(type), _element(elem){}
            virtual void get();
        private:
            
            type::Type* _type;
            std::string _element;
        };
        
        class CasesHaveDifferentTypes : public CompilationException{
        public:
            CasesHaveDifferentTypes(nodes::AbstractNode* n, type::Type* found, type::Type* expected)
                : CompilationException(n), _found(found), _expected(expected){}
            virtual void get();
        private:
            
            type::Type* _found;
            type::Type* _expected;
        };
        
        class MissingDefaultCase : public CompilationException{
        public:
            MissingDefaultCase(nodes::AbstractNode* n, type::Type* type)
                : CompilationException(n), _type(type){}
            virtual void get();
        private:
            
            type::Type* _type;
        };

        class InvalidOperandBinaryIs : public CompilationException{
        public:
            InvalidOperandBinaryIs(nodes::AbstractNode* n, type::Type* found, std::string expected, std::string left_right)
                : CompilationException(n), _found(found), _expected(expected), _left_right(left_right){}
            virtual void get();
        private:

            type::Type* _found;
            std::string _expected;
            std::string _left_right;
        };

        class InvalidThrowValue : public CompilationException{
        public:
            InvalidThrowValue(nodes::AbstractNode* n, type::Type* found)
                : CompilationException(n), _found(found){}
            virtual void get();
        private:

            type::Type* _found;
        };

        class UnsetStructureType : public CompilationException{
        public:
            UnsetStructureType(nodes::AbstractNode* n, type::Type* found)
                : CompilationException(n), _found(found){}
            virtual void get();
        private:

            type::Type* _found;
        };

        class UndeducableType : public CompilationException{
        public:
            UndeducableType(nodes::AbstractNode* n)
                : CompilationException(n), _node(n) {}
            virtual void get();
        private:

            nodes::AbstractNode* _node;
        };
    }
}

#endif /* defined(__TIMScript__CompilationExceptions__) */
