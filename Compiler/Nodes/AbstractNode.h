//
//  AbstractNode.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__AbstractNode__
#define __TIMScript__AbstractNode__

#include <iostream>
#include <vector>
#include "Utils.h"
#include "Bytecode.h"
#include "TSType.h"

namespace ts{
    namespace nodes{
        
        enum NODE_TYPE{
            N_UNDEFINED, N_PASS, N_ASSIGNABLE, N_NEW_INSTANCE, N_NEW_ARRAY, N_NEW_TUPLE, N_DOT_OPERATION, N_VIRTUAL_ACCESS,
            N_IMPLEMENTATION_ACCESS, N_DEFINITION_REFERENCE, N_CLASS, N_INTERFACE, N_INITIALIZERLIST, N_LOCAL_VARIABLE_NODE,
            N_ATTRIBUTE_ACCESS, N_FIELD_ACCESS, N_NULL_REF};
        
        class AbstractNode{
        public:
            
            AbstractNode(size_t pos, size_t length) : _pos(pos), _length(length) {}
            virtual ~AbstractNode() {}
            
            inline type::Type* type(){return _cachedType;}
            inline size_t pos(){return _pos;}
            inline size_t len(){return _length;}
            
            virtual void semanticTraverse()=0;
            
            virtual void pushBytecode(std::vector<TSINSTR>& program)=0;
            virtual std::string toString()=0;
            
            virtual NODE_TYPE getNodeType(){return N_UNDEFINED;}
            
        protected:
            
            type::Type* _cachedType;
            size_t _pos;
            size_t _length;
        };
    }
}

#endif /* defined(__TIMScript__AbstractNode__) */
