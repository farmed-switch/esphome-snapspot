set(_json_test_cmake_list_file ${CMAKE_CURRENT_LIST_FILE})

include(download_test_data)

add_test(NAME "download_test_data" COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    --target download_test_data
)
set_tests_properties(download_test_data PROPERTIES FIXTURES_SETUP TEST_DATA)

if(JSON_Valgrind)
    find_program(CMAKE_MEMORYCHECK_COMMAND valgrind)
    message(STATUS "Executing test suite with Valgrind (${CMAKE_MEMORYCHECK_COMMAND})")
    set(memcheck_command "${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1 --leak-check=full")
    separate_arguments(memcheck_command)
endif()

set(compiler_supports_cpp_11 TRUE)

foreach(feature ${CMAKE_CXX_COMPILE_FEATURES})
    if (${feature} STREQUAL cxx_std_14)
        set(compiler_supports_cpp_14 TRUE)
    elseif (${feature} STREQUAL cxx_std_17)
        set(compiler_supports_cpp_17 TRUE)
    elseif (${feature} STREQUAL cxx_std_20)
        set(compiler_supports_cpp_20 TRUE)
    elseif (${feature} STREQUAL cxx_std_23)
        set(compiler_supports_cpp_23 TRUE)
    endif()
endforeach()

function(json_test_set_test_options tests)
    cmake_parse_arguments(args "" ""
        "CXX_STANDARDS;COMPILE_DEFINITIONS;COMPILE_FEATURES;COMPILE_OPTIONS;LINK_LIBRARIES;LINK_OPTIONS;TEST_PROPERTIES"
        ${ARGN})

    if(NOT args_CXX_STANDARDS)
        set(args_CXX_STANDARDS "all")
    endif()

    foreach(test ${tests})
        if("${test}" STREQUAL "all")
            set(test "")
        endif()

        foreach(cxx_standard ${args_CXX_STANDARDS})
            if("${cxx_standard}" STREQUAL "all")
                if("${test}" STREQUAL "")
                    message(FATAL_ERROR "Not supported. Change defaults in: ${_json_test_cmake_list_file}")
                endif()
                set(test_interface _json_test_interface_${test})
            else()
                set(test_interface _json_test_interface_${test}_cpp_${cxx_standard})
            endif()

            if(NOT TARGET ${test_interface})
                add_library(${test_interface} INTERFACE)
            endif()

            target_compile_definitions(${test_interface} INTERFACE ${args_COMPILE_DEFINITIONS})
            target_compile_features(${test_interface} INTERFACE ${args_COMPILE_FEATURES})
            target_compile_options(${test_interface} INTERFACE ${args_COMPILE_OPTIONS})
            target_link_libraries (${test_interface} INTERFACE ${args_LINK_LIBRARIES})
            target_link_options(${test_interface} INTERFACE ${args_LINK_OPTIONS})

            set_property(DIRECTORY PROPERTY
                ${test_interface}_TEST_PROPERTIES "${args_TEST_PROPERTIES}"
            )
        endforeach()
    endforeach()
endfunction()

function(_json_test_apply_test_properties test_target properties_target)

    get_property(test_properties DIRECTORY PROPERTY ${properties_target}_TEST_PROPERTIES)
    if(test_properties)
        set_tests_properties(${test_target} PROPERTIES ${test_properties})
    endif()
endfunction()

