cmake_minimum_required(VERSION 3.10)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

project(ComputeDuck LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

if(WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Bin/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Bin/Release)

set(COMPUTEDUCK_BUILD_WITH_LLVM "OFF" CACHE STRING "build with LLVM JIT engine(using LLVM 14.0.6 version)")
set_property(CACHE COMPUTEDUCK_BUILD_WITH_LLVM PROPERTY STRINGS "OFF" "PREBUILT" "SOURCE")

option(COMPUTEDUCK_BUILD_WITH_SDL2 "build SDL2 third party for cdsdl2" OFF) 
option(COMPUTEDUCK_BUILD_WITH_OPENGL "build glad third party for cdopengl" OFF)  

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cd")
source_group("examples" FILES ${EXAMPLES})

set(LIB_NAME dycomputeduck)
set(EXE_NAME computeduck)
add_executable(${EXE_NAME} main.cc ${EXAMPLES})

if(COMPUTEDUCK_BUILD_WITH_SDL2)
    add_subdirectory(library/cdsdl2)
endif()

if(COMPUTEDUCK_BUILD_WITH_OPENGL)
    add_subdirectory(library/cdopengl)
endif()

file(GLOB ROOT_SRC "*.h" "*.cpp" "*.inl" "*.cmake")
source_group("" FILES ${ROOT_SRC})

if(${COMPUTEDUCK_BUILD_WITH_LLVM} STREQUAL "PREBUILT")
    add_library(${LIB_NAME} SHARED ${ROOT_SRC})

    include(SetLLVM.cmake)
    SetPrebuiltLLVM(LLVM_LIBS)
    
    target_link_libraries(${LIB_NAME} PRIVATE ${LLVM_LIBS})
    target_link_libraries(${EXE_NAME} PRIVATE ${LLVM_LIBS})
    target_compile_definitions(${LIB_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)
    target_compile_definitions(${EXE_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)
elseif(${COMPUTEDUCK_BUILD_WITH_LLVM} STREQUAL "SOURCE")
    set(LLVM_ROOT_DIR "" CACHE PATH "set LLVM project root directory")
    set(LLVM_GENERATE_DIR "${CMAKE_BINARY_DIR}/3rd/llvm")
    set(LLVM_DIR ${LLVM_ROOT_DIR}/llvm)
    
    include(SetLLVM.cmake)
    SetSourceLLVM(${LLVM_DIR} LLVM_LIBS)

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    if(NOT ${LLVM_PACKAGE_VERSION} STREQUAL "14.0.6")
        message(FATAL_ERROR "LLVM Version require 14.0.6")
    endif()
    
    message(STATUS "LLVM_LIBS=${LLVM_LIBS}")
    message(STATUS "LLVM_GENERATE_DIR=${LLVM_GENERATE_DIR}")
    
    add_library(${LIB_NAME} SHARED ${ROOT_SRC})
    target_link_libraries(${LIB_NAME} PRIVATE ${LLVM_LIBS})
    target_link_libraries(${EXE_NAME} PRIVATE ${LLVM_LIBS})
    target_include_directories(${LIB_NAME} PRIVATE ${LLVM_DIR}/include ${LLVM_GENERATE_DIR}/include)
    target_include_directories(${EXE_NAME} PRIVATE ${LLVM_DIR}/include ${LLVM_GENERATE_DIR}/include)
    target_compile_definitions(${LIB_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)  
    target_compile_definitions(${EXE_NAME} PUBLIC COMPUTEDUCK_BUILD_WITH_LLVM)
else()
    add_library(${LIB_NAME} SHARED ${ROOT_SRC})
endif()

target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
target_compile_definitions(${LIB_NAME} PUBLIC COMPUTEDUCK_BUILD_DLL)

if(MSVC)
    target_compile_definitions(${LIB_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${EXE_NAME} PUBLIC _CRT_SECURE_NO_WARNINGS)

    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;")
    target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;")
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME} )
endif()
