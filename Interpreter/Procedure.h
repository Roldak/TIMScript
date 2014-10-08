//
//  Procedure.h
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Procedure__
#define __TIMScript__Procedure__

#include <iostream>
#include "Utils.h"
#include "ExceptionTable.h"

/* contains the bytecode of a function, as well as the estimated stack size and 
 the number of local variable of the function. */

namespace ts{

    class Procedure{
    public:
        Procedure(const std::string& name);

        Procedure(const size_t st_sz,
                  const size_t lcl_sz,
                  const TSINSTR* bytecode,
                  const size_t bytecodeLength,
                  size_t_Array** refsLoc,
                  ExceptionTable** exTable,
                  const std::string& name);
        
        ~Procedure(){/*delete[] _bytecode;*/}
        
        inline const size_t stackSize() const{return _stack_size;}
        inline const size_t numberOfLocals() const{return _local_var_nb;}
        inline const TSINSTR* byteCode() const{return _bytecode;}
        inline const size_t byteCodeLength() const{return _bytecode_length;}
        
        inline void setRefMap(size_t_Array** refMap){_refsLocations=refMap;}
        inline size_t_Array* getRefMap() const{return *_refsLocations;}

        inline void setExTable(ExceptionTable** exTable){_exTable=exTable;}
        inline ExceptionTable* getExTable() const{return *_exTable;}

        inline const std::string& name() const{return _name;}
        
    private:
        
        const size_t _stack_size, _local_var_nb;
        const TSINSTR* _bytecode;
        const size_t _bytecode_length;
        size_t_Array** _refsLocations;
        ExceptionTable** _exTable;
        const std::string _name;
    };

}

#endif /* defined(__TIMScript__Procedure__) */
