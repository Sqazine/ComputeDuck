
if(WIN32)
    # Prefix all shared libraries with 'lib'.
    set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
    # Prefix all static libraries with 'lib'.
    set(CMAKE_STATIC_LIBRARY_PREFIX "lib")
endif()

set(CD_OPENGL_LIB_NAME cdopengl)

file(GLOB glad "${CMAKE_CURRENT_SOURCE_DIR}/glad/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/glad/*.c")
add_library(${CD_OPENGL_LIB_NAME} SHARED  cdopengl.h  cdopengl.cpp ${glad})
target_include_directories(${CD_OPENGL_LIB_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/glad" ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${CD_OPENGL_LIB_NAME} PRIVATE ${LIB_NAME})
target_compile_definitions(${CD_OPENGL_LIB_NAME} PUBLIC COMPUTE_DUCK_BUILD_DLL)
if(MSVC)
    target_compile_options(${CD_OPENGL_LIB_NAME} PRIVATE "/wd4251;")
endif()