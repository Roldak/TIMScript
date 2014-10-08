//
//  Instance.cpp
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Instance.h"
#include <cstdlib>

namespace ts {
    namespace objects{

        Instance::Instance(Class* c) : _class(c), _scoped(false) {
            _attrs=(TSDATA*)calloc(c->numberOfAttributes(), sizeof(TSDATA));
        }

        Instance::Instance(Class* c, char* memPool) : _class(c), _scoped(true){
            _attrs=(TSDATA*)memPool;
        }

        Instance::~Instance(){
            if (!_scoped)
                free(_attrs);
        }

        bool Instance::instanceOf(objects::Class* clss){
            objects::Class* objClass=_class;
            while(objClass){
                if(objClass==clss)
                    return true;
                objClass=objClass->superClass();
            }
            return false;
        }

        void Instance::gcPushRefs(std::vector<Object*>& objs){
            _class->pushRefsInstance(this, objs);
        }
    }
}
