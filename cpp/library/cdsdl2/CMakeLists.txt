add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/SDL")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/SDL/include")


if (BUILD_SDL2)
add_library(cdsdl2 SHARED  cdsdl2.h  cdsdl2.cpp)
target_link_libraries(cdsdl2 PRIVATE SDL2::SDL2 libcomputeduck)
target_compile_definitions(cdsdl2 PUBLIC COMPUTE_DUCK_BUILD_DLL)
endif()
