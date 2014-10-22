//
//  DebugInfos.h
//  TIMScript
//
//  Created by Romain Beguet on 30.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__DebugInfos__
#define __TIMScript__DebugInfos__

#include <iostream>
#include <vector>
#include "Utils.h"

namespace ts{

    struct DebugVar{
        size_t index;
        std::string* name;
        DEBUG_TYPE type;
    };

    typedef std::vector<DebugVar> VarDeclTable;

    class DebugInfos{
    public:

        DebugInfos(const VarDeclTable& varDecls) : _vars(varDecls) {}

        inline const VarDeclTable& getVars(){return _vars;}

    private:

        const VarDeclTable _vars;

    };
}

#endif /* defined(__TIMScript__DebugInfos__) */
