//
//  GC.cpp
//  TIMScript
//
//  Created by Romain Beguet on 09.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "GC.h"

namespace ts{

    GarbageCollector::GarbageCollector(size_t maxSize, data_Array* roots, ExecutionContext* ctx)
    : _currentSize(0), _maxSize(maxSize), _roots(roots), _ctx(ctx), _temporaries(5), _backup(NULL){
    }
    GarbageCollector::~GarbageCollector(){
        if (_backup) {
            
            for (size_t i=0; i<_items.size(); ++i)
                _backup->trace(_items[i]);
            
            for (size_t i=0; i<_temporaries.size(); ++i)
                if (_temporaries[i])
                    _backup->trace(_temporaries[i]);
            
        }else{
            
            for (size_t i=0; i<_items.size(); ++i)
                delete _items[i];
            
            for (size_t i=0; i<_temporaries.size(); ++i)
                delete _temporaries[i];
            
        }
    }
    
    void GarbageCollector::reset(data_Array* roots, ExecutionContext* ctx){
        _roots=roots;
        _ctx=ctx;
    }
    
    void GarbageCollector::trace(objects::Object* item){
 
            // this prevents this newly created object from being freed by the gc in case its addition to it triggered a
            // collection before it could be store in a variable accessible from a root, (e.g. a temporary object)
        
        item->setTemporary();
        
            // add the item to the temporaries objects vector
        
        emplaceTemporaryItem(item);
        
            // also add it to the collection of every objects
        
        _items.push_back(item); 
        
            // readjust the current heap size
        
        _currentSize=_currentSize+item->size();
        
            // perform colllection if the size is too big

        if (_currentSize>_maxSize)
            performCollection();
    }
    
    void GarbageCollector::store(objects::Object* item){
        
            // do nothing if the objects was already stored in a variable
        if (!item)
            return;

        if (item->marked()!=objects::TEMPORARY)
            return;
        
            // the object is not temporary anymore
        
        item->unsetTemporary();
        
            // make the slot available
        
        _temporaries[item->getIndex()]=NULL;
    }
    
    void GarbageCollector::makeTemp(objects::Object* item){
        if(item->marked()==objects::TEMPORARY)
            return;
        
        item->setTemporary();
        emplaceTemporaryItem(item);
    }
    
    void GarbageCollector::forceCollection(){
        performCollection();
    }
    
    // to avoid having a constantly size-growing vector of temporaries, we emplace each new item in an available slot if possible.
    
    void GarbageCollector::emplaceTemporaryItem(objects::Object* item){
        for (size_t i=0, s=_temporaries.size(); i<s; ++i) {
            if (_temporaries[i]==NULL) {
                _temporaries[i]=item;
                item->setIndex(i);
                return;
            }
        }
        item->setIndex(_temporaries.size());
        _temporaries.push_back(item);
    }
    
    void GarbageCollector::performCollection(){
        markStep();
        sweepStep();
    }

    void GarbageCollector::markStep(){
        std::vector<objects::Object*> objs;
        objs.reserve(_items.size()); // there will be at max _curNbItems referenced
        
            // first, push all the objects pointed by the roots variable
        
        for (size_t i=0; i<_roots->_count; ++i) {
            objects::Object* pointer((objects::Object*)_roots->_array[i].Ref);
            if (pointer)
                objs.push_back(pointer);
        }
        
            // add the temporaries
        
        for (size_t i=0, e=_temporaries.size(); i<e; ++i)
            if (_temporaries[i]!=NULL)
                objs.push_back(_temporaries[i]);
        
            // next, traverse all the collections and for each objects, add to the collection all its inner references

        for (size_t i=0; i<objs.size(); ++i) {
            objs[i]->gcPushRefs(objs);
            objs[i]->mark();
        }

    }
    
    void GarbageCollector::sweepStep(){
        size_t newSize=_currentSize;
        size_t last=0;
    
        for (size_t i=0, end=_items.size(); i<end; ++i, ++last) {
            while (!_items[i]->marked()) {
                newSize-=_items[i]->size();
                _items[i]->freeThis(_ctx);
                delete _items[i];
                ++i;
                
                if(i>=end)
                    goto out;
            }
            _items[i]->unmark();
            _items[last]=_items[i];
        }
        
    out:
        
        _currentSize=newSize;
        _items.resize(last);
        
        _maxSize=_maxSize+_currentSize;
    }
}
