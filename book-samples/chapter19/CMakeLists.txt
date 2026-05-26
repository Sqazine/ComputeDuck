cmake_minimum_required(VERSION 3.10)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

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

# ++ 新增内容
option(COMPUTEDUCK_BUILD_WITH_SDL2 "build SDL2 third party for cdsdl2" OFF) 
option(COMPUTEDUCK_BUILD_WITH_OPENGL "build glad third party for cdopengl" OFF)  
# -- 新增内容

file(GLOB EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.cd")
source_group("examples" FILES ${EXAMPLES})

set(LIB_NAME dycomputeduck)
set(EXE_NAME computeduck)
add_executable(${EXE_NAME} main.cc ${EXAMPLES})

# ++ 新增内容
if(COMPUTEDUCK_BUILD_WITH_SDL2)
    add_subdirectory(library/cdsdl2)
endif()

if(COMPUTEDUCK_BUILD_WITH_OPENGL)
    add_subdirectory(library/cdopengl)
endif()
# -- 新增内容

file(GLOB ROOT_SRC "*.h" "*.cpp")
source_group("" FILES ${ROOT_SRC})
add_library(${LIB_NAME} SHARED ${ROOT_SRC})

target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
target_compile_definitions(${LIB_NAME} PRIVATE COMPUTEDUCK_BUILD_DLL)

if(MSVC)
    target_compile_definitions(${LIB_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${EXE_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    
    set_property ( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXE_NAME} )
endif()
