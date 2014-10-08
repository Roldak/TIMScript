//
//  BinaryOperationNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 30.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "BinaryOperationNodes.h"

#include "CastNode.h"
#include "ClassNode.h"
#include "CallNode.h"
#include "InstanceNodes.h"
#include "DotOperation.h"
#include "CompilationExceptions.h"
#include "CastUtils.h"

namespace ts {
    namespace nodes{
        
        template<OP_CODE onInt, OP_CODE onReal, bool returnsBoolean>
        void BinaryOperation<onInt, onReal, returnsBoolean>::semanticTraverse(){
            if (_alrdyComputed)
                return;
            _alrdyComputed=true;
            
            _l->semanticTraverse();
            _r->semanticTraverse();
            
            type::Type* tl=_l->type();
            type::Type* tr=_r->type();
            
            if(tl==type::BasicType::Bool)   tl=type::BasicType::Int;
            if(tr==type::BasicType::Bool)   tr=type::BasicType::Int;
            
            if (tl==type::BasicType::Int && tr==type::BasicType::Real){
                _l=new CastNode(_l, tr);
                tl=tr;
            }
            else if(tr==type::BasicType::Int && tl==type::BasicType::Real){
                _r=new CastNode(_r, tl);
                tr=tl;
            }

            if(type::getIf<type::ClassType*>(tl) || type::getIf<type::ComposedType*>(tl)){

                _isMethodCall=true;

                // binary operators overloading in a class takes the form of '_+_', '_*_', etc.

                _l=new DotOperationNode(_l->pos(), _r->pos(), _l, "_"+getOPCODElitteral<onInt>()+"_");
                _l=new CallNode(_l->pos(), _l->len(), _l, {_r});
                _l->semanticTraverse();

                _cachedType=_l->type();
                return;
            }
            else if (tl==type::BasicType::Real || tr==type::BasicType::Real){
                _castType=type::BasicType::Real;
                _cachedType=type::BasicType::Real;
            }
            else if(tl==type::BasicType::Int && tr==type::BasicType::Int){
                _castType=type::BasicType::Int;
                _cachedType=type::BasicType::Int;
            }
            
            _cachedType=returnsBoolean ? type::BasicType::Bool : _cachedType;
        }
        
        template<OP_CODE onInt, OP_CODE onReal, bool returnsBoolean>
        void BinaryOperation<onInt, onReal, returnsBoolean>::pushBytecode(std::vector<TSINSTR>& program){
            if (_isMethodCall) { // in case of operator overloading
                _l->pushBytecode(program);
            }
            else{
                _l->pushBytecode(program);
                _r->pushBytecode(program);
                
                if(_castType==type::BasicType::Int)
                    program.push_back(onInt);
                else if(_castType==type::BasicType::Real)
                    program.push_back(onReal);
            }
        }
        
        template<OP_CODE onInt, OP_CODE onReal, bool returnsBoolean>
        std::string BinaryOperation<onInt, onReal, returnsBoolean>::toString(){
            return "("+_l->toString()+getOPCODElitteral<onInt>()+_r->toString()+")";
        }
        
        // BINARY BOOLEAN OPERATION
        
        template<OP_CODE code>
        void BinaryBoolOperation<code>::semanticTraverse(){
            if (_alrdyComputed)
                return;
            _alrdyComputed=true;
            
            _l->semanticTraverse();
            _r->semanticTraverse();
            
            type::Type* tl=_l->type();
            type::Type* tr=_r->type();
            
            if(type::getIf<type::ClassType*>(tl) || type::getIf<type::ComposedType*>(tl)){

                _isMethodCall=true;

                // binary operators overloading in a class takes the form of '_&&_', '_||_', etc.

                _l=new DotOperationNode(_l->pos(), _r->pos(), _l, "_"+getOPCODElitteral<code>()+"_");
                _l=new CallNode(_l->pos(), _l->len(), _l, {_r});
                _l->semanticTraverse();

                _cachedType=_l->type();
                return;
            }
            else if (tl!=type::BasicType::Bool)
                _l=new CastNode(_l, type::BasicType::Bool);
            else if (tr!=type::BasicType::Bool)
                _r=new CastNode(_r, type::BasicType::Bool);
            
            _cachedType=type::BasicType::Bool;
        }
        
