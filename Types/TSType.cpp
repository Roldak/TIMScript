//
//  TSType.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "TSType.h"
#include "ClassNode.h"
#include "InterfaceNode.h"
#include "FunctionNode.h"

namespace ts {
    namespace type{
        
        // BASIC TYPE
        
        Type* BasicType::Bool(new BasicType("bool"));
        Type* BasicType::Int(new BasicType("int"));
        Type* BasicType::Real(new BasicType("real"));
        Type* BasicType::Void(new BasicType("void"));
        Type* BasicType::Class(new BasicType("class"));
        Type* BasicType::NotYetDefined(new BasicType("nyd"));
        Type* BasicType::Any(new BasicType("Any"));
        
        bool BasicType::isBasicType(type::Type* t){
            return t==Bool || t==Int || t==Real || t==Void || t==Class;
        }
        
        // FUNCTION TYPE
        
        FunctionType::FunctionType(const std::vector<Type*>& at, Type* rt, bool isN)
        : _argsTypes(at), _returnType(rt), _isNative(isN), _isAbstract(false)
        {
        
        }
        
        bool FunctionType::isOverrideOf(FunctionType* other){
            if (other->_argsTypes.size()!=_argsTypes.size())
                return false;


            /*
            if (!_returnType->equals(other->_returnType))
                return false;
            */
            for (size_t i=1; i<_argsTypes.size(); ++i) {
                if (!_argsTypes[i]->equals(other->_argsTypes[i]))
                    return false;
            }
            
            if (ClassType* ct_a=getIf<ClassType*>(_argsTypes[0])) 
                if (ClassType* ct_b=getIf<ClassType*>(other->_argsTypes[0]))
                    if (!ct_a->inheritsFrom(ct_b))
                        return false;
            
            return true;
        }

        size_t FunctionType::totalArgsSize()
        {
            size_t s=0;
            for(size_t i=0; i<_argsTypes.size(); ++i){
                s+=_argsTypes[i]->size();
            }
            return s;
        }

        std::vector<Type*> FunctionType::flattenTypes()
        {
            std::vector<Type*> types;
            for(size_t i=0; i<_argsTypes.size(); ++i){
                if(StructType* st=getIf<StructType*>(_argsTypes[i]))
                    st->addFlatten(types);
                else
                    types.push_back(_argsTypes[i]);
            }
            return types;
        }
        
        bool FunctionType::equals(Type* other){
            if (FunctionType* fother=type::getIf<FunctionType*>(other)){
                if (fother->_argsTypes.size()!=_argsTypes.size())
                    return false;
                
                if (!_returnType->equals(fother->_returnType))
                    return false;
                
                for (size_t i=0; i<_argsTypes.size(); ++i) {
                    if (!_argsTypes[i]->equals(fother->_argsTypes[i]))
                        return false;
                }
                
                return true;
            }
            return false;
        }
        
        std::string FunctionType::toString(){
            std::string toRet="(";
            for (size_t i=0; i<_argsTypes.size(); ++i) {
                toRet+=_argsTypes[i]->toString();
                if (i!=_argsTypes.size()-1)
                    toRet+=", ";
            }
            return toRet+")->"+_returnType->toString();;
        }
        
        FunctionType* FunctionType::RealToReal(new FunctionType({BasicType::Real}, BasicType::Real));
        FunctionType* FunctionType::IntToInt(new FunctionType({BasicType::Int}, BasicType::Int));
        
        
        // ARRAY TYPE
        
        bool ArrayType::equals(Type* other){
            if (ArrayType* ot=type::getIf<ArrayType*>(other)) {
                return ot->_elementsType->equals(_elementsType) && ot->_nbElems==_nbElems;
            }
            return false;
        }
        
        std::string ArrayType::toString(){
            return _elementsType->toString()+"["+(_nbElems==-1 ? "" : T_toString(_nbElems))+"]";
        }
        
        // TUPLE TYPE
        
        bool TupleType::equals(Type* other){
            if (TupleType* tt=type::getIf<TupleType*>(other)) {
                const std::vector<Type*>& otherTypes(tt->types());
                if (otherTypes.size()!=_types.size())
                    return false;

                for (size_t i=0; i<otherTypes.size(); ++i) {
                    if (!otherTypes[i]->equals(_types[i])){
                        return false;
                    }
                }
                return true;
            }
            return false;
        }

        std::string TupleType::toString(){
            std::string toRet="(";
            for (size_t i=0; i<_types.size(); ++i) {
                toRet+=_types[i]->toString();
                if (i!=_types.size()-1)
                    toRet+=", ";
            }
            return toRet+")";
        }
        
        // CLASS TYPE
        
        ClassType::ClassType(nodes::ClassNode* c) : BasicType(c->name()), _class(c) {
            
        }
        
        bool ClassType::inheritsFrom(ClassType* c){
            const nodes::ClassNode* thisClass=_class;
            nodes::ClassNode* otherClass=c->getClass();
            
            while (thisClass!=NULL) {
                if (thisClass!=otherClass) {
                    thisClass=thisClass->superClass();
                }else{
                    return true;
                }
            }
            return false;
        }

        bool ClassType::inheritsFrom(InterfaceType* i){
            const nodes::ClassNode* thisClass=_class;

            while (thisClass!=NULL) {
                const std::vector<nodes::InterfaceNode*>& interfaces=thisClass->getInterfaces();

                for(nodes::InterfaceNode* thisInterface : interfaces)
                    if(thisInterface->typeInterface()->inheritsFrom(i))
                        return true;

                thisClass=thisClass->superClass();
            }
            return false;
        }

