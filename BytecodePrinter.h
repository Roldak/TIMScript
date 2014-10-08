//
//  BytecodePrinter.h
//  TIMScript
//
//  Created by Romain Beguet on 29.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__BytecodePrinter__
#define __TIMScript__BytecodePrinter__

#include <iostream>
#include "Utils.h"
#include "Procedure.h"

namespace ts{
    
    class BytecodePrinter{
    public:
        
        static std::string translate(const Procedure* proc);
        
    private:
        
        static std::string instrName(TSINSTR instr);
        
    };
    
}

#endif /* defined(__TIMScript__BytecodePrinter__) */
