cmake_minimum_required(VERSION 3.10)

option(BUILD_SDL2 "required sdl2 third party" ON)

if (BUILD_SDL2)
set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(ComputeDuck LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)


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
VM.cpp VM.h)
target_compile_definitions(libcomputeduck PUBLIC COMPUTE_DUCK_BUILD_DLL)

if (BUILD_SDL2)
find_package(SDL2 CONFIG REQUIRED)
add_library(sdl2 SHARED  library/sdl2/sdl2.h  library/sdl2/sdl2.cpp)
target_link_libraries(sdl2 PRIVATE SDL2::SDL2 libcomputeduck)
target_compile_definitions(sdl2 PUBLIC COMPUTE_DUCK_BUILD_DLL)
endif()

file(GLOB SRC main.cpp)
add_executable(computeduck ${SRC})
target_link_libraries(computeduck PRIVATE libcomputeduck)