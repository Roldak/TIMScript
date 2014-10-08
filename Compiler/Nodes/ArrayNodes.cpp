//
//  ArrayNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 27.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ArrayNodes.h"
#include "CastNode.h"
#include "CompilationExceptions.h"

namespace ts{
    namespace nodes{
        
        // NEW ARRAY NODE
        
        NewArrayNode::NewArrayNode(size_t pos, size_t length, type::ArrayType* type) : AbstractNode(pos, length) {
            _cachedType=type;
        }
        
        void NewArrayNode::semanticTraverse(){

        }
        
        void NewArrayNode::pushBytecode(std::vector<TSINSTR>& program){
            if (((type::ArrayType*)_cachedType)->nbElements()!=-1) {
                program.push_back(PUSH_I);
                program.push_back((TSINSTR)((type::ArrayType*)_cachedType)->nbElements());
            }
            program.push_back(NEW_ARRAY);
            program.push_back(!type::BasicType::isBasicType(((type::ArrayType*)_cachedType)->elementsType()));
        }
        
        std::string NewArrayNode::toString(){
            return "new "+_cachedType->toString();
        }
        
        // MAKE ARRAY NODE
        
        void MakeArrayNode::semanticTraverse(){
            if (_arytype->nbElements()!=-1 && _arytype->nbElements()!=_nodes.size()) {
                throw exception::NumberOfElementsMismatchInitializerList(this, _nodes.size(), (size_t)_arytype->nbElements());
            }
            for (size_t i=0; i<_nodes.size(); ++i) {
                if (_nodes[i]->type()!=_arytype->elementsType()){
                    _nodes[i]=new CastNode(_nodes[i], _arytype->elementsType());
                    _nodes[i]->semanticTraverse();
                }
            }
            _cachedType=_arytype;
        }
        
        void MakeArrayNode::pushBytecode(std::vector<TSINSTR>& program){
            for (size_t i=0; i<_nodes.size(); ++i)
                _nodes[i]->pushBytecode(program);

            program.push_back(MK_ARRAY);
            program.push_back(_nodes.size());
            program.push_back(!type::BasicType::isBasicType(_arytype->elementsType()));
        }
        
        std::string MakeArrayNode::toString(){
            std::string toRet="[";
            for (size_t i=0; i<_nodes.size(); ++i) {
                toRet+=_nodes[i]->toString();
                if (i!=_nodes.size()-1) {
                    toRet+=", ";
                }
            }
            return toRet+"]";
        }
        
        
    }
}
