//
//  ExceptionNodes.cpp
//  TIMScript
//
//  Created by Romain Beguet on 27.05.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "ExceptionNodes.h"
#include "CompilationExceptions.h"
#include "CastNode.h"
#include "Scope.h"

namespace ts{
    namespace nodes{
        
        // THROW NODE

        void ThrowNode::semanticTraverse(){
            _content->semanticTraverse();

            if(!type::getIf<type::ClassType*>(_content->type())){
                throw exception::InvalidThrowValue(_content, _content->type());
            }

            _cachedType=type::BasicType::Void;
        }
    
        void ThrowNode::pushBytecode(std::vector<TSINSTR>& program){
            _content->pushBytecode(program);
            program.push_back(THROW);
        }
        
        std::string ThrowNode::toString(){
            return "throw "+_content->toString();
        }

        // TRY CATCH NODE

        TryCatchNode::TryCatchNode(size_t pos, size_t length, AbstractNode* tryNode)
            : AbstractNode(pos, length), _tryNode(tryNode), _beginTryNodeIP(0), _endTryNodeIP(0) {}

        TryCatchNode::~TryCatchNode(){
            delete _tryNode;

            for(auto& n : _catchNodes){
                delete n.second;
            }
        }

        void TryCatchNode::semanticTraverse(){
            _tryNode->semanticTraverse();
            _cachedType=_tryNode->type();

            for(std::pair<cmplr::Variable*, AbstractNode*>& n : _catchNodes){
                if(!type::getIf<type::ClassType*>(n.first->type())){
                    throw exception::InvalidOperandBinaryIs(n.second, n.first->type(), "Instance", "right");
                }

                n.second->semanticTraverse();
                if(!n.second->type()->equals(_cachedType)){
                    n.second=new CastNode(pos(), len(), n.second, _cachedType);
                    n.second->semanticTraverse();
                }
            }
        }

        void TryCatchNode::pushBytecode(std::vector<TSINSTR>& program){
            _beginTryNodeIP=program.size();
            _tryNode->pushBytecode(program);
            _endTryNodeIP=program.size();

            _catchEntryPoints.clear();
            std::vector<size_t> gotoPositions;

            program.push_back(GOTO);
            gotoPositions.push_back(program.size());
            program.push_back(0);

            for(std::pair<cmplr::Variable*, AbstractNode*>& n : _catchNodes){
                _catchEntryPoints.push_back(program.size());
                n.second->pushBytecode(program);
                program.push_back(GOTO);
                gotoPositions.push_back(program.size());
                program.push_back(0);
            }

            size_t here=program.size();
            for(size_t pos : gotoPositions){
                program[pos]=here;
            }
        }

        std::string TryCatchNode::toString(){
            return "try "+_tryNode->toString();
        }
        
        // EXCEPTION TABLE NODE

        ExTableNode::~ExTableNode(){

        }

        void ExTableNode::semanticTraverse(){

        }

        void ExTableNode::pushBytecode(std::vector<TSINSTR>& program){
            program.push_back(MK_EX_TABLE);
            program.push_back(_tryCatchNodes.size());

            for(TryCatchNode* trycatch : _tryCatchNodes){
                program.push_back(trycatch->getTryBlockIPStart());
                program.push_back(trycatch->getTryBlockIPEnd());

                const std::vector<std::pair<cmplr::Variable*, AbstractNode*>>& catchNodes=trycatch->getCatchNodes();
                const std::vector<size_t>& catchNodesEntryPoints=trycatch->getCatchEntryPoints();

                program.push_back(catchNodes.size());

                size_t i=0;
                for(const std::pair<cmplr::Variable*, AbstractNode*>& pair : catchNodes){
                    program.push_back(pair.first->index());
                    program.push_back(((type::ClassType*)pair.first->type())->getClassDefIndex());
                    program.push_back(catchNodesEntryPoints[i++]);
                }

            }
        }

        std::string ExTableNode::toString(){
            return "";
        }
    }
}
