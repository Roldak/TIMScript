//
//  CastUtils.h
//  TIMScript
//
//  Created by Romain Beguet on 04.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__CastUtils__
#define __TIMScript__CastUtils__

#include <iostream>
#include "AssignableNode.h"
#include "ConstNodes.h"
#include "ArrayNodes.h"
#include "InstanceNodes.h"
#include "TupleNodes.h"
#include "StructureNodes.h"
#include "DotOperation.h"
#include "ClassNode.h"
#include "InterfaceNode.h"
#include "InitializerListNode.h"
#include "VariableNode.h"

namespace ts{
    namespace nodes{
        
        template<class T>
        static T getIf(AbstractNode* n){
            return NULL;
        }
        
        template<>
        AssignableNode* getIf<AssignableNode*>(AbstractNode* n){
            NODE_TYPE t=n->getNodeType();
            if( t==N_ASSIGNABLE || t==N_LOCAL_VARIABLE_NODE ||
                t==N_ATTRIBUTE_ACCESS || t==N_FIELD_ACCESS){
                return (AssignableNode*)n;
            }
            return NULL;
        }
        
        template<>
        NewArrayNode* getIf<NewArrayNode*>(AbstractNode* n){
            return n->getNodeType()==N_NEW_ARRAY ? (NewArrayNode*)n : NULL;
        }
        
        template<>
        NewInstanceNode* getIf<NewInstanceNode*>(AbstractNode* n){
            return n->getNodeType()==N_NEW_INSTANCE ? (NewInstanceNode*)n : NULL;
        }
        
        template<>
        NewTupleNode* getIf<NewTupleNode*>(AbstractNode* n){
            return n->getNodeType()==N_NEW_TUPLE ? (NewTupleNode*)n : NULL;
        }

        template<>
        DotOperationNode* getIf<DotOperationNode*>(AbstractNode* n){
            return n->getNodeType()==N_DOT_OPERATION ? (DotOperationNode*)n : NULL;
        }
    
        template<>
        VirtualAccessNode* getIf<VirtualAccessNode*>(AbstractNode* n){
            return n->getNodeType()==N_VIRTUAL_ACCESS ? (VirtualAccessNode*)n : NULL;
        }

        template<>
        ImplementationAccessNode* getIf<ImplementationAccessNode*>(AbstractNode* n){
            return n->getNodeType()==N_IMPLEMENTATION_ACCESS ? (ImplementationAccessNode*)n : NULL;
        }

        template<>
        DefinitionReferenceNode* getIf<DefinitionReferenceNode*>(AbstractNode* n){
            return n->getNodeType()==N_DEFINITION_REFERENCE ? (DefinitionReferenceNode*)n : NULL;
        }

        template<>
        ClassNode* getIf<ClassNode*>(AbstractNode* n){
            return n->getNodeType()==N_CLASS ? (ClassNode*)n : NULL;
        }

        template<>
        InterfaceNode* getIf<InterfaceNode*>(AbstractNode* n){
            return n->getNodeType()==N_INTERFACE ? (InterfaceNode*)n : NULL;
        }
        
        template<>
        InitializerListNode* getIf<InitializerListNode*>(AbstractNode* n){
            return n->getNodeType()==N_INITIALIZERLIST ? (InitializerListNode*)n : NULL;
        }
        
        template<>
        LocalVariableNode* getIf<LocalVariableNode*>(AbstractNode* n){
            return n->getNodeType()==N_LOCAL_VARIABLE_NODE ? (LocalVariableNode*)n : NULL;
        }

        template<>
        AttributeAccessNode* getIf<AttributeAccessNode*>(AbstractNode* n){
            return n->getNodeType()==N_ATTRIBUTE_ACCESS ? (AttributeAccessNode*)n : NULL;
        }

        template<>
        StructureFieldAccessNode* getIf<StructureFieldAccessNode*>(AbstractNode* n){
            return n->getNodeType()==N_FIELD_ACCESS ? (StructureFieldAccessNode*)n : NULL;
        }

        template<>
        NullReferenceNode* getIf<NullReferenceNode*>(AbstractNode* n){
            return n->getNodeType()==N_NULL_REF ? (NullReferenceNode*)n : NULL;
        }
    }
}

#endif /* defined(__TIMScript__CastUtils__) */
