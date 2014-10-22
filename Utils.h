//
//  Utils.h
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#ifndef TIMScript_Utils_h
#define TIMScript_Utils_h

#include <sstream>

#define GC_INITIAL_MAXSIZE 1024
#define INITIAL_STACK_SIZE 10000
#define INITIAL_LOCALS_SIZE 10000

#define PTR_SIZE sizeof(void*)

namespace ts {
    namespace objects{
        class Instance;
    }

    struct SymbolLocation{
        enum Pool{K_POOL, N_POOL, NOT_FOUND};
        
        SymbolLocation() : index(0), pool(NOT_FOUND) {}
        SymbolLocation(size_t i, Pool p) : index(i), pool(p) {}
        SymbolLocation(const SymbolLocation& other) : index(other.index), pool(other.pool) {}

        const size_t index;
        const Pool pool;
    };

    struct ImplementationLocation{

        ImplementationLocation() : Empty(true), interface(NULL), interfaceID(0), index(0) {}
        ImplementationLocation(void* intf, size_t id, size_t i) : Empty(false), interface(intf), interfaceID(id), index(i) {}

        bool Empty;

        void* interface;
        size_t interfaceID;
        size_t index;
    };
}

template<unsigned int size> struct AdaptedType   { typedef int adapted_int; typedef float adapted_real; };
template<>                  struct AdaptedType<4>{ typedef int32_t adapted_int; typedef float adapted_real; };
template<>                  struct AdaptedType<8>{ typedef int64_t adapted_int; typedef double adapted_real; };

typedef bool TSBOOL;
typedef AdaptedType<PTR_SIZE>::adapted_int TSINT;
typedef AdaptedType<PTR_SIZE>::adapted_real TSREAL;
typedef void* TSREF;
typedef std::string* TSSTRING;

typedef unsigned long long TSINSTR;

union TSDATA {
    TSINT Int;
    TSREAL Real;
    TSREF Ref;
    TSSTRING Str;
    ts::objects::Instance* Instance;
};

union INSTRtoREAL{
    TSINSTR Instr;
    TSREAL Real;
};

inline TSREAL instr_to_r(TSINSTR i){
    INSTRtoREAL u;
    u.Instr=i;
    return u.Real;
}

inline TSINSTR r_to_instr(TSREAL r){
    INSTRtoREAL u;
    u.Real=r;
    return u.Instr;
}

size_t nbOfArgs(TSINSTR instr, const TSINSTR* rest);
bool isBranch(TSINSTR);

template<typename T>
inline std::string T_toString(T val){
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

template<typename T>
inline T String_toT(const std::string& val){
    std::istringstream oss(val);
    T res;
    oss >> res;
    return res;
}

template<typename T>
struct T_Array{
    T_Array() : _count(0), _array(NULL) {}
    T_Array(size_t c, T* a) : _count(c), _array(a) {}
    size_t _count;
    T* _array;
};

typedef T_Array<size_t> size_t_Array;
typedef T_Array<TSDATA> data_Array;

enum DEBUG_TYPE{T_BOOL=0, T_INT, T_REAL, T_REF};

#endif
