//
//  BlockNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "BlockNode.h"
#include "CastNode.h"

namespace ts{
    namespace nodes{
        
        BlockNode::~BlockNode(){
            for (size_t i=0; i<_content.size(); ++i)
                delete _content[i];
        }
        
        void BlockNode::semanticTraverse(){
            for (size_t i=0; i<_content.size(); ++i){
                _content[i]->semanticTraverse();
                
                if (i!=_content.size()-1)// last elem
                    if (_content[i]->type()!=type::BasicType::Void)
                        _content[i]=new CastNode(_content[i], type::BasicType::Void);
            }
            
            if (_content.size()==0)
                _cachedType=type::BasicType::Void;
            else
                _cachedType=_content[_content.size()-1]->type();
        }
        
        void BlockNode::pushBytecode(std::vector<TSINSTR>& program){
            for (size_t i=0; i<_content.size(); ++i){
                if(_generateDebugInstructions){
                    program.push_back(DBG_EOS);
                    program.push_back(_content[i]->pos());
                }
                _content[i]->pushBytecode(program);
            }
        }
        
        std::string BlockNode::toString(){
            std::string toReturn("{\n");
            for (size_t i=0; i<_content.size(); ++i)
                toReturn+=_content[i]->toString()+"\n";
            return toReturn+"}";
        }
        
    }
}
