cmake_minimum_required(VERSION 3.10)

project(ComputeDuck LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

option(COMPUTEDUCK_BUILD_WITH_LLVM "build llvm for jit engine" ON)
option(COMPUTEDUCK_BUILD_WITH_SDL2 "build SDL2 third party for cdsdl2" ON) 
option(COMPUTEDUCK_BUILD_WITH_OPENGL "build glad third party for cdopengl" ON)  

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cd")

source_group("examples" FILES ${EXAMPLES})

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/3rd")

if(MSVC)
    set(LIB_NAME libdycomputeduck)
else()
    set(LIB_NAME dycomputeduck)
endif()

set(EXE_NAME computeduck)

if(COMPUTEDUCK_BUILD_WITH_SDL2)
    add_subdirectory("${THIRD_PARTY_DIR}/SDL2")
    add_subdirectory(library/cdsdl2)
endif()

if(COMPUTEDUCK_BUILD_WITH_OPENGL)
    add_subdirectory(library/cdopengl)
endif()

set(ROOT_SRC Ast.h
             Allocator.h Allocator.cpp
             Chunk.cpp Chunk.h
             Compiler.h Compiler.cpp
             ConstantFolder.h ConstantFolder.cpp 
             Lexer.cpp Lexer.h
             Object.h Object.cpp
             Parser.cpp Parser.h
             Token.h
             SymbolTable.h
             Utils.h Utils.cpp
             Value.cpp Value.h
             BuiltinManager.cpp BuiltinManager.h
             VM.h VM.cpp
             PreProcessor.h
             Config.cpp Config.h
             SetLLVM.cmake)

add_executable(${EXE_NAME} main.cpp ${EXAMPLES})

if(COMPUTEDUCK_BUILD_WITH_LLVM)
    include(SetLLVM.cmake)

    set(LLVM_DIR "${THIRD_PARTY_DIR}/llvm/llvm")
    set(LLVM_GENERATE_DIR "${CMAKE_BINARY_DIR}/3rd/llvm/llvm")
    
    SetLLVM(${LLVM_DIR} LLVM_LIBS)
    
    message(STATUS "LLVM_LIBS=${LLVM_LIBS}")
    message(STATUS "LLVM_GENERATE_DIR=${LLVM_GENERATE_DIR}")
    
    list(APPEND ROOT_SRC Jit.h Jit.cpp)
    source_group("src" FILES ${ROOT_SRC})
    add_library(${LIB_NAME} SHARED ${ROOT_SRC})
    target_link_libraries(${LIB_NAME} PRIVATE ${LLVM_LIBS})
    target_include_directories(${LIB_NAME} PRIVATE ${LLVM_DIR}/include ${LLVM_GENERATE_DIR}/include)
    target_include_directories(${EXE_NAME} PRIVATE ${LLVM_DIR}/include ${LLVM_GENERATE_DIR}/include)
    target_compile_definitions(${LIB_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)
else()
    source_group("src" FILES ${ROOT_SRC})
    add_library(${LIB_NAME} SHARED ${ROOT_SRC})
endif()

target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
target_compile_definitions(${LIB_NAME} PUBLIC COMPUTE_DUCK_BUILD_DLL)

if(MSVC)
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX)
    target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX)

    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;")
    target_compile_options(${EXE_NAME} PRIVATE "/wd4251;")
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME} )
endif()
