cmake_minimum_required(VERSION 3.10)

option(BUILD_SDL2 "required sdl2 third party" ON)

project(ComputeDuck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

add_library(libcomputeduck SHARED Ast.h
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

file(GLOB SRC main.cpp)
add_executable(computeduck ${SRC})
target_link_libraries(computeduck PRIVATE libcomputeduck)