//
//  CompilationContext.h
//  TIMScript
//
//  Created by Romain Beguet on 30.09.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__CompilationContext__
#define __TIMScript__CompilationContext__

#include <iostream>
#include "Compiler.h"

namespace ts {

    namespace cmplr{

        class CompilationContext{
        public:

            static void enable(Compiler* compiler){ if(!cmp) cmp=compiler; }
            static void disable(){ cmp=NULL; }

            static const Compiler* getCompiler() { return cmp; }

            static bool inDebugMode() { if(cmp) return cmp->inDebugMode(); else return false; }

        private:
            static Compiler* cmp;
        };

    }
}

#endif /* defined(__TIMScript__CompilationContext__) */
