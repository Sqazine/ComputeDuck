cmake_minimum_required(VERSION 3.10)

project(ComputeDuck)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

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

option(COMPUTEDUCK_BUILD_WITH_LLVM "build with LLVM JIT engine(using LLVM 14.0.6 version)" OFF) 
option(COMPUTEDUCK_BUILD_WITH_SDL2 "build SDL2 third party for cdsdl2" OFF) 
option(COMPUTEDUCK_BUILD_WITH_OPENGL "build glad third party for cdopengl" OFF)  

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cd")
source_group("examples" FILES ${EXAMPLES})

set(LIB_NAME dycomputeduck)
set(EXE_NAME computeduck)
add_executable(${EXE_NAME} main.cc ${EXAMPLES})

if(COMPUTEDUCK_BUILD_WITH_SDL2)
    set(SDL2_ROOT_DIR "${CMAKE_SOURCE_DIR}/3rd/SDL-release-2.28.2")
    set(SDL2_GENERATE_DIR "${CMAKE_BINARY_DIR}/3rd/SDL2")
    if(NOT EXISTS "${SDL2_ROOT_DIR}")
        message(STATUS "Extracting 3rd/SDL-release-2.28.2.zip ...")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_SOURCE_DIR}/3rd/SDL-release-2.28.2.zip"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rd"
            RESULT_VARIABLE SDL_EXTRACT_RESULT)
        if(NOT SDL_EXTRACT_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to extract 3rd/SDL-release-2.28.2.zip")
        endif()
    endif()
    add_subdirectory(library/cdsdl2)
endif()

if(COMPUTEDUCK_BUILD_WITH_OPENGL)
    add_subdirectory(library/cdopengl)
endif()

file(GLOB ROOT_SRC "*.h" "*.cpp" "*.inl" "*.cmake")
source_group("" FILES ${ROOT_SRC})

if(COMPUTEDUCK_BUILD_WITH_LLVM)
    set(LLVM_ROOT_DIR "${CMAKE_SOURCE_DIR}/3rd/LLVM-release-14.x")
    set(LLVM_GENERATE_DIR "${CMAKE_BINARY_DIR}/3rd/llvm")
    if(NOT EXISTS "${LLVM_ROOT_DIR}")
        message(STATUS "Extracting 3rd/LLVM-release-14.x.zip ...")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_SOURCE_DIR}/3rd/LLVM-release-14.x.zip"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rd"
            RESULT_VARIABLE LLVM_EXTRACT_RESULT)
        if(NOT LLVM_EXTRACT_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to extract 3rd/LLVM-release-14.x.zip")
        endif()
    endif()
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
    target_compile_options(${LIB_NAME} PRIVATE "/wd4251;" "/bigobj;")
    target_compile_options(${EXE_NAME} PRIVATE "/wd4251;" "/bigobj;")
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME} )
endif()
