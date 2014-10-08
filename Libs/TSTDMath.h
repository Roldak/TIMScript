//
//  TSTDMath.h
//  TIMScript
//
//  Created by Romain Beguet on 23.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__TSTDMath__
#define __TIMScript__TSTDMath__

#include <iostream>
#include "TIMScript.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#ifndef M_E
#define M_E           2.7182818284590452353
#endif

namespace ts {
    namespace tstd{
        
        void importMath(cmplr::Compiler& cmp);
        
    }
}

#endif /* defined(__TIMScript__TSTDMath__) */
