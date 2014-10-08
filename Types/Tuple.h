//
//  Tuple.h
//  TIMScript
//
//  Created by Romain Beguet on 06.03.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Tuple__
#define __TIMScript__Tuple__

#include <iostream>
#include "Object.h"
#include "Utils.h"

namespace ts{
    namespace objects{
        
        class Tuple : public Object{
        public:
            Tuple(size_t nb, size_t_Array** refsLocations);
            Tuple(TSDATA* tab, size_t_Array** refsLocations);
            virtual ~Tuple();
            
            inline TSDATA getAttr(size_t index){return _elems[index];}
            inline void setAttr(size_t index, TSDATA data){_elems[index]=data;}
            
            // GC
            
            virtual void gcPushRefs(std::vector<Object*>& objs);
            virtual const size_t size(){return sizeof(Tuple);}
            
        protected:
            
            TSDATA* _elems;
            size_t_Array** _refsLocations;
        };
        
    }
}

#endif /* defined(__TIMScript__Tuple__) */
