if(BuildSDL2)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/SDL")
    add_library(cdsdl2 SHARED  cdsdl2.h  cdsdl2.cpp)
    target_include_directories(cdsdl2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/SDL/include" ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(cdsdl2 PRIVATE SDL2::SDL2 libcomputeduck)
    target_compile_definitions(cdsdl2 PUBLIC COMPUTE_DUCK_BUILD_DLL)
    if(MSVC)
        target_compile_options(cdsdl2 PRIVATE "/wd4251;")
    endif()
endif()