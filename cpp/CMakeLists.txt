cmake_minimum_required(VERSION 3.10)

set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")


project(ComputeDuck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(libcomputeduck Ast.h 
BuiltinFunctionManager.h BuiltinFunctionManager.cpp
Chunk.h Chunk.cpp
Compiler.h Compiler.cpp
Lexer.h Lexer.cpp
Object.h
Parser.h Parser.cpp
SemanticAnalyzer.h SemanticAnalyzer.cpp
SymbolTable.h
Token.h
Utils.h
Value.h Value.cpp
VM.h VM.cpp) 

add_executable(computeduck main.cpp)
target_link_libraries(computeduck PRIVATE libcomputeduck)

find_package(SDL2 CONFIG REQUIRED)
add_executable(SDL2Test SDL2Test.cpp)
target_link_libraries(SDL2Test PRIVATE libcomputeduck  SDL2::SDL2)