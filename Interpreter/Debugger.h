//
//  Debugger.h
//  TIMScript
//
//  Created by Romain Beguet on 20.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Debugger__
#define __TIMScript__Debugger__

#include <iostream>
#include <vector>
#include "Utils.h"

namespace ts{
    class AbstractDebugger{
    public:

        void setBreakPointBetween(size_t startOfLine, size_t endOfLine);
        void removeBreakPoint(size_t index);

        virtual bool shouldBreak(size_t pos);
        virtual void updateDebuggingInfo(size_t pos, TSDATA* locals, size_t localsCount) = 0;
        virtual void functionCalled(const std::string& funcName, void* address) = 0;
        virtual void functionReturned() = 0;
        virtual void readMessage() = 0;

    protected:

        std::vector<std::pair<size_t, size_t>> _breakPoints;

    };
}

#endif /* defined(__TIMScript__Debugger__) */
