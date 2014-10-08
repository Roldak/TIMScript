//
//  PrintNode.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__PrintNode__
#define __TIMScript__PrintNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
    namespace nodes{
        
        class PrintNode : public AbstractNode{
        public:
            
            PrintNode(size_t pos, size_t length, const std::vector<AbstractNode*>& n) : AbstractNode(pos, length), _nodes(n) {}
            virtual ~PrintNode() {
                for(AbstractNode* n : _nodes)
                    delete n;
            }
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            std::vector<AbstractNode*> _nodes;
            
        };
        
    }
}

#endif /* defined(__TIMScript__PrintNode__) */