function(_json_test_add_test test_name file main cxx_standard)
    set(test_target ${test_name}_cpp${cxx_standard})

    if(TARGET ${test_target})
        message(FATAL_ERROR "Target ${test_target} has already been added.")
    endif()

    add_executable(${test_target} ${file})
    target_link_libraries(${test_target} PRIVATE ${main})

    set_target_properties(${test_target} PROPERTIES
        CXX_STANDARD ${cxx_standard}
        CXX_STANDARD_REQUIRED ON
    )

    if(TARGET _json_test_interface__cpp_${cxx_standard})
        target_link_libraries(${test_target} PRIVATE _json_test_interface__cpp_${cxx_standard})
    endif()

    if(TARGET _json_test_interface_${test_name})
        target_link_libraries(${test_target} PRIVATE _json_test_interface_${test_name})
    endif()

    if(TARGET _json_test_interface_${test_name}_cpp_${cxx_standard})
        target_link_libraries(${test_target} PRIVATE
            _json_test_interface_${test_name}_cpp_${cxx_standard}
        )
    endif()

    if (JSON_FastTests)
        add_test(NAME ${test_target}
            COMMAND ${test_target} ${DOCTEST_TEST_FILTER}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    else()
        add_test(NAME ${test_target}
            COMMAND ${test_target} ${DOCTEST_TEST_FILTER} --no-skip
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endif()
    set_tests_properties(${test_target} PROPERTIES LABELS "all" FIXTURES_REQUIRED TEST_DATA)

    if(TARGET _json_test_interface__cpp_${cxx_standard})
        _json_test_apply_test_properties(${test_target} _json_test_interface__cpp_${cxx_standard})
    endif()

    if(TARGET _json_test_interface_${test_name})
        _json_test_apply_test_properties(${test_target} _json_test_interface_${test_name})
    endif()

    if(TARGET _json_test_interface_${test_name}_cpp_${cxx_standard})
        _json_test_apply_test_properties(${test_target}
            _json_test_interface_${test_name}_cpp_${cxx_standard}
        )
    endif()

    if(JSON_Valgrind)
        add_test(NAME ${test_target}_valgrind
            COMMAND ${memcheck_command} $<TARGET_FILE:${test_target}> ${DOCTEST_TEST_FILTER}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
        set_tests_properties(${test_target}_valgrind PROPERTIES
            LABELS "valgrind" FIXTURES_REQUIRED TEST_DATA
        )
    endif()
endfunction()

function(json_test_add_test_for file)
    cmake_parse_arguments(args "FORCE" "MAIN;NAME" "CXX_STANDARDS" ${ARGN})

    if("${args_MAIN}" STREQUAL "")
        message(FATAL_ERROR "Required argument MAIN <main> missing.")
    endif()

    if("${args_NAME}" STREQUAL "")
        get_filename_component(file_basename ${file} NAME_WE)
        string(REGEX REPLACE "unit-([^$]+)" "test-\\1" test_name ${file_basename})
    else()
        set(test_name ${args_NAME})
        if(NOT test_name MATCHES "test-[^$]+")
            message(FATAL_ERROR "Test name must start with 'test-'.")
        endif()
    endif()

    if("${args_CXX_STANDARDS}" STREQUAL "")
        set(args_CXX_STANDARDS 11)
    endif()

    file(READ ${file} file_content)
    foreach(cxx_standard ${args_CXX_STANDARDS})
        if(NOT compiler_supports_cpp_${cxx_standard})
            continue()
        endif()

        if(NOT ("${cxx_standard}" STREQUAL 11 OR args_FORCE))
            string(FIND "${file_content}" JSON_HAS_CPP_${cxx_standard} has_cpp_found)
            if(${has_cpp_found} EQUAL -1)
                continue()
            endif()
        endif()

        _json_test_add_test(${test_name} ${file} ${args_MAIN} ${cxx_standard})
    endforeach()
endfunction()

function(json_test_should_build_32bit_test build_32bit_var build_32bit_only_var input)
    set(${build_32bit_only_var} OFF PARENT_SCOPE)
    string(TOUPPER "${input}" ${build_32bit_var})
    if("${${build_32bit_var}}" STREQUAL AUTO)

        include(CheckTypeSize)
        check_type_size("size_t" sizeof_size_t LANGUAGE CXX)
        if(sizeof_size_t AND ${sizeof_size_t} EQUAL 4)
            message(STATUS "Auto-enabling 32bit unit test.")
            set(${build_32bit_var} ON)
        else()
            set(${build_32bit_var} OFF)
        endif()
    elseif("${${build_32bit_var}}" STREQUAL ONLY)
        set(${build_32bit_only_var} ON PARENT_SCOPE)
    endif()

    set(${build_32bit_var} "${${build_32bit_var}}" PARENT_SCOPE)
endfunction()
