cmake_minimum_required(VERSION 3.10)

project(ComputeDuck LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

set(EXE_NAME computeduck)
add_executable(${EXE_NAME} main.cpp ${EXAMPLES})

set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/3rd")

if(MSVC)
    set(LIB_NAME libdycomputeduck)
else()
    set(LIB_NAME dycomputeduck)
endif()

if(COMPUTEDUCK_BUILD_WITH_SDL2)
    add_subdirectory(${THIRD_PARTY_DIR}/SDL2)
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
             Table.cpp Table.h)

if(COMPUTEDUCK_BUILD_WITH_LLVM)
    list(APPEND ROOT_SRC Jit.h Jit.cpp JitUtils.h JitUtils.cpp)
    source_group("src" FILES ${ROOT_SRC})

    set(LLVM_DIR "" CACHE STRING "set LLVMConfig.cmake directory")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    find_package(LLVM REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

    if(NOT ${LLVM_PACKAGE_VERSION} STREQUAL "14.0.6")
        message(FATAL_ERROR "LLVM Version require 14.0.6")
    endif()

    include_directories(${LLVM_INCLUDE_DIRS})
    separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
    add_definitions(${LLVM_DEFINITIONS_LIST})

    add_library(${LIB_NAME} SHARED ${ROOT_SRC})

    set(LLVM_LINK_COMPONENTS Core ExecutionEngine Object OrcJIT native)

    llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})

    target_link_libraries(${LIB_NAME} PRIVATE ${llvm_libs})
    target_link_libraries(${EXE_NAME} PRIVATE ${llvm_libs})

    target_compile_definitions(${LIB_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)
    target_compile_definitions(${EXE_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)
else()
    source_group("src" FILES ${ROOT_SRC})
    add_library(${LIB_NAME} SHARED ${ROOT_SRC})
endif()

target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
target_compile_definitions(${LIB_NAME} PUBLIC COMPUTE_DUCK_BUILD_DLL)

if(MSVC)
    target_compile_definitions(${LIB_NAME} PUBLIC NOMINMAX)
    target_compile_definitions(${EXE_NAME} PUBLIC NOMINMAX)

    target_compile_definitions(${LIB_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${EXE_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)

    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD")
    target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;" "/MD")
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME} )
else()
    target_compile_options(${LIB_NAME} PRIVATE "-fpermissive")
    target_compile_options(${EXE_NAME} PRIVATE "-fpermissive")
endif()
