//
//  StructureNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__StuctureNodes__
#define __TIMScript__StuctureNodes__

#include <iostream>
#include <map>
#include "AssignableNode.h"

namespace ts{
    
    namespace cmplr{
        class Definition;
    }

    namespace nodes{
        
        class MakeStructureNode : public AbstractNode{
        public:

            MakeStructureNode(size_t pos, size_t length, std::vector<AbstractNode*>& nodes, type::StructType* type)
                : AbstractNode(pos, length), _nodes(nodes), _structType(type) {}

            virtual ~MakeStructureNode(){}

            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();

        private:

            std::vector<AbstractNode*>& _nodes;
            type::StructType* _structType;
        };

        class StructureFieldAccessNode : public AssignableNode{
        public:
            
            StructureFieldAccessNode(size_t pos, size_t length, AbstractNode* l, const std::string& i)
                : AssignableNode(pos, length), _left(l), _identifier(i), _node(NULL), _fieldOffset(0) {}
            
            virtual ~StructureFieldAccessNode();
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void setInferedType(type::Type* t);
            virtual std::string toString();
            
            virtual NODE_TYPE getNodeType(){return N_FIELD_ACCESS;}
            
        protected:
            AbstractNode* _left;
            const std::string _identifier;
            
            AssignableNode* _node;

            size_t _fieldOffset;
        };

    }
}

#endif /* defined(__TIMScript__StructureNodes__) */