        bool ClassType::inheritsFrom(ComposedType *c)
        {
            if(c->getClass()){
                if(!inheritsFrom(c->getClass())){
                    return false;
                }
            }

            for(InterfaceType* i : c->getInterfaces()){
                if(!inheritsFrom(i))
                    return false;
            }
            return true;
        }

        bool ClassType::inheritsFrom(Type *t)
        {
            if(ClassType* ct=getIf<ClassType*>(t))
                return inheritsFrom(ct);
            else if(InterfaceType* it=getIf<InterfaceType*>(t))
                return inheritsFrom(it);
            else if(ComposedType* cpt=getIf<ComposedType*>(t))
                return inheritsFrom(cpt);
            else
                return false;
        }

        // INTERFACE TYPE

        InterfaceType::InterfaceType(nodes::InterfaceNode *i) : BasicType(i->name()), _interface(i){

        }

        bool InterfaceType::inheritsFrom(ClassType *c)
        {
            return false;
        }

        bool InterfaceType::inheritsFrom(InterfaceType* other)
        {
            if(this==other)
                return true;

            for(nodes::InterfaceNode* i : _interface->getInterfaces()){
                if(i->typeInterface()->inheritsFrom(other))
                    return true;
            }

            return false;
        }

        bool InterfaceType::inheritsFrom(ComposedType *c)
        {
            return false;
        }

        bool InterfaceType::inheritsFrom(Type *t)
        {
            if(ClassType* ct=getIf<ClassType*>(t))
                return inheritsFrom(ct);
            else if(InterfaceType* it=getIf<InterfaceType*>(t))
                return inheritsFrom(it);
            else if(ComposedType* cpt=getIf<ComposedType*>(t))
                return inheritsFrom(cpt);
            else
                return false;
        }

        std::string InterfaceType::toString()
        {
            return "@"+BasicType::toString();
        }

        // STRUCTURE TYPE

        StructType::StructType(const std::string &name, const std::vector<nodes::Argument*> fields) : BasicType(name), _fields(fields), _size(0){
            for(nodes::Argument* field : fields){
                _size+=field->type()->size();
            }
        }

        size_t StructType::fieldIndex(const std::string& fieldName)
        {
            for(size_t i=0; i<_fields.size(); ++i){
                if(_fields[i]->name()==fieldName)
                    return i;
            }
            return std::string::npos;
        }

        size_t StructType::fieldOffset(const std::string &fieldName)
        {
            size_t offset=0;
            for(size_t i=0; i<_fields.size(); ++i){
                if(_fields[i]->name()==fieldName)
                    return offset;
                else
                    offset+=_fields[i]->type()->size();
            }
            return std::string::npos;
        }

        void StructType::addFlatten(std::vector<Type*>& types)
        {
            for(size_t i=0; i<_fields.size(); ++i){
                if(StructType* st=getIf<StructType*>(_fields[i]->type()))
                    st->addFlatten(types);
                else
                    types.push_back(_fields[i]->type());
            }
        }

        // COMPOSED TYPE

        ComposedType::ComposedType(ClassType *ct, const std::vector<InterfaceType *> &interfaces) : _class(ct), _interfaces(interfaces){

        }

        bool ComposedType::inheritsFrom(ClassType* other){
            if(!_class)
                return false;

            if(_class->inheritsFrom(other)){
                for(nodes::InterfaceNode* i : other->getClass()->getInterfaces()){
                    if(!implements(i->typeInterface()))
                        return false;
                }
                return true;
            }
            return false;
        }

        bool ComposedType::inheritsFrom(InterfaceType* other){
            return implements(other);
        }

        bool ComposedType::inheritsFrom(ComposedType* other){
            if(!(_class && other->_class))
                return false;

            if(!_class->inheritsFrom(other->_class))
                return false;

            for(InterfaceType* i : other->_interfaces){
                if(!implements(i))
                    return false;
            }

            return true;
        }

        bool ComposedType::inheritsFrom(Type *t)
        {
            if(ClassType* ct=getIf<ClassType*>(t))
                return inheritsFrom(ct);
            else if(InterfaceType* it=getIf<InterfaceType*>(t))
                return inheritsFrom(it);
            else if(ComposedType* cpt=getIf<ComposedType*>(t))
                return inheritsFrom(cpt);
            else
                return false;
        }

        bool ComposedType::implements(InterfaceType* i){
            for(InterfaceType* it : _interfaces){
                if(i->inheritsFrom(it))
                    return true;
            }
            return false;
        }

        ImplementationLocation ComposedType::getInterfaceOwning(const std::string& identifier)
        {
            for(InterfaceType* i : _interfaces){
                ImplementationLocation loc=i->getInterface()->getDefinitionIndex(identifier);
                if(!loc.Empty)
                    return loc;
            }
            return ImplementationLocation();
        }

        bool ComposedType::equals(Type* other){
            if(ComposedType* o=getIf<ComposedType*>(other)){
                if(_class==o->_class){
                    if(_interfaces.size()==o->_interfaces.size()){
                        for(InterfaceType* it : _interfaces){
                            if(!o->implements(it))
                                return false;
                        }
                        return true;
                    }
                }
            }
            return false;
        }

        std::string ComposedType::toString(){
            std::string toRet=_class ? _class->toString()+" extends " : "";
            toRet+=_interfaces[0]->toString();
            for(size_t i=1; i<_interfaces.size(); ++i)
                toRet+=", "+_interfaces[i]->toString();
            return toRet;
        }

    }
}
