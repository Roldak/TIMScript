//
//  main.cpp
//  TIMScript
//
//  Created by Romain Beguet on 28.01.14.
//  Copyright (c) 2014 Romain Beguet. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "Compiler.h"
#include "Bytecode.h"
#include "Interpreter.h"
#include "BytecodePrinter.h"
#include "FunctionBuilder.h"
#include "Instance.h"
#include "ClassNode.h"
#include "InterfaceNode.h"
#include "DefinitionReferenceNode.h"
#include "CompilationExceptions.h"
#include <cmath>
#include "Array.h"

#include "TSTD.h"


using namespace ts;
using namespace std;

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
 */
/*
class TIPCDebugger : public AbstractDebugger{
public:

    TIPCDebugger(size_t port) {}

    virtual void waitForResponse(){

    }

private:

};
*/

/*
int main(int argc, const char * argv[])
{   
    // LOAD FILE

    string fileName;

    if (argc>1)
        fileName=argv[1];
    else    
        fileName="source.tsf";

    ifstream file(fileName.c_str());
    string content;

    while (file.good())
        content+=file.get();
    
    file.close();

    // COMPILATION

    clock_t comp=clock();

    cmplr::Compiler cmp(argv[0], true);

    Program* program;

    try{
        tstd::importSTD(cmp);
        program=cmp.compile(content);
    }catch(ts::exception::ParsingError& ex){
        cerr<<ex.what()<<endl;
        return 0;
    }

    if(argc>2) return 0;

#ifdef DEBUG
    cerr<<endl<<"bytecode : "<<endl<<BytecodePrinter::translate(program->main())<<endl<<endl<<"output : "<<endl;
#endif


    Interpreter itp(program);
    ExecutionContext* ctx=itp.createExecutionContext();

    cout<<endl<<"Compilation Time : "<<(clock()-comp)/(double)CLOCKS_PER_SEC<<endl;
    cout<<"-----Starting Program------"<<endl<<endl;
    
    clock_t exec=clock();
    
    Interpreter::exec<true>(ctx, program->main());
    
    cout<<endl<<endl<<"------Ending Program------"<<endl;
    cout<<"Execution Time : "<<(clock()-exec)/(double)CLOCKS_PER_SEC<<endl<<endl;

#ifdef _WIN32
    //system("pause");
#endif

    return 0;
}
*/
