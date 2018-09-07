set(TEST_NAME masked_connectors)

set(EXE_NAME exe_tests_${TEST_NAME})

add_verilator(
    NAME hdl_tests_masked_l2m_connector
    SOURCE "${CMAKE_SOURCE_DIR}/verilog/masked_l2m_connector.sv"
    TOP_MODULE masked_l2m_connector
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/verilog
        ${CMAKE_CURRENT_SOURCE_DIR})

add_verilator(
    NAME hdl_tests_masked_m2l_connector
    SOURCE "${CMAKE_SOURCE_DIR}/verilog/masked_m2l_connector.sv"
    TOP_MODULE masked_m2l_connector
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/verilog
        ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(
    ${EXE_NAME}
    main.cpp)

target_link_libraries(
    ${EXE_NAME}
    PUBLIC
        hdl_tests_masked_l2m_connector
        hdl_tests_masked_m2l_connector
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
unset(TEST_NAME)
