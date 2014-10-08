//
//  TSTD.h
//  TIMScript
//
//  Created by Romain Beguet on 23.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__TSTD__
#define __TIMScript__TSTD__

#include <iostream>
#include "TIMScript.h"

#include "TSTDPrimitive.h"
#include "TSTDMath.h"
#include "TSTDString.h"
#include "TSTDClock.h"
#include "TSTDThread.h"
#include "TSTDArray.h"
#include "TSTDIterable.h"
#include "TSTDVector.h"
#include "TSTDMap.h"
#include "TSTDFile.h"

namespace ts{
    namespace tstd{
        
        static void importSTD(cmplr::Compiler& cmp){
            importPrimitive(cmp); // import Primitives lib
            importMath(cmp); // import Math lib
            importString(cmp); // import String lib
            importClock(cmp); // import Clock lib
            importThread(cmp); // import Thead lib
            importArray(cmp); // import Array lib
            importIterable(cmp); // import Iterator lib
            importVector(cmp); // import Containers lib
            importMap(cmp); // import Map lib
            importFile(cmp); // import File lib
            /* TODO :
             
                -List
                -Map
                -Set
                -Regexp
             
                ...
             */
        }
        
    }
}

#endif /* defined(__TIMScript__TSTD__) */
