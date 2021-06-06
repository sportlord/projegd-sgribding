cmake_minimum_required(VERSION 3.19)
project(projegd-sgribding)

set(CMAKE_CXX_STANDARD 14)

set(VERSION "UNRELEASED" CACHE STRING "Software version")

option(ENABLE_DEBUG "Enable debugging" OFF)
if(ENABLE_DEBUG)
    add_definitions(-DDEBUG)
endif()

option(ENABLE_SIMULATE "Only simulate" OFF)
if(ENABLE_SIMULATE)
    add_definitions(-DSIMULATE)
endif(ENABLE_SIMULATE)

configure_file(src/version.h.tpl src/version.h)

include_directories(${PROJECT_BINARY_DIR})

add_executable(projegd-sgribding src/main.cpp src/games/lol.h src/random.h src/common.h src/version.h src/mapping.h src/types.h)