//
//  ScopeNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 31.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ScopeNode.h"

namespace ts{
    namespace nodes{
        
        void ScopeNode::semanticTraverse(){
            _content->semanticTraverse();
            _cachedType=_content->type();
        }
        
        void ScopeNode::pushBytecode(std::vector<TSINSTR>& program){
            program.push_back(INIT_SCOPE);
            _content->pushBytecode(program);
            program.push_back(CLEAN_SCOPE);
        }
        
        std::string ScopeNode::toString(){
            return "scope"+_content->toString();
        }
        
    }
}