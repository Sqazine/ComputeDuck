cmake_minimum_required(VERSION 3.10)

project(ComputeDuck LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

option(BuildLLVMImpl "build llvm impl" ON) 

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cd")

source_group("examples" FILES ${EXAMPLES})

file(GLOB SRC main.cpp)
add_executable(computeduck ${SRC} ${EXAMPLES})

if(MSVC)
target_compile_options(computeduck PRIVATE "/wd4251;")
endif()

if(NOT BuildLLVMImpl)
    option(BuildSDL2 "build SDL2 third party for cdsdl2" ON) 
    option(BuildOpengl "build glad third party for cdopengl" ON) 
    add_library(libcomputeduck SHARED Ast.h
    Config.h
    Chunk.cpp Chunk.h
    Compiler.cpp Compiler.h 
    ConstantFolder.h ConstantFolder.cpp 
    Lexer.cpp Lexer.h
    Object.h 
    Parser.cpp Parser.h
    SymbolTable.h
    Token.h
    Utils.h
    Value.cpp Value.h
    BuiltinManager.cpp BuiltinManager.h
    VM.cpp VM.h
    PreProcessor.h)
    target_compile_definitions(libcomputeduck PUBLIC COMPUTE_DUCK_BUILD_DLL)

    add_subdirectory(library/cdsdl2)
    add_subdirectory(library/cdopengl)
    
    if(MSVC)
        target_compile_options(libcomputeduck PRIVATE "/wd4251;")
    endif()
    
else()

    set(LLVM_DIR "3rd/llvm14/LLVM/llvm")

    if(NOT EXISTS ${LLVM_DIR})
        message(NOTICE "llvm14 directory is required while open BuildLLVMImpl option.Please read Readme to operate.")
    endif()

    add_subdirectory(${LLVM_DIR})

    add_library(libcomputeduck-llvm SHARED Ast.h
        Config.h
        Chunk.cpp Chunk.h
        ConstantFolder.h ConstantFolder.cpp 
        Lexer.cpp Lexer.h
        Parser.cpp Parser.h
        Token.h
        Utils.h
        PreProcessor.h)
        target_compile_definitions(libcomputeduck-llvm  PUBLIC COMPUTE_DUCK_BUILD_DLL)

    if(MSVC)
        target_compile_options(libcomputeduck-llvm  PRIVATE "/wd4251;")
    endif()
endif()


target_link_libraries(computeduck PRIVATE libcomputeduck)

