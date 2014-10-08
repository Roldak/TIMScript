//
//  DefinitionReferenceNode.h
//  TIMScript
//
//  Created by Romain Beguet on 04.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__DefinitionReferenceNode__
#define __TIMScript__DefinitionReferenceNode__

#include <iostream>
#include "AssignableNode.h"
#include "UserData.h"
#include "Scope.h"

namespace ts{
    
    namespace nodes{
        
        class DefinitionReferenceNode : public AssignableNode{
        public:
            DefinitionReferenceNode(size_t pos, size_t length, const std::string& n, cmplr::Definition* r)
                : AssignableNode(pos, length), _name(n), _def(r){}
            
            DefinitionReferenceNode(const std::string& n, cmplr::Definition* r);
            
            virtual ~DefinitionReferenceNode(){}
            
            inline const std::string name(){return _name;}
            inline cmplr::Definition* definition(){return _def;}
            inline void setDefinition(cmplr::Definition* def){_def=def;}

            virtual void nextOverloadedDefinition(){ _def=_def->getNextOverloadedDefinition(); }
            
            virtual size_t definitionIndex() {return _def->index();}
            virtual type::Type* definitionType(){
                if(_def->value()){
                    _def->value()->semanticTraverse();
                    return _def->value()->type();
                }else{
                    return _def->type();
                }
            }
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void setInferedType(type::Type* t);
            virtual std::string toString();
            
            virtual NODE_TYPE getNodeType(){return N_DEFINITION_REFERENCE;}
            
        private:
            
            const std::string _name;
            cmplr::Definition* _def;
        };
        
        class NativeDefinitionReferenceNode : public DefinitionReferenceNode{
        public:
            NativeDefinitionReferenceNode(size_t pos, size_t length, const std::string& name, const UserData& dat)
                : DefinitionReferenceNode(pos, length, name, NULL), _data(dat){}
            
            virtual ~NativeDefinitionReferenceNode(){}
            
            inline UserData& getUserData(){return _data;}
            
            inline objects::NativeData* getData(){return _data._data;}
            
            virtual void nextOverloadedDefinition(){}
            
            virtual size_t definitionIndex(){return _data._index;}
            virtual type::Type* definitionType(){return _data._type;}
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            
        private:

            UserData _data;
        };
        
    }
}

#endif /* defined(__TIMScript__DefinitionReferenceNode__) */
