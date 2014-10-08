//
//  Scope.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Scope.h"
#include "ClassNode.h"
#include "FunctionNode.h"

namespace ts{
    namespace cmplr{

        Scope::Scope(Scope* p) : _parent(p), _offset(_parent!=NULL ? _parent->_offset+_parent->_nbLocals : 0), _nbLocals(0), _independant(false)
        {
        }

        Variable* Scope::var(const std::string& name){
            auto it=_locals.find(name);
            if(it!=_locals.end())
                return it->second;
            else if(_parent==NULL || _independant)
                return NULL;
            else
                return _parent->var(name);
        }

        Variable* Scope::setVar(const std::string& name, Variable* var){
            Variable* newVar=new Variable(var, _offset+(_nbLocals++));
            _locals[name]=newVar;
            return newVar;
        }

        Variable* Scope::setVar(const std::string& name, type::Type* t){
            Variable* newVar=new Variable(t, _offset+(_nbLocals++));
            _locals[name]=newVar;
            return newVar;
        }

        Variable* Scope::setStructVar(const std::string& name, type::StructType* t)
        {
            Variable* newVar=new Variable(t, _offset+_nbLocals);
            _locals[name]=newVar;
            return newVar;
        }

        Definition* Scope::def(const std::string& name){
            auto it=_defs.find(name);
            if(it!=_defs.end())
                return it->second;
            else if(_parent==NULL)
                return NULL;
            else
                return _parent->def(name);
        }

        Definition* Scope::setDef(const std::string& name, size_t i, type::Type* t, nodes::AbstractNode* n, nodes::ClassNode* curClass){
            Definition* newDef;

            if (curClass!=NULL)
                newDef=new Definition(curClass->name()+"."+name, i, t, n);
            else
                newDef=new Definition(name, i, t, n);

            auto it=_defs.find(name);

            if(it!=_defs.end())
                it->second->addOverloadedDefinition(newDef);
            else
                _defs[name]=newDef;

            return newDef;
        }

        type::Type* Scope::getType(const std::string& name){
            auto it=_types.find(name);
            if(it!=_types.end())
                return it->second;
            else if(_parent==NULL)
                return NULL;
            else
                return _parent->getType(name);
        }

        void Scope::setUnimplemented(const std::string& name, type::Type* t, nodes::AbstractNode* n){
            Scope* s=this;
            while(s->_parent!=NULL)
                s=s->_parent;
            s->setType(name, t);
            s->setDef(name, 0, type::BasicType::Class, n);
        }

        void Scope::recoverUnimplemented(type::Type* t, nodes::AbstractNode* n){
            Scope* s=this;
            while(s->_parent!=NULL)
                s=s->_parent;

            for (auto tit=s->_types.begin(); tit!=s->_types.end(); ++tit) {
                if (tit->second==t) {
                    const std::string& name(tit->first);
                    s->_types.erase(tit);
                    setType(name, t);

                    for (auto dit=s->_defs.begin(); dit!=s->_defs.end(); ++dit) {
                        if (dit->second->value()==n) {

                            const std::string& name(dit->first);
                            _defs[name]=dit->second;
                            s->_defs.erase(dit);

                            return;
                        }
                    }

                    //ERROR
                }
            }
            //ERROR
        }

        void Scope::setType(const std::string& name, type::Type* t){
            _types[name]=t;
        }

        void Scope::setUnimplementedType(const std::string &name, type::Type *t){
            Scope* s=this;
            while(s->_parent!=NULL)
                s=s->_parent;
            s->setType(name, t);
        }
    }
}
