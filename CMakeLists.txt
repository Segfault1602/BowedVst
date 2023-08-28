cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)
project(BowedVst VERSION 0.0.1)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13")

set(LIBDSP_LIB_ONLY
    ON
    CACHE BOOL "Only build libdsp static lib" FORCE)

set(LIBDSP_BUILD_TESTS
    OFF
    CACHE BOOL "Don't build libdsp tests" FORCE)

add_subdirectory(externals)
add_subdirectory(src)
