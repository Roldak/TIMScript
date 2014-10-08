//
//  AsyncNode.h
//  TIMScript
//
//  Created by Romain Beguet on 01.04.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__AsyncNode__
#define __TIMScript__AsyncNode__

#include <iostream>
#include "AbstractNode.h"

namespace ts{
    namespace nodes{
        
        class AsyncNode : public AbstractNode{
        public:
            
            AsyncNode(size_t pos, size_t length, AbstractNode* node, AbstractNode* threadClassRef)
                : AbstractNode(pos, length), _function(node), _finalNode(NULL), _threadClassReference(threadClassRef) {}
            
            virtual ~AsyncNode(){
                if (_finalNode)
                    delete _finalNode;
                else{
                    delete _function;
                    delete _threadClassReference;
                }
            }
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
        private:
            
            AbstractNode* _function;
            AbstractNode* _finalNode;
            AbstractNode* _threadClassReference;
        };
        
    }
}

#endif /* defined(__TIMScript__AsyncNode__) */
