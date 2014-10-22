//
//  Class.h
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef __TIMScript__Class__
#define __TIMScript__Class__

#include <iostream>
#include <vector>
#include "Utils.h"
#include "Object.h"
#include "DebugInfos.h"

namespace ts{
    namespace objects{
        
        class Class : public Object{
        public:
            Class(size_t nb, Class** superClass, TSDATA* vt, TSDATA** it,
                  std::vector<size_t> vec, size_t destr, DebugInfos** infos, const std::string& name);
            virtual ~Class(){delete _vtable;}
            
            inline size_t numberOfAttributes(){return _attrNumber;}
            inline TSDATA getVirtual(size_t index){return _vtable[index];}
            inline TSDATA getImplementation(size_t interfaceID, size_t index){
                return _itable[interfaceID][index];
            }
            
            TSDATA newInstance();
            void deleteInstance(objects::Instance* instance, ExecutionContext* ctx);
            
            inline const std::string& name(){return _name;}
            inline Class* superClass(){return _superClass ? *_superClass : NULL;}

            inline DebugInfos* getDebugInfos(){return _debugInfos ? *_debugInfos : NULL;}

            // GC
            
            void pushRefsInstance(objects::Instance* instance, std::vector<Object*>& objs);
            
            virtual void gcPushRefs(std::vector<Object*>& objs);
            virtual const size_t size(){return sizeof(Class);}
            
        private:
            
            size_t _attrNumber;
            TSDATA* _vtable;
            TSDATA** _itable;
            std::vector<size_t> _refsAmongAttrs;
            size_t _destrIndex;

            DebugInfos** _debugInfos;

            const std::string _name;

            Class** _superClass;
        };
        
    }
}

#endif /* defined(__TIMScript__Class__) */
