//
//  FunctionNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "FunctionNode.h"
#include "CastNode.h"
#include "VariableNode.h"

namespace ts {
    namespace nodes{
        
        // Function Node
        
        FunctionNode::FunctionNode(size_t pos, size_t length,
                     const std::vector<Argument*>& a, const std::vector<LocalVariableNode*>& c,
                     type::Type* t, ReturnNode* b,
                     size_t refMapTupleIndex,
                     size_t exTableIndex,
                     size_t nbLocals, size_t dbgInfoIndex)
        
        : AbstractNode(pos, length), _args(a), _captures(c), _retType(t), _body(b),
          _refMapTupleIndex(refMapTupleIndex), _exTableIndex(exTableIndex), _nbLocals(nbLocals),  _dbgInfoIndex(dbgInfoIndex)
        {
            std::vector<type::Type*> argTypes(_args.size());
            for (size_t i=0; i<argTypes.size(); ++i)
                argTypes[i]=_args[i]->type();
            
            _signature=new type::FunctionType(argTypes, t);
            _cachedType=NULL;
        }
        
        void FunctionNode::setBody(AbstractNode* node){
            _body->setNode(node);
        }
        
        AbstractNode* FunctionNode::body(){
            return _body->node();
        }
        
        void FunctionNode::semanticTraverse(){
            if (_cachedType!=NULL)
                return;
            
            _cachedType=_signature;
            
            _body->semanticTraverse();

            if (_retType==type::BasicType::NotYetDefined)
                _retType=_body->type(); // infered type
            
            ((type::FunctionType*)_cachedType)->setReturnType(_retType);

            if (!_retType->equals(_body->type()))
                _body->setNode(new CastNode(_body->node(), _retType)); // auto-cast

            for (size_t i=0; i<_captures.size(); ++i)
                _captures[i]->semanticTraverse();
        }
        
        void FunctionNode::pushBytecode(std::vector<TSINSTR>& program){

            size_t captureCount=0;
            for (size_t i=0; i<_captures.size(); ++i){
                captureCount+=_captures[i]->type()->size();
                _captures[i]->pushBytecode(program);
            }
            
            program.push_back(MK_FUNC);
            program.push_back(10);
            program.push_back(_nbLocals);
            program.push_back(captureCount);
            program.push_back(_refMapTupleIndex);
            program.push_back(_exTableIndex);
            size_t pos=program.size();
            program.push_back(0);
            
            std::vector<TSINSTR> theFunc;
            _body->pushBytecode(theFunc);
            program.insert(program.end(), theFunc.begin(), theFunc.end());
            size_t length=theFunc.size();
            program[pos]=length-1;
        }
        
        std::string FunctionNode::toString(){
            std::string toRet="(";
            for (size_t i=0; i<_args.size(); ++i) {
                toRet+=_args[i]->name()+":"+_args[i]->type()->toString();
                if (i!=_args.size()-1)
                    toRet+=", ";
            }
            return toRet+")->"+_retType->toString()+_body->toString();
        }
        
        // Explicit Return Node
        
        void ExplicitReturnNode::semanticTraverse(){
            _node->semanticTraverse();
            _cachedType=type::BasicType::Void;
        }
        
        void ExplicitReturnNode::pushBytecode(std::vector<TSINSTR>& program){
            _node->pushBytecode(program);
            if (_node->type()==type::BasicType::Void)
                program.push_back(RET_V);
            else
                program.push_back(RET);
        }
        
        std::string ExplicitReturnNode::toString(){
            return "return "+_node->toString();
        }
        
        // Return Node
        
        void ReturnNode::semanticTraverse(){
            _node->semanticTraverse();
            _cachedType=_node->type();
        }
        
        void ReturnNode::pushBytecode(std::vector<TSINSTR>& program){
            _node->pushBytecode(program);
            if (_node->type()==type::BasicType::Void)
                program.push_back(RET_V);
            else if(type::BasicType::isBasicType(_node->type()))
                program.push_back(RET);
            else
                program.push_back(RET_R);
        }
        
        std::string ReturnNode::toString(){
            return "=>"+_node->toString();
        }
        
    }
}
