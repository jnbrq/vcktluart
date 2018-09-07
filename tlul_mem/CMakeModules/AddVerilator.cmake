if (COMMAND add_verilator_target)
    return()
endif()

include(CMakeParseArguments)

# SOURCE is relative to the source directory
# INCLUDE_DIRS is absolute

function(add_verilator)
    find_package(Verilator REQUIRED)

    set(options TRACE_ON)
    set(oneValueArgs NAME SOURCE TOP_MODULE LANGUAGE)
    set(multiValueArgs INCLUDE_DIRS DEFS UNDEFS APPEND PREPEND)
    cmake_parse_arguments(ADDV "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(ADDV_NAME)
        set(VTARGET ${ADDV_NAME})
        if(TARGET ${VTARGET})
            message(FATAL_ERROR "add_verilator: a target with NAME already exists")
        else()
            
        endif()
    else()
        message(FATAL_ERROR "add_verilator: NAME is necessary")
    endif()
    
    set(VCOMMAND ${VERILATOR_EXECUTABLE} --cc ${ADDV_PREPEND})
    
    set(VSOURCE)
    if(ADDV_SOURCE)
        if (IS_ABSOLUTE "${ADDV_SOURCE}")
            set(VSOURCE "${ADDV_SOURCE}")
        else()
            set(VSOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${ADDV_SOURCE}")
        endif()
        set(VCOMMAND ${VCOMMAND} ${VSOURCE})
    else()
        message(FATAL_ERROR "add_verilator: SOURCE is necessary")
    endif()
    
    foreach(INCLUDE_DIR ${ADDV_INCLUDE_DIRS})
        set(VCOMMAND ${VCOMMAND} -y ${INCLUDE_DIR})
    endforeach()
    
    foreach(DEF ${ADDV_DEFS})
        set(VCOMMAND ${VCOMMAND} "-D${DEF}")
    endforeach()
    
    foreach(UNDEF ${ADDV_UNDEFS})
        set(VCOMMAND ${VCOMMAND} "-U${UNDEF}")
    endforeach()
    
    if(ADDV_TRACE_ON)
        set(VCOMMAND ${VCOMMAND} "--trace")
    endif()
    
    if (ADDV_TOP_MODULE)
        set(VCOMMAND ${VCOMMAND} "--top-module" "${ADDV_TOP_MODULE}")
    endif()
    
    # all relative to the build directory, so fine
    
    set(VPREFIX ${VTARGET})
    set(VCOMMAND ${VCOMMAND} ${ADDV_APPEND} --prefix ${VPREFIX})
    set(VBASEDIR "hdl_${VTARGET}")
    set(VOBJDIR "${VBASEDIR}/obj_dir")
    
    message(STATUS "add_verilator: VCOMMAND= " ${VCOMMAND})
    message(STATUS "add_verilator: VBASEDIR= " ${VBASEDIR})
    message(STATUS "add_verilator: VOBJDIR= " ${VOBJDIR})
    
    add_custom_target(
        make_verilator_base_directory_${VTARGET}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${VBASEDIR})
    
    set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${VBASEDIR})
    
    set(VSTATICLIB "${VOBJDIR}/${VPREFIX}__ALL.a")
    message(STATUS "add_verilator: VSTATICLIB= " ${VSTATICLIB})
    
    add_custom_target(
        run_verilator_${VTARGET}
        COMMAND ${VCOMMAND}
        COMMAND make -C obj_dir -f ${VPREFIX}.mk
        DEPENDS ${VSOURCE}
        WORKING_DIRECTORY ${VBASEDIR})
    
    add_dependencies(run_verilator_${VTARGET} make_verilator_base_directory_${VTARGET})
    
    add_library(${VTARGET} INTERFACE)
    add_dependencies(${VTARGET} run_verilator_${VTARGET})
    target_link_libraries(${VTARGET} INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/${VSTATICLIB})
    target_include_directories(
        ${VTARGET}
        INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/${VOBJDIR}
        INTERFACE ${VERILATOR_INCLUDE_DIR})
    target_sources(${VTARGET}
        INTERFACE ${VERILATOR_INCLUDE_DIR}/verilated.cpp)
    if(ADDV_TRACE_ON)
        target_sources(${VTARGET}
            INTERFACE  ${VERILATOR_INCLUDE_DIR}/verilated_vcd_c.cpp)
        target_compile_definitions(${VTARGET} INTERFACE VM_TRACE)
    endif()
endfunction()

