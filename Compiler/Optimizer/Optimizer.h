//
//  Optimizer.h
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Optimizer__
#define __TIMScript__Optimizer__

#include <iostream>
#include <vector>
#include <list>
#include "Utils.h"
#include "CodePattern.h"

namespace ts{
    namespace opt{
        
        class Optimizer{
        public:
            
            Optimizer(std::vector<TSINSTR>& prog) : _prg(prog){}
            
            void optimize();
            
        private:
            
            void makeBranchesList();
            void insertBranch(TSINSTR src, TSINSTR dest);
            void adjustBranches(size_t ip, int offset);
            
            // optimizations
            
            void opCodeOptimizations();
            void constantsEvaluation();
            
            void performReplacement(const std::vector<CodePattern*>& pattern, bool reset=false);
            
            std::list<std::pair<TSINSTR, TSINSTR>> _brSrc_Dest;
            std::vector<TSINSTR>& _prg;
            
        };
        
    }
}

#endif /* defined(__TIMScript__Optimizer__) */
