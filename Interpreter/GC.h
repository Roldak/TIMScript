//
//  GC.h
//  TIMScript
//
//  Created by Romain Beguet on 09.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__GC__
#define __TIMScript__GC__

#include <iostream>
#include <list>
#include <vector>
#include "Utils.h"

#include "Object.h"

namespace ts{
    class ExecutionContext;
    
    class GarbageCollector{
    public:
        GarbageCollector(size_t maxSize, data_Array* roots, ExecutionContext* ctx);
        ~GarbageCollector();
        
        void trace(objects::Object* item);
        void store(objects::Object* item);
        void fastStore(objects::Object* item);
        void makeTemp(objects::Object* item);
        void forceCollection();
        
        void reset(data_Array* roots, ExecutionContext* ctx);
        
        void setBackupGC(GarbageCollector* gc){_backup=gc;}

        bool contains(TSREF ref);
        size_t getCurrentSize();
        size_t getMaxSize();
        
    private:

        void emplaceTemporaryItem(objects::Object* item);
        void performCollection();
        void markStep();
        void sweepStep();
        
        size_t _maxSize;
        size_t _currentSize;
        std::vector<objects::Object*> _items;
        std::vector<objects::Object*> _temporaries;
        data_Array* _roots;
        ExecutionContext* _ctx;
        
        GarbageCollector* _backup;
    };
    
    
    inline void GarbageCollector::fastStore(objects::Object* item){
        // the object is not temporary anymore
        
        item->unsetTemporary();
        
        // make the slot available
        
        _temporaries[item->getIndex()]=NULL;
    }
}

#endif /* defined(__TIMScript__GC__) */
