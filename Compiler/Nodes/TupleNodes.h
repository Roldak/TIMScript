//
//  TupleNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 06.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__TupleNodes__
#define __TIMScript__TupleNodes__

#include <iostream>
#include "AssignableNode.h"

namespace ts {
    namespace nodes{
        
        class NewTupleNode : public AbstractNode{
        public:
            
            NewTupleNode(size_t pos, size_t length, type::TupleType* type);
            virtual ~NewTupleNode(){}
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
            virtual NODE_TYPE getNodeType(){return N_NEW_TUPLE;}
        };
        
        class MakeTupleNode : public AbstractNode{
        public:
            
            MakeTupleNode(size_t pos, size_t length, std::vector<AbstractNode*>& nodes, type::TupleType* type)
                : AbstractNode(pos, length), _nodes(nodes), _tupleType(type) {}
            
            virtual ~MakeTupleNode(){}
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            std::vector<AbstractNode*>& _nodes;
            type::TupleType* _tupleType;
        };
        
        class TupleElementAccessNode : public AssignableNode{
        public:
            
            TupleElementAccessNode(size_t pos, size_t length, AbstractNode* l, const std::string& i)
                : AssignableNode(pos, length), _left(l), _identifier(i) {}
            
            virtual ~TupleElementAccessNode(){}
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual void pushAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushPlusPlusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void pushMinusMinusAssignementBytecode(std::vector<TSINSTR>& program);
            virtual void setInferedType(type::Type* t);
            virtual std::string toString();
            
            virtual NODE_TYPE getNodeType(){return N_ASSIGNABLE;}
            
        protected:
            AbstractNode* _left;
            const std::string _identifier;
            
            size_t _elementIndex;
        };
        
    }
}

#endif /* defined(__TIMScript__TupleNodes__) */
