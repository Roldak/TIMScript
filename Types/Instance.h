//
//  Instance.h
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Instance__
#define __TIMScript__Instance__

#include <iostream>
#include "Class.h"

namespace ts{
    namespace objects{
        
        class Instance : public Object{
        public:
            Instance(Class* c);
            Instance(Class* c, char* memPool);
            virtual ~Instance();
            
            virtual void freeThis(ExecutionContext* ctx){
                _class->deleteInstance(this, ctx);
            }
            
            inline TSDATA getAttr(size_t index){return _attrs[index];}
            inline void setAttr(size_t index, TSDATA data){_attrs[index]=data;}
            
            inline TSDATA getVirtual(size_t index){return _class->getVirtual(index);}
            inline TSDATA getImplementation(size_t interfaceID, size_t index){return _class->getImplementation(interfaceID, index);}
            
            inline Class* getClass(){return _class;}

            bool instanceOf(objects::Class* clss);
            
            // GC
            
            virtual void gcPushRefs(std::vector<Object*>& objs);
            virtual const size_t size(){return sizeof(Instance);}
            
        private:
            
            TSDATA* _attrs;
            Class* _class;
            bool _scoped;
        };
        
    }
}

#endif /* defined(__TIMScript__Instance__) */
