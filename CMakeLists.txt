cmake_minimum_required(VERSION 3.10)

project(ComputeDuck LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

option(BuildLLVMImpl "build llvm impl" ON)
option(BuildSDL2 "build SDL2 third party for cdsdl2" OFF) 
option(BuildOpengl "build glad third party for cdopengl" OFF)  

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cd")

source_group("examples" FILES ${EXAMPLES})

add_subdirectory(library/cdsdl2)
add_subdirectory(library/cdopengl)

set(ROOT_SRC Ast.h
             Config.h
             Chunk.cpp Chunk.h
             Compiler.cpp Compiler.h 
             ConstantFolder.h ConstantFolder.cpp 
             Lexer.cpp Lexer.h
             Object.h 
             Parser.cpp Parser.h
             Token.h
             Utils.h
             Value.cpp Value.h
             BuiltinManager.cpp BuiltinManager.h
             VM.cpp VM.h
             PreProcessor.h)

add_executable(computeduck main.cpp ${EXAMPLES})

if(BuildLLVMImpl)
    include(SetLLVM.cmake)
    
    set(LLVM_DIR "3rd/llvm18/LLVM/llvm" )

    SetLLVM(${LLVM_DIR} LLVM_LIBS)

    message(STATUS "LLVM_LIBS=${LLVM_LIBS}")

    list(APPEND ROOT_SRC LLVMCompiler.cpp LLVMCompiler.h LLVMJit.h LLVMJit.cpp)

    source_group("src" FILES ${ROOT_SRC})
    add_library(libcomputeduck SHARED ${ROOT_SRC})
    
    target_link_libraries(libcomputeduck PRIVATE ${LLVM_LIBS})
    target_include_directories(libcomputeduck PRIVATE ${LLVM_DIR}/include ${LLVM_BINARY_DIR}/include)
    target_include_directories(computeduck PRIVATE ${LLVM_DIR}/include ${LLVM_BINARY_DIR}/include)
    
    message(STATUS "LLVM_BINARY_DIR=${LLVM_BINARY_DIR}")

    target_compile_definitions(libcomputeduck PUBLIC BUILD_WITH_LLVM)
    target_compile_definitions(computeduck PUBLIC BUILD_WITH_LLVM)
else()
    source_group("src" FILES ${ROOT_SRC})
    add_library(libcomputeduck SHARED ${ROOT_SRC})
endif()

target_link_libraries(computeduck PRIVATE libcomputeduck)
target_compile_definitions(libcomputeduck PUBLIC COMPUTE_DUCK_BUILD_DLL)

if(MSVC)
    target_compile_options(libcomputeduck PRIVATE "/wd4251;")
    target_compile_options(computeduck PRIVATE "/wd4251;")
endif()

set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT computeduck )