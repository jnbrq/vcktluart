# Canberk Sönmez

cmake_minimum_required(VERSION 3.11)
project(tilelink_implementation)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

include(AddVerilator)

enable_testing()

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(Verilator REQUIRED)

add_subdirectory(tests/)

