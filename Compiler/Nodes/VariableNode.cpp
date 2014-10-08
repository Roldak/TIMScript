//
//  VariableNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "VariableNode.h"
#include "FunctionNode.h"

namespace ts {
    namespace nodes{
        
        void LocalVariableNode::semanticTraverse(){
            _cachedType=_var->type();
        }

        void pushSingleBytecode(type::Type* t, size_t& index, std::vector<TSINSTR>& program){
            if(type::StructType* st=type::getIf<type::StructType*>(t)){
                for(size_t i=0; i<st->fields().size(); ++i){
                    pushSingleBytecode(st->fields()[i]->type(), index, program);
                }
            }
            else{
                program.push_back(LOAD_N);
                program.push_back(index);
                ++index;
            }
        }
        
        void LocalVariableNode::pushBytecode(std::vector<TSINSTR>& program){
            size_t index=_var->index();
            pushSingleBytecode(_cachedType, index, program);
        }

        void pushSingleAssignementByteCode(type::Type* t, size_t& index, std::vector<TSINSTR>& program){
            if(type::StructType* st=type::getIf<type::StructType*>(t)){
                for(size_t i=0; i<st->fields().size(); ++i){
                    pushSingleAssignementByteCode(st->fields()[i]->type(), index, program);
                }
            }
            else{
                if (type::BasicType::isBasicType(t))
                    program.push_back(STORE_N);
                else
                    program.push_back(STORE_R);

                program.push_back(index);
                --index;
            }
        }
        
        void LocalVariableNode::pushAssignementBytecode(std::vector<TSINSTR>& program){
            size_t index=_var->index();
            if(type::StructType* st=type::getIf<type::StructType*>(_cachedType))
                index+=st->size()-1;

            pushSingleAssignementByteCode(_cachedType, index, program);
        }
        
        void LocalVariableNode::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program){
            if (_cachedType==type::BasicType::Int) {
                program.push_back(INC_I_1);
                program.push_back(_var->index());
            }
            else if(_cachedType==type::BasicType::Real){
                program.push_back(INC_R_1);
                program.push_back(_var->index());
            }
        }
        
        void LocalVariableNode::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program){
            if (_cachedType==type::BasicType::Int) {
                program.push_back(DEC_I_1);
                program.push_back(_var->index());
            }
            else if(_cachedType==type::BasicType::Real){
                program.push_back(DEC_R_1);
                program.push_back(_var->index());
            }
        }
        
        void LocalVariableNode::setInferedType(type::Type* t){
            _var->setType(t);
            _cachedType=_var->type();
        }
        
        std::string LocalVariableNode::toString(){
            return _name;
        }
        
    }
}
