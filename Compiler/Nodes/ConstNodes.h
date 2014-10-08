//
//  ConstNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__ConstNodes__
#define __TIMScript__ConstNodes__

#include <iostream>
#include "AbstractNode.h"

namespace ts {
    
    namespace cmplr{
        class Definition;
    }
    
    namespace nodes{
        
        // BOOLEAN
        
        class BooleanNode : public AbstractNode{
        public:
            BooleanNode(size_t pos, size_t length, TSBOOL v) : AbstractNode(pos, length), _value(v){}
            virtual ~BooleanNode(){}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            const TSBOOL _value;
        };
        
        // INTEGER
        
        class IntegerNode : public AbstractNode{
        public:
            IntegerNode(size_t pos, size_t length, TSINT v) : AbstractNode(pos, length), _value(v){}
            virtual ~IntegerNode(){}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            const TSINT _value;
        };
        
        // REAL
        
        class RealNode : public AbstractNode{
        public:
            RealNode(size_t pos, size_t length, TSREAL v) : AbstractNode(pos, length),  _value(v){}
            virtual ~RealNode(){}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            const TSREAL _value;
        };
        
        // STRING
        
        class StringNode : public AbstractNode{
        public:
            StringNode(size_t pos, size_t length, ClassNode* sc, const std::string& v)
                : AbstractNode(pos, length), _resultingNode(NULL), _stringClass(sc), _value(v){}
            
            virtual ~StringNode();
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            AbstractNode* _resultingNode;
            ClassNode* _stringClass;
            const std::string _value;
        };
        
        class MakeStringNode : public AbstractNode{
        public:
            MakeStringNode(size_t pos, size_t length, const std::string& v) : AbstractNode(pos, length), _value(v){}
            virtual ~MakeStringNode(){}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:

            const std::string _value;
        };

        class NullReferenceNode : public AbstractNode{
        public:
            NullReferenceNode(size_t pos, size_t length) : AbstractNode(pos, length) { _cachedType=type::BasicType::Any; }
            virtual ~NullReferenceNode(){}

            void useAs(type::Type* t);

            virtual void semanticTraverse();

            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();

            virtual NODE_TYPE getNodeType(){return N_NULL_REF;}
        };
    }
}

#endif /* defined(__TIMScript__ConstNodes__) */
