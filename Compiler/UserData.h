//
//  UserData.h
//  TIMScript
//
//  Created by Romain Beguet on 19.02.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef TIMScript_UserData_h
#define TIMScript_UserData_h

#include <iostream>

namespace ts {
    
    namespace objects{
        class NativeData;
    }
    namespace type{
        class Type;
    }
    
    struct UserData{
        UserData(size_t i, objects::NativeData* dat, type::Type* t) : _index(i), _data(dat), _type(t) {}
        UserData() : _index(0), _data(NULL), _type(NULL) {}
        
        size_t _index;
        objects::NativeData* _data;
        type::Type* _type;
    };
    
    struct UserDataInfo{
        UserDataInfo(objects::NativeData* dat, type::Type* t) : _data(dat), _type(t) {}
        UserDataInfo(objects::NativeData* dat, const std::string& str) : _data(dat), _type(NULL), _typestr(str) {}
        UserDataInfo(type::Type* t) : _data(NULL), _type(t) {}
        UserDataInfo(const std::string& str) : _data(NULL), _type(NULL), _typestr(str) {}
        
        objects::NativeData* _data;
        type::Type* _type; // either this one
        const std::string _typestr; // or this one
    };
}

#endif
