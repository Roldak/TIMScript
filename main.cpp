//
//  main.cpp
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

/*
 TODO:
-DONE. [->>>> fix the GC !! <<<<-]
    -function/method overload
    -generics
-DONE. [interfaces]
    -fill up the stdlib
-DONE. [add some concurrency stuff]
-DONE. [AST nodes should store their initial position, and handle errors correctly]
    -LANGUAGE DOCUMENTATIOOOONNN !!!!!
-DONE. [remove dynamic casts]
-DONE. [Tuples]
    -Structures (value types). (struct Point[x:int, y:int])
-DONE. [solve captures bug]
-DONE. [allow stack allocations (destroyed at the end of the scope) (syntax : x=myClass(2)/x=scoped myClass(2) ?, instead of x=new myClass(2))]
    -complete possible operators (<=, >=, !, +=, -=, *=, /=, <<, >>)
-DONE. [rewrite interpreter (should not allocate on 'run' method (maybe at the creation of the ExecutionContext?))]
-DONE. [fix gc bug with function-returned references]
    -implicit constructors
    -deallocate all scopes when explicit return keyword is used in the middle of the function. (and break, continue, throw)
    -gc should traverse scoped objects
-DONE. [add the possibility to specify custom destructors]
-DONE. [fix bug with virtual functions redefining native functions with user functions or vice versa]
    -create tools ( >>debugger<< , etc)
    -import (source (DONE) / class / dll) feature
-DONE. [match statement]
    (-add type syntax, 'T*' (or '[T]', or 'T?' ?), which indicates that T may have a null value. T cannot.)
    -(zero-cost) exceptions (throw/try/catch, stack trace, should work even when thrown from imported modules (dlls))
    -definitions names should be stored in functions and class objects, so they are available at run time.
    -break and continue loop
-DONE. [operator instanceof]
-DONE. [must store the stack pointer corresponding to no objects on the stack when a function is called (to deal with returns and throws when sp!=0)]
-DONE. [foreach loop]
    -null
    -default values in functions
    -add special operator ('!' ?) which can be used in a function signature to express the requirement of a non null reference for the designated argument.
    -create an convenient interface for the library (1 header + lib file), to compile, run, debug TS programs from C++.
 */
/*
#include "TSEngine.h"

int main(){

    ts::Engine tseng;

    std::cerr<<tseng.run("begin{ print 1}").Int<<std::endl;

    ts::Compiler* cmp = tseng.newCompiler();

    cmp->compile("class Test[ x: int ] extends Object { new()=>{ x = 2 } }");
    cmp->compile("class Hihi[]{ def static lala()=>{ print new Test() } }");
    cmp->compile("begin{ Hihi.lala(); STest.ok() }");
    cmp->import("test.tsf");

    ts::Class* Clss     = cmp->newClass("SpecialVector", {"x:int", "y:int", "o:Test"}, "Object");
    ts::Class* String   = cmp->getClass("String");
    ts::Class* Object   = cmp->getClass("Object");
    ts::Class* Test     = cmp->getClass("Test");

    ts::Method* objToString = object->method("toString");



    ts::Function* constr = clss->newConstructor([Clss, Test](Execution* ctx, TSDATA* args){
        ts::objects::Instance* self = args[0];
        self->setAttr(0, 0);
        self->setAttr(1, 1);
        self->setAttr(2, ctx->instantiate(Test, ));
        return self;
    });

    ts::Function* specVecConstr = clss->newConstructor([String, objToString](ExecutionContext* ctx, TSDATA* args){
        TSDATA myString = ctx->newString(string, "");
        ctx->
    });


}
*/
