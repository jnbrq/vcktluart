# Canberk Sönmez

cmake_minimum_required(VERSION 3.11)
project(tlul_uart)

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
    NAME hdl_tlul_uart
    TRACE_ON
    SOURCE ${CMAKE_SOURCE_DIR}/verilog/tlul_uart.sv
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/verilog ${CMAKE_CURRENT_SOURCE_DIR})

add_verilator(
    NAME hdl_tlul_uart_echo
    TRACE_ON
    SOURCE ${CMAKE_SOURCE_DIR}/verilog/tlul_uart_echo.sv
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/verilog ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()

# tlul_uart_tb
add_executable(tlul_uart_tb src/tlul_uart_tb.cpp)
target_link_libraries(tlul_uart_tb hdl_tlul_uart hdl_tlul_uart_echo Boost::boost)
add_test(NAME test_tlul_uart COMMAND tlul_uart_tb)