        template<OP_CODE code>
        void BinaryBoolOperation<code>::pushBytecode(std::vector<TSINSTR>& program){
            if (_isMethodCall) { // in case of operator overloading
                _l->pushBytecode(program);
            }
            else{
                _l->pushBytecode(program);
                _r->pushBytecode(program);
                program.push_back(code);
            }
        }
        
        template<OP_CODE code>
        std::string BinaryBoolOperation<code>::toString(){
            return "("+_l->toString()+getOPCODElitteral<code>()+_r->toString()+")";
        }
            
        // INDEX ACCESS OPERATION
        
        void IndexAccessOperation::semanticTraverse(){
            if(_alrdyComputed)
                return;

            _accessed->semanticTraverse();
            _index->semanticTraverse();
            if (type::ArrayType* aryT=type::getIf<type::ArrayType*>(_accessed->type())) {
                if (_index->type()!=type::BasicType::Int)
                    _index=new CastNode(_index, type::BasicType::Int);
                _cachedType=aryT->elementsType();
            }
            else if(type::getIf<type::ClassType*>(_accessed->type())){
                _accessed=new DotOperationNode(_accessed->pos(), _accessed->len(), _accessed, "_[_");
                _accessed=new CallNode(_accessed->pos(), _accessed->len(), _accessed, {_index});
                _accessed->semanticTraverse();

                _cachedType=_accessed->type();
            }

            _alrdyComputed=true;
        }
        
        void IndexAccessOperation::pushBytecode(std::vector<TSINSTR>& program){
            if (type::getIf<type::ArrayType*>(_accessed->type())) {
                _accessed->pushBytecode(program);
                _index->pushBytecode(program);
                program.push_back(GET_AT);
            }
            else{
                _accessed->pushBytecode(program);
            }
        }
        
        void IndexAccessOperation::pushAssignementBytecode(std::vector<TSINSTR>& program){
            if (type::ArrayType* at=type::getIf<type::ArrayType*>(_accessed->type())) {
                _accessed->pushBytecode(program);
                _index->pushBytecode(program);
                if(type::BasicType::isBasicType(at->elementsType()))
                    program.push_back(SET_AT);
                else
                    program.push_back(SET_AT_R);
            }
        }
        
        void IndexAccessOperation::pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program){
            
        }
        
        void IndexAccessOperation::pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program){
            
        }
        
        void IndexAccessOperation::setInferedType(type::Type* t){
            
        }
        
        std::string IndexAccessOperation::toString(){
            return _accessed->toString()+"["+_index->toString()+"]";
        }

        // BINARY IS

        void BinaryIs::semanticTraverse(){
            _l->semanticTraverse();
            _r->semanticTraverse();

            if(!type::getIf<type::ClassType*>(_l->type()) && !type::getIf<type::InterfaceType*>(_l->type())){
                throw exception::InvalidOperandBinaryIs(_l, _l->type(), "Instance", "left");
            }

            if(_r->type()!=type::BasicType::Class && !getIf<NullReferenceNode*>(_r)){
                throw exception::InvalidOperandBinaryIs(_r, _r->type(), "Class", "right");
            }

            if(!getIf<DefinitionReferenceNode*>(_r)){
                // ERROR
            }

            _cachedType=type::BasicType::Bool;
        }

        void BinaryIs::pushBytecode(std::vector<TSINSTR>& program){
            _l->pushBytecode(program);
            _r->pushBytecode(program);

            if(getIf<NullReferenceNode*>(_r)){
                program.push_back(CMP_I_EQ);
            }else{
                program.push_back(INSTANCEOF);
            }
        }

        std::string BinaryIs::toString(){
            return _l->toString()+" is "+_r->toString();
        }
        
        // EXPLICIT INSTANTIATION
        
        template class BinaryOperation<ADD_I, ADD_R, false>;
        template class BinaryOperation<SUB_I, SUB_R, false>;
        template class BinaryOperation<MUL_I, MUL_R, false>;
        template class BinaryOperation<DIV_I, DIV_R, false>;
        template class BinaryOperation<MOD_I, MOD_R, false>;
        template class BinaryOperation<POW_I, POW_R, false>;
        template class BinaryOperation<CMP_I_L, CMP_R_L, true>;
        template class BinaryOperation<CMP_I_G, CMP_R_G, true>;
        template class BinaryOperation<CMP_I_EQ, CMP_R_EQ, true>;
        template class BinaryOperation<CMP_I_NEQ, CMP_R_NEQ, true>;
        template class BinaryBoolOperation<AND_B>;
        template class BinaryBoolOperation<OR_B>;
        
    }
}
