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
             SymbolTable.h
             Token.h
             Utils.h
             Value.cpp Value.h
             BuiltinManager.cpp BuiltinManager.h
             VM.cpp VM.h
             PreProcessor.h)

add_executable(computeduck main.cpp ${EXAMPLES})

if(BuildLLVMImpl)

    set(LLVM_BUILD_TOOLS OFF)
    set(LLVM_INCLUDE_TOOLS OFF)
    set(LLVM_BUILD_UTILS OFF)
    set(LLVM_INCLUDE_UTILS OFF)
    set(LLVM_BUILD_TESTS OFF)
    set(LLVM_INCLUDE_TESTS OFF)
    set(LLVM_INCLUDE_GO_TESTS OFF)
    set(LLVM_BUILD_BENCHMARKS OFF)
    set(LLVM_INCLUDE_BENCHMARKS OFF)
    set(LLVM_ENABLE_BACKTRACES OFF)
    set(LLVM_ENABLE_CRASH_OVERRIDES OFF)
    set(LLVM_ENABLE_UNWIND_TABLES OFF)
    set(LLVM_ENABLE_TERMINFO OFF)
    set(LLVM_ENABLE_LIBEDIT OFF)
    set(LLVM_ENABLE_LIBPFM OFF)
    set(LLVM_ENABLE_PEDANTIC OFF)
    set(LLVM_INCLUDE_DOCS OFF)
    set(LLVM_ENABLE_OCAMLDOC OFF)
    set(LLVM_ENABLE_BINDINGS OFF)
    set(LLVM_VERSION_PRINTER_SHOW_HOST_TARGET_INFO OFF)

    set(LLVM_DIR "3rd/llvm14/LLVM/llvm")

    add_subdirectory(${LLVM_DIR})

    set(LLVM_LINK_COMPONENTS
        Analysis
        Core
        ExecutionEngine
        InstCombine
        Object
        OrcJIT
        RuntimeDyld
        ScalarOpts
        Support
        native
        )

    llvm_map_components_to_libnames(llvm_libs ${LLVM_LINK_COMPONENTS})

    set(LLVM_SRC LLVMCompiler.cpp 
                 LLVMCompiler.h)

    add_library(libcomputeduck SHARED ${ROOT_SRC} ${LLVM_SRC})
    target_link_libraries(libcomputeduck PRIVATE ${llvm_libs})
    target_include_directories(libcomputeduck PRIVATE ${LLVM_DIR}/include ${LLVM_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/llvm)
    target_include_directories(computeduck PRIVATE ${LLVM_DIR}/include ${LLVM_BINARY_DIR}/include)
    
    target_compile_definitions(libcomputeduck PUBLIC BUILD_WITH_LLVM)
    target_compile_definitions(computeduck PUBLIC BUILD_WITH_LLVM)
else()
    add_library(libcomputeduck SHARED ${ROOT_SRC})
endif()

target_link_libraries(computeduck PRIVATE libcomputeduck)
target_compile_definitions(libcomputeduck PUBLIC COMPUTE_DUCK_BUILD_DLL)

if(MSVC)
    target_compile_options(libcomputeduck PRIVATE "/wd4251;")
    target_compile_options(computeduck PRIVATE "/wd4251;")
endif()
