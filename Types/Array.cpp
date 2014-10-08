//
//  Array.cpp
//  TIMScript
//
//  Created by Romain Beguet on 27.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Array.h"

namespace ts {
    namespace objects{
        
        Array::Array(TSINT length, bool refs) : _array((size_t)length), _areRefs(refs)
        {
        }
        
        Array::~Array(){

        }
        
        // GC
        
        void Array::gcPushRefs(std::vector<Object*>& objs){
            if (_areRefs){
                for (size_t i=0; i<_array.size(); ++i){
                    Object* obj=(Object*)_array[i].Ref;
                    if (obj && !obj->marked())
                        objs.push_back(obj);
                }
            }   
        }
    }
}
