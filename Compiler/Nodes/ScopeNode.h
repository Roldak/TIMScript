//
//  ScopeNode.h
//  TIMScript
//
//  Created by Romain Beguet on 31.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ScopeNode__
#define __TIMScript__ScopeNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts{
    namespace nodes{
        
        class ScopeNode : public AbstractNode{
        public:
            
            ScopeNode(size_t pos, size_t length, AbstractNode* node) : AbstractNode(pos, length), _content(node) {}
            virtual ~ScopeNode(){delete _content;}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
        private:
            
            AbstractNode* _content;
        };
        
    }
}

#endif /* defined(__TIMScript__ScopeNode__) */
