//
//  DebugInfoNode.cpp
//  TIMScript
//
//  Created by Romain Beguet on 29.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "DebugInfoNode.h"
#include "Scope.h"

namespace ts{
    namespace nodes{

        DebugInfoNode::DebugInfoNode(const std::vector<std::pair<std::string, cmplr::Variable*>>& vars)
            : AbstractNode(0, 0),  _vars(vars)
        {

        }

        void DebugInfoNode::semanticTraverse(){}
        
        void DebugInfoNode::pushBytecode(std::vector<TSINSTR>& program){

            for(size_t i=0; i<_vars.size(); ++i){
                program.push_back(MK_STRING);

                std::string name=_vars[i].first;

                program.push_back(name.size());

                for(size_t j=0; j<name.size(); ++j){
                    program.push_back(name[j]);
                }
            }

            program.push_back(MK_DBG_INFO);
            program.push_back(_vars.size());
        }

        std::string DebugInfoNode::toString(){
            return "";
        }
        
    }
}
