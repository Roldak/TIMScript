//
//  Class.cpp
//  TIMScript
//
//  Created by Romain Beguet on 18.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include "Class.h"
#include "Instance.h"
#include "Function.h"
#include "Interpreter.h"

namespace ts{
    namespace objects{
        
        Class::Class(size_t nb, Class** superClass, TSDATA* vt, TSDATA **it, std::vector<size_t> vec, size_t destr, DebugInfos **infos, const std::string& name)
            : _attrNumber(nb), _vtable(vt), _itable(it), _refsAmongAttrs(vec),
              _destrIndex(destr), _debugInfos(infos), _superClass(superClass), _name(name)
        {
        }
        
        TSDATA Class::newInstance(){
            return TSDATA{.Ref=new Instance(this)};
        }
        
        void Class::deleteInstance(objects::Instance* instance, ExecutionContext* ctx){
            if (_destrIndex!=std::string::npos) {
                std::vector<TSDATA> args(1);
                args[0].Ref=instance;
                ((objects::Function*)_vtable[_destrIndex].Ref)->call(ctx, args);
            }
        }
        
        void Class::pushRefsInstance(objects::Instance* instance, std::vector<Object*>& objs){
            for (size_t i=0; i<_refsAmongAttrs.size(); ++i){
                Object* obj((Object*)instance->getAttr(_refsAmongAttrs[i]).Ref);
                if(obj && !obj->marked())
                    objs.push_back(obj);
            }
        }
        
        void Class::gcPushRefs(std::vector<Object*>& objs){
            
        }
    }
}
