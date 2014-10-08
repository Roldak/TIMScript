//
//  NewInstanceNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "InstanceNodes.h"
#include "ClassNode.h"
#include "DefinitionReferenceNode.h"
#include "Scope.h"
#include "FunctionNode.h"
#include "ConstNodes.h"
#include "CastUtils.h"
#include "CompilationExceptions.h"

namespace ts {
    namespace nodes{
        
        // MAKE OBJECT NODE
        
        MakeObject::MakeObject(size_t pos, size_t length, ClassNode* c, bool scoped)
        : AbstractNode(pos, length), _class(c)
        {
            _cachedType=_class->typeClass();
            _scoped=scoped;
        }
        
        void MakeObject::semanticTraverse(){

        }
        
        void MakeObject::pushBytecode(std::vector<TSINSTR>& program){
            program.push_back(LOAD_CST);
            program.push_back(_class->typeClass()->getClassDefIndex());
            program.push_back(_scoped ? NEW_SCOPED : NEW);
        }
        
        std::string MakeObject::toString(){
            return "";
        }
        
        // NEW INSTANCE NODE
        
        NewInstanceNode::NewInstanceNode(size_t pos, size_t length, ClassNode* c, bool scoped)
            : AbstractNode(pos, length), _class(c)
        {
            _scoped=scoped;
        }
        
        DefinitionReferenceNode* NewInstanceNode::getConstructor(){
            _constrIndex=_class->getDefinitionIndex("_new_");
            if (_constrIndex!=std::string::npos)
                return _class->defs()[_constrIndex];
            else{
                throw exception::ReferingMissingConstructor(this, _class->name());
            }
            return NULL;
        }
        
        void NewInstanceNode::semanticTraverse(){
            _cachedType=getConstructor()->definitionType();
        }
        
        void NewInstanceNode::pushBytecode(std::vector<TSINSTR>& program){
            getConstructor()->pushBytecode(program);
        }
        
        std::string NewInstanceNode::toString(){
            return "new "+_class->name();
        }
        
        // ATTRIBUTE ACCESS NODE
        
        void AttributeAccessNode::semanticTraverse(){
            _left->semanticTraverse();
            type::ClassType* ct=type::getIf<type::ClassType*>(_left->type());
            if(!ct)
                ct=((type::ComposedType*)_left->type())->getClass();

            if (ct) {
                _attributeIndex=ct->getClass()->getAttributeIndex(_identifier);
                _cachedType=ct->getClass()->getAttributes()[_attributeIndex]->type();
            }
        }
        
        void AttributeAccessNode::pushBytecode(std::vector<TSINSTR>& program){
            _left->pushBytecode(program);
            
            if (nodes::getIf<LocalVariableNode*>(_left) && program[program.size()-2]==LOAD_N && program[program.size()-1]==0) {
                program[program.size()-2]=LD_0_GET_F;
                program[program.size()-1]=_attributeIndex;
            }
            else{
                program.push_back(GET_F);
                program.push_back(_attributeIndex);
            }
        }
        
        void AttributeAccessNode::pushAssignementBytecode(std::vector<TSINSTR>& program){
            _left->pushBytecode(program);
            if (program[program.size()-2]==LOAD_N && program[program.size()-1]==0) {
                if(type::BasicType::isBasicType(_cachedType))
                    program[program.size()-2]=LD_0_SET_F;
                else
                    program[program.size()-2]=LD_0_SET_F_R;
                
                program[program.size()-1]=_attributeIndex;
            }
            else{
                if(type::BasicType::isBasicType(_cachedType))
                    program.push_back(SET_F);
                else
                    program.push_back(SET_F_R);
                
                program.push_back(_attributeIndex);
            }
        }
        
        void AttributeAccessNode::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program){
            pushBytecode(program);
            
            if (_cachedType==type::BasicType::Int){
                IntegerNode(0,0,1).pushBytecode(program);
                program.push_back(ADD_I);
            }
            else if(_cachedType==type::BasicType::Real){
                RealNode(0,0,1).pushBytecode(program);
                program.push_back(ADD_R);
            }
            
            pushAssignementBytecode(program);
            pushBytecode(program);
        }
        
