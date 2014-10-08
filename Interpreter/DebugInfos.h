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

    class DebugInfos{
    public:

        DebugInfos(const std::vector<std::string*>& varNames) : _varNames(varNames) {}



    private:

        const std::vector<std::string*> _varNames;

    };
}

#endif /* defined(__TIMScript__DebugInfos__) */
