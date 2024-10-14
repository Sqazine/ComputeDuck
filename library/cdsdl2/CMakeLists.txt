

if(WIN32)
    set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

set(CD_SDL2_LIB_NAME cdsdl2)

set(BUILD_SHARED_LIBS OFF)

set(SDL2_ROOT_DIR "" CACHE PATH "set SDL2 root directory")
set(SDL2_GENERATE_DIR "${CMAKE_BINARY_DIR}/3rd/SDL2")

add_subdirectory(${SDL2_ROOT_DIR} ${SDL2_GENERATE_DIR} EXCLUDE_FROM_ALL)

add_library(${CD_SDL2_LIB_NAME} SHARED  cdsdl2.h  cdsdl2.cpp)
target_include_directories(${CD_SDL2_LIB_NAME} PRIVATE "${SDL2_ROOT_DIR}/include" ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${CD_SDL2_LIB_NAME} PRIVATE SDL2::SDL2 ${LIB_NAME})
target_compile_definitions(${CD_SDL2_LIB_NAME} PUBLIC COMPUTE_DUCK_BUILD_DLL)
if(MSVC)
    target_compile_options(${CD_SDL2_LIB_NAME} PRIVATE "/wd4251;")
endif()