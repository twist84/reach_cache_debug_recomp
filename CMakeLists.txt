# reach_cache_debug - ReXGlue Recompiled Project
#
# This file is yours to edit. 'rexglue migrate' will NOT overwrite it.
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(reach_cache_debug LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(REACHCACHEDEBUG_SOURCES
    source/main.cpp
)

if(WIN32)
    add_executable(reach_cache_debug WIN32 ${REACHCACHEDEBUG_SOURCES})
else()
    add_executable(reach_cache_debug ${REACHCACHEDEBUG_SOURCES})
endif()

file(GLOB_RECURSE REACH_GAME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/reach/source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/reach/source/*.c"
)
target_sources(reach_cache_debug PRIVATE
    ${REACH_GAME_SOURCES}
)
target_include_directories(reach_cache_debug PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/reach/source
)

rexglue_setup_target(reach_cache_debug)

