//
//  UnaryOperationNodes.h
//  TIMScript
//
//  Created by Romain Beguet on 04.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__UnaryOperationNodes__
#define __TIMScript__UnaryOperationNodes__


#include <iostream>
#include "AbstractNode.h"
#include "Bytecode.h"
#include "AssignableNode.h"

namespace ts{
    namespace nodes{
        
        template<OP_CODE code>
        std::string getUNROPCODElitteral(){
            switch (code) {
                case INC_I_1:   return "++";
                case NEG_I:     return "-";
                default:
                    break;
            }
            return "";
        }
        
        template<OP_CODE onInt, OP_CODE onReal>
        class UnaryOperation : public AbstractNode{
        public:
            
            UnaryOperation(size_t pos, size_t length, AbstractNode* e) : AbstractNode(pos, length), _exp(e) {}
            virtual ~UnaryOperation(){
                delete _exp;
            }
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            virtual std::string toString();
            
        protected:
            AbstractNode* _exp;
        };
        
        template<OP_CODE onInt, OP_CODE onReal>
        class UnaryModifierOperation : public UnaryOperation<onInt, onReal>{
        public:
            UnaryModifierOperation(size_t pos, size_t length, AssignableNode* e)
                : UnaryOperation<onInt, onReal>(pos, length, e) {}
            
            virtual void semanticTraverse();
            virtual void pushBytecode(std::vector<TSINSTR>& program);
            
        };
        
        typedef UnaryOperation<NEG_I, NEG_R> UnaryMinus;
        typedef UnaryModifierOperation<INC_I_1, INC_R_1> UnaryPlusPlus;
        typedef UnaryModifierOperation<DEC_I_1, DEC_R_1> UnaryMinusMinus;
    }
}

#endif /* defined(__TIMScript__UnaryOperationNodes__) */
