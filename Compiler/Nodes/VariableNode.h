//
//  VariableNode.h
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__VariableNode__
#define __TIMScript__VariableNode__

#include <iostream>
#include "AssignableNode.h"
#include "Scope.h"

namespace ts{
    namespace nodes{

        class LocalVariableNode : public AssignableNode{
        public:
            LocalVariableNode(size_t pos, size_t length, const std::string& n, cmplr::Variable* v)
                : AssignableNode(pos, length), _name(n), _var(v) {}
            
            virtual ~LocalVariableNode() {}
            
            inline const std::string& name(){return _name;}
            inline cmplr::Variable* var(){return _var;}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void setInferedType(type::Type* t);
            virtual std::string toString();
            
            virtual NODE_TYPE getNodeType(){return N_LOCAL_VARIABLE_NODE;}
            
        private:
            
            const std::string _name;
            cmplr::Variable* _var;
        };
        
    }
}

#endif /* defined(__TIMScript__VariableNode__) */
