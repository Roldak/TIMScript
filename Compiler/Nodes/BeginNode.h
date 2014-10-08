//
//  BeginNode.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__BeginNode__
#define __TIMScript__BeginNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts{
    namespace nodes{
        
        class BeginNode : public AbstractNode{
        public:
            
            BeginNode(size_t pos, size_t length, AbstractNode* node) : AbstractNode(pos, length), _content(node) {}
            virtual ~BeginNode(){delete _content;}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
        private:
            
            AbstractNode* _content;
        };
        
    }
}

#endif /* defined(__TIMScript__BeginNode__) */
