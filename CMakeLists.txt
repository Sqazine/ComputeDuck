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

if (BUILD_SDL2)
find_package(SDL2 CONFIG REQUIRED)
endif()

file(GLOB SRC "*.h" "*.cpp")
add_executable(computeduck ${SRC})

if (BUILD_SDL2)
target_compile_definitions(computeduck PUBLIC BUILD_WITH_SDL2)
target_link_libraries(computeduck PRIVATE  SDL2::SDL2)
endif()