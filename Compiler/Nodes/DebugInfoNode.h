//
//  DebugInfoNode.h
//  TIMScript
//
//  Created by Romain Beguet on 29.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__DebugInfoNode__
#define __TIMScript__DebugInfoNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
    
    namespace cmplr{
        class Variable;
    }
    
    namespace nodes{

        class FunctionDebugInfoNode : public AbstractNode{
        public:
            
            FunctionDebugInfoNode(const std::vector<std::pair<std::string, cmplr::Variable*>>& vars);
            virtual ~FunctionDebugInfoNode() {}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            const std::vector<std::pair<std::string, cmplr::Variable*>> _vars;
            
        };
        

        class ClassDebugInfoNode : public AbstractNode{
        public:

            ClassDebugInfoNode(nodes::ClassNode* clss);
            virtual ~ClassDebugInfoNode() {}

            virtual void semanticTraverse();

            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();

        private:

            nodes::ClassNode* _class;

        };
    }
}

#endif /* defined(__TIMScript__DebugInfoNode__) */
