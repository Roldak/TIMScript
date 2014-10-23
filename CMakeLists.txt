project(TIMScript)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(./ SRC_LIST)
aux_source_directory(./Compiler SRC_LIST)
aux_source_directory(./Compiler/Nodes SRC_LIST)
aux_source_directory(./Compiler/Optimizer SRC_LIST)
aux_source_directory(./Interpreter SRC_LIST)
aux_source_directory(./Libs SRC_LIST)
aux_source_directory(./Tokenizer SRC_LIST)
aux_source_directory(./Tokenizer/Tokens SRC_LIST)
aux_source_directory(./Tools SRC_LIST)
aux_source_directory(./Types SRC_LIST)

#add_executable(${PROJECT_NAME} ${SRC_LIST})
add_library(${PROJECT_NAME} STATIC ${SRC_LIST})

set(CMAKE_CXX_FLAGS "-std=c++11 -o3 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")

set(CMAKE_BUILD_TYPE "release")
include_directories( ./
    ./Compiler/
    ./Compiler/Nodes/
    ./Compiler/Optimizer/
    ./Interpreter/
    ./Libs/
    ./Tokenizer/
    ./Tokenizer/Tokens/
    ./Tools/
    ./Types/
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
