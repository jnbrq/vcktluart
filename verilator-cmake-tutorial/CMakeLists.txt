# Canberk Sönmez

cmake_minimum_required(VERSION 3.11)
project(spr_001)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if (MSVC)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

include(AddVerilator)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(Verilator REQUIRED)

add_verilator(
    NAME hdl_seq_shift
    TRACE_ON
    SOURCE seq_shift.v)

add_executable(seq_shift main.cpp)
target_link_libraries(seq_shift hdl_seq_shift Boost::boost)

