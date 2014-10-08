//
//  Optimizer.cpp
//  TIMScript
//
//  Created by Romain Beguet on 19.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Optimizer.h"
#include "Bytecode.h"

namespace ts{
    namespace opt{
        
        void Optimizer::optimize(){
            makeBranchesList();
            
            opCodeOptimizations();
            constantsEvaluation();
        }
        
        void Optimizer::makeBranchesList(){
            _brSrc_Dest.clear();
            
            size_t ip=0;
            while (ip<_prg.size()) {
                if (isBranch(_prg[ip]))
                    insertBranch(ip, _prg[ip+1]);
                
                ip+=nbOfArgs(_prg[ip], _prg.data()+ip+1);
                ++ip;
            }
        }
        
        void Optimizer::insertBranch(TSINSTR src, TSINSTR dest){
            auto it=_brSrc_Dest.begin();
            while (it!=_brSrc_Dest.end() && src<it->first)
                ++it;
            
            _brSrc_Dest.insert(it, std::pair<TSINSTR, TSINSTR>(src, dest));
        }
        
        
        void Optimizer::adjustBranches(size_t ip, int offset){
            for (auto& br : _brSrc_Dest) {
                if (br.first>ip)
                    br.first=(size_t)((int)br.first-offset);
                if (br.second>ip)
                    br.second=(size_t)((int)br.second-offset);
                
                if(isBranch(_prg[br.first]))
                    _prg[br.first+1]=br.second;
            }
        }
        
        void Optimizer::opCodeOptimizations(){
            performReplacement(CodePattern::CastOptimizationPatterns);
            performReplacement(CodePattern::IncrementationOptimizationPatterns);
            performReplacement(CodePattern::BranchesOptimizations);
            performReplacement(CodePattern::TautologyOptimizations);
        }
        
        void Optimizer::constantsEvaluation(){
            performReplacement(CodePattern::ConstantsOptimizations, true);
        }
        
        void Optimizer::performReplacement(const std::vector<CodePattern*>& pattern, bool reset){
            size_t ip=0;
            while (ip<_prg.size()) {
                int offsetSize=0;
            redo:
                for (auto it=pattern.begin(); it!=pattern.end(); ++it) {
                    if((*it)->length()+ip<_prg.size()){
                        if((*it)->tryMatchAndReplace(_prg, ip)){
                            offsetSize+=(*it)->length()-(*it)->replaceLength();
                            goto redo;
                        }
                    }
                }
                
                if (offsetSize>0){
                    adjustBranches(ip, offsetSize);
                    if (reset) {
                        ip=0;
                        continue;
                    }
                }
                
                ip+=nbOfArgs(_prg[ip], _prg.data()+ip+1);
                ++ip;
            }
        }
    }
}