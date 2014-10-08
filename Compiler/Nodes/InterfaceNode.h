//
//  InterfaceNode.h
//  TIMScript
//
//  Created by Romain Beguet on 13.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__InterfaceNode__
#define __TIMScript__InterfaceNode__

#include <iostream>
#include <map>
#include "AbstractNode.h"
#include "DefinitionReferenceNode.h"

namespace ts {
    namespace cmplr{
        class Definition;
    }
    
    namespace nodes{
        class FunctionNode;

        struct InterfaceField{

            InterfaceField() {}
            InterfaceField(const std::string& n, DefinitionReferenceNode* ref)  : name(n), def(ref) {}

            const std::string name;

            DefinitionReferenceNode* def;

        };

        class InterfaceNode : public AbstractNode{
        public:
            
            InterfaceNode(size_t pos, size_t length, const std::string& n, size_t id);
            virtual ~InterfaceNode();

            inline void resetNodeLocation(size_t pos, size_t length){_pos=pos; _length=length;}
            
            inline const std::string& name(){return _name;}
            inline type::InterfaceType* typeInterface(){return _interfaceType;}

            inline void addDefinition(const std::string& name, cmplr::Definition* d){
                addDefinition(name, new DefinitionReferenceNode(0, 0, name, d));
            }

            inline void addDefinition(const std::string& name, DefinitionReferenceNode* ref){
                _defs.push_back(InterfaceField(name, ref));
            }

            void addDefinitions(const std::map<std::string, cmplr::Definition*>& definitions);

            inline const std::vector<InterfaceField>& defs(){ return _defs; }

            inline ImplementationLocation getDefinitionIndex(const std::string& name){
                for(size_t i=0; i<_defs.size(); ++i){
                    if(_defs[i].name==name)
                        return ImplementationLocation(this, _ID, i);
                }

                for(InterfaceNode* interface : _interfaces){
                    ImplementationLocation loc=interface->getDefinitionIndex(name);
                    if(!loc.Empty)
                        return loc;
                }

                return ImplementationLocation();
            }

            inline void addInterface(InterfaceNode* i){_interfaces.push_back(i);}
            inline const std::vector<InterfaceNode*>& getInterfaces(){return _interfaces;}

            inline size_t interfaceID(){return _ID;}

            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();

            virtual NODE_TYPE getNodeType(){return N_INTERFACE;}
            
        private:

            const std::string _name;
            size_t _ID;

            type::InterfaceType* _interfaceType;

            std::vector<InterfaceField> _defs;
            std::vector<InterfaceNode*> _interfaces;
        };
        
    }
}


#endif /* defined(__TIMScript__ClassNode__) */
