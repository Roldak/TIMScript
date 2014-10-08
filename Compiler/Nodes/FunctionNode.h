//
//  FunctionNode.h
//  TIMScript
//
//  Created by Romain Beguet on 05.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__FunctionNode__
#define __TIMScript__FunctionNode__

#include <iostream>
#include "Function.h"
#include "AbstractNode.h"
#include "DebugInfoNode.h"

namespace ts {
    namespace nodes{
        class LocalVariableNode;
        
        class Argument{
        public:
            
            Argument(const std::string& n, type::Type* t) : _type(t), _name(n) {}
            
            inline const std::string& name(){return _name;}
            inline type::Type* type(){return _type;}
            
        private:
            type::Type* _type;
            std::string _name;
        };
        
        class ReturnNode : public AbstractNode{
        public:
            
            ReturnNode(size_t pos, size_t length, AbstractNode* n) : AbstractNode(pos, length), _node(n) {}
            virtual ~ReturnNode() {delete _node;}
            
            inline AbstractNode* node(){return _node;}
            inline void setNode(AbstractNode* n){_node=n;}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            AbstractNode* _node;
            
        };
        
        class ExplicitReturnNode : public AbstractNode{
        public:
            
            ExplicitReturnNode(size_t pos, size_t length, AbstractNode* n) : AbstractNode(pos, length), _node(n) {}
            virtual ~ExplicitReturnNode() {delete _node;}
            
            inline AbstractNode* node(){return _node;}
            inline void setNode(AbstractNode* n){_node=n;}
            
            virtual void semanticTraverse();
            
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            AbstractNode* _node;
            
        };
        
        class FunctionNode : public AbstractNode{
        public:
            
            FunctionNode(size_t pos, size_t length,
                         const std::vector<Argument*>& a, const std::vector<LocalVariableNode*>& c,
                         type::Type* t, ReturnNode* b,
                         size_t refMapTupleIndex,
                         size_t exTableIndex,
                         size_t nbLocals, size_t dbgInfoIndex=std::string::npos);
            
            virtual ~FunctionNode() {}
            
            void setBody(AbstractNode* node);
            AbstractNode* body();
            
            inline type::FunctionType* getSignature(){return _signature;}
            
            inline const std::vector<Argument*>& arguments(){return _args;}

            inline size_t getDebugInfoNodeIndex(){ return _dbgInfoIndex; }
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        private:
            
            type::FunctionType* _signature;
            
            std::vector<Argument*> _args;
            std::vector<LocalVariableNode*> _captures;
            type::Type* _retType;
            ReturnNode* _body;
            size_t _refMapTupleIndex;
            size_t _exTableIndex;
            size_t _dbgInfoIndex;
            size_t _nbLocals;
        };

    }
}


#endif /* defined(__TIMScript__FunctionNode__) */
