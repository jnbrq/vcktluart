# Source: github tymonx/logic

if (COMMAND _find_verilator)
    return()
endif()

function(_find_verilator)
    find_package(PackageHandleStandardArgs REQUIRED)

    find_program(VERILATOR_EXECUTABLE verilator
        HINTS $ENV{VERILATOR_ROOT}
        PATH_SUFFIXES bin
        DOC "Path to the Verilator executable"
    )

    find_program(VERILATOR_COVERAGE_EXECUTABLE verilator_coverage
        HINTS $ENV{VERILATOR_ROOT}
        PATH_SUFFIXES bin
        DOC "Path to the Verilator coverage executable"
    )

    get_filename_component(VERILATOR_EXECUTABLE_DIR ${VERILATOR_EXECUTABLE}
        DIRECTORY)

    find_path(VERILATOR_INCLUDE_DIR verilated.h
        HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_EXECUTABLE_DIR}/..
        PATH_SUFFIXES include share/verilator/include
        DOC "Path to the Verilator headers"
    )

    mark_as_advanced(VERILATOR_EXECUTABLE)
    mark_as_advanced(VERILATOR_COVERAGE_EXECUTABLE)
    mark_as_advanced(VERILATOR_INCLUDE_DIR)

    find_package_handle_standard_args(Verilator REQUIRED_VARS
        VERILATOR_EXECUTABLE VERILATOR_COVERAGE_EXECUTABLE VERILATOR_INCLUDE_DIR)

    set(VERILATOR_FOUND ${VERILATOR_FOUND} PARENT_SCOPE)
    set(VERILATOR_EXECUTABLE "${VERILATOR_EXECUTABLE}" PARENT_SCOPE)
    set(VERILATOR_INCLUDE_DIR "${VERILATOR_INCLUDE_DIR}" PARENT_SCOPE)
    set(VERILATOR_COVERAGE_EXECUTABLE "${VERILATOR_COVERAGE_EXECUTABLE}"
        PARENT_SCOPE)
    
    if (VERILATOR_FOUND)
        message(STATUS "VERILATOR_EXECUTABLE: ${VERILATOR_EXECUTABLE}")
    else()
        message(STATUS "Verilator is NOT FOUND!!!")
    endif()
endfunction()

_find_verilator()