        void AttributeAccessNode::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program){
            pushBytecode(program);
            
            if (_cachedType==type::BasicType::Int){
                IntegerNode(0,0,1).pushBytecode(program);
                program.push_back(SUB_I);
            }
            else if(_cachedType==type::BasicType::Real){
                RealNode(0,0,1).pushBytecode(program);
                program.push_back(SUB_R);
            }
            
            pushAssignementBytecode(program);
            pushBytecode(program);
        }
        
        void AttributeAccessNode::setInferedType(type::Type* t){
            
        }
        
        std::string AttributeAccessNode::toString(){
            return _left->toString()+"."+_identifier;
        }
        
        // VIRTUAL ACCESS NODE
        
        DefinitionReferenceNode* VirtualAccessNode::definition(){
            type::ClassType* ct=type::getIf<type::ClassType*>(_left->type());
            if(!ct)
                ct=((type::ComposedType*)_left->type())->getClass();

            return ct->getClass()->defs()[_defIndex];
        }
        
        void VirtualAccessNode::setDefinitionIndex(size_t defIndex){
            _defIndex=defIndex;
            _cachedType=definition()->definitionType();
            _nbArgs=((type::FunctionType*)_cachedType)->argsTypes().size();
        }
        
        void VirtualAccessNode::semanticTraverse(){
            _left->semanticTraverse();

            type::ClassType* ct=type::getIf<type::ClassType*>(_left->type());
            if(!ct)
                ct=((type::ComposedType*)_left->type())->getClass();

            if (ct) {
                _defIndex=ct->getClass()->getDefinitionIndex(_identifier);
                if (_defIndex==std::string::npos) {
                    throw exception::UnknownMember(this, "class", ct->getClass()->name(), _identifier);
                }
                _cachedType=ct->getClass()->defs()[_defIndex]->definitionType();
                _nbArgs=((type::FunctionType*)_cachedType)->argsTypes().size();
            }
        }
        
        void VirtualAccessNode::pushBytecode(std::vector<TSINSTR>& program){
            if(_forFunctionCall){
                program.push_back(DUP);
                program.push_back(_nbArgs);
            }else{
                _left->pushBytecode(program);
            }
            program.push_back(GET_V);
            program.push_back(_defIndex);
        }
        
        std::string VirtualAccessNode::toString(){
            return _left->toString()+"."+_identifier;
        }
        
        // IMPLEMENTATION ACCESS NODE

        void ImplementationAccessNode::semanticTraverse(){
            _left->semanticTraverse();
            type::InterfaceType* it=type::getIf<type::InterfaceType*>(_left->type());

            if(!it){
                _defLocation=((type::ComposedType*)_left->type())->getInterfaceOwning(_identifier);
                _interface=(InterfaceNode*)_defLocation.interface;
            }
            else if(type::InterfaceType* t=type::getIf<type::InterfaceType*>(_left->type())){
                _interface=t->getInterface();
                _defLocation=_interface->getDefinitionIndex(_identifier);
            }

            if (_defLocation.Empty) {
                throw exception::UnknownMember(this, "interface", _left->type()->toString(), _identifier);
            }

            DefinitionReferenceNode* def=((InterfaceNode*)_defLocation.interface)->defs()[_defLocation.index].def;
            _cachedType=def->definitionType();
        }

        void ImplementationAccessNode::pushBytecode(std::vector<TSINSTR>& program){
            if(_forFunctionCall){
                program.push_back(DUP);
                program.push_back(((type::FunctionType*)_cachedType)->argsTypes().size());
            }else{
                _left->pushBytecode(program);
            }
            program.push_back(GET_I);
            program.push_back(_defLocation.interfaceID);
            program.push_back(_defLocation.index);
        }

        std::string ImplementationAccessNode::toString(){
            return _left->toString()+"."+_identifier;
        }

    }
}
