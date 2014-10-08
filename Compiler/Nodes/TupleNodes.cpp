//
//  TupleNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 06.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TupleNodes.h"
#include "CastNode.h"
#include "CompilationExceptions.h"
#include <istream>

namespace ts{
    namespace nodes{
        
        NewTupleNode::NewTupleNode(size_t pos, size_t length, type::TupleType* type) : AbstractNode(pos, length){
            _cachedType=type;
        }
        
        void NewTupleNode::semanticTraverse(){
            
        }
        
        void NewTupleNode::pushBytecode(std::vector<TSINSTR>& program){
            program.push_back(NEW_TUPLE);
            program.push_back(((type::TupleType*)_cachedType)->types().size());
            program.push_back(((type::TupleType*)_cachedType)->refMapIndex());
        }
        
        std::string NewTupleNode::toString(){
            return "new "+_cachedType->toString();
        }
        
        // MAKE TUPLE NODE
        
        void MakeTupleNode::semanticTraverse(){
            if (_tupleType->types().size()!=_nodes.size()) {
                throw exception::NumberOfElementsMismatchInitializerList(this, _nodes.size(), (size_t)_tupleType->types().size());
            }
            for (size_t i=0; i<_nodes.size(); ++i) {
                if (_nodes[i]->type()!=_tupleType->types()[i])
                    _nodes[i]=new CastNode(_nodes[i], _tupleType->types()[i]);
            }
            _cachedType=_tupleType;
        }
        
        void MakeTupleNode::pushBytecode(std::vector<TSINSTR>& program){
            for (size_t i=0; i<_nodes.size(); ++i)
                _nodes[i]->pushBytecode(program);
            
            program.push_back(MK_TUPLE);
            program.push_back(_nodes.size());
            program.push_back(_tupleType->refMapIndex());
        }
        
        std::string MakeTupleNode::toString(){
            std::string toRet="[";
            for (size_t i=0; i<_nodes.size(); ++i) {
                toRet+=_nodes[i]->toString();
                if (i!=_nodes.size()-1) {
                    toRet+=", ";
                }
            }
            return toRet+"]";
        }
        
        // TUPLE ELEMENT ACCESS NODE
        
        void TupleElementAccessNode::semanticTraverse(){
            type::TupleType* t=(type::TupleType*)_left->type();
            size_t pos_=_identifier.find('_');
            
            if (pos_!=std::string::npos) {
                std::string index=_identifier.substr(pos_+1, _identifier.size()-pos_-1);
                std::istringstream iss(index);
                iss>>_elementIndex;
                if (_elementIndex>0 && _elementIndex<=t->types().size()){
                    _elementIndex--;
                    _cachedType=t->types()[_elementIndex];
                    return;
                }
            }
            
            throw exception::UnknownTupleElement(this, t, _identifier);
        }
        
        void TupleElementAccessNode::pushBytecode(std::vector<TSINSTR>& program){
            _left->pushBytecode(program);
            program.push_back(GET_TE);
            program.push_back(_elementIndex);
        }
        
        void TupleElementAccessNode::pushAssignementBytecode(std::vector<TSINSTR>& program){
            _left->pushBytecode(program);
            if(type::BasicType::isBasicType(_cachedType))
                program.push_back(SET_TE);
            else
                program.push_back(SET_TE_R);
            program.push_back(_elementIndex);
        }
        
        void TupleElementAccessNode::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program){
            
        }
        
        void TupleElementAccessNode::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program){
         
        }
        
        void TupleElementAccessNode::setInferedType(type::Type* t){
            
        }
        
        std::string TupleElementAccessNode::toString(){
            return _left->toString()+"."+_identifier;
        }
        
    }
}