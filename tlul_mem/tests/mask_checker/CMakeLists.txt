set(TEST_NAME mask_checker)

set(HDL_NAME hdl_tests_${TEST_NAME})
set(EXE_NAME exe_tests_${TEST_NAME})

add_verilator(
    NAME ${HDL_NAME}_8bits
    SOURCE "${CMAKE_SOURCE_DIR}/verilog/mask_checker.sv"
    TOP_MODULE mask_checker
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/verilog
        ${CMAKE_CURRENT_SOURCE_DIR}
    APPEND
        -pvalue+W=8)

add_verilator(
    NAME ${HDL_NAME}_64bits
    SOURCE "${CMAKE_SOURCE_DIR}/verilog/mask_checker.sv"
    TOP_MODULE mask_checker
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/verilog
        ${CMAKE_CURRENT_SOURCE_DIR}
    APPEND
        -pvalue+W=64)

add_executable(
    ${EXE_NAME}
    main.cpp)

target_link_libraries(
    ${EXE_NAME}
    PUBLIC
        ${HDL_NAME}_8bits
        ${HDL_NAME}_64bits
        Boost::unit_test_framework)

target_compile_definitions(
    ${EXE_NAME}
    PUBLIC
        BOOST_TEST_DYN_LINK)

target_include_directories(
    ${EXE_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include)

add_test(
    NAME test_${TEST_NAME}
    COMMAND ${EXE_NAME})

unset(EXE_NAME)
unset(HDL_NAME)
unset(TEST_NAME)
