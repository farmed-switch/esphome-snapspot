

function(NANOPB_GENERATE_CPP SRCS HDRS)
  cmake_parse_arguments(NANOPB_GENERATE_CPP "" "RELPATH" "" ${ARGN})
  if(NOT NANOPB_GENERATE_CPP_UNPARSED_ARGUMENTS)
    return()
  endif()

  if(MSVC)
    set(CUSTOM_COMMAND_PREFIX call)
  endif()

  if(NANOPB_GENERATE_CPP_APPEND_PATH)

    foreach(FIL ${NANOPB_GENERATE_CPP_UNPARSED_ARGUMENTS})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(APPEND _nanopb_include_path "-I${ABS_PATH}")
    endforeach()
  else()
    set(_nanopb_include_path "-I${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  if(NANOPB_GENERATE_CPP_RELPATH)
    list(APPEND _nanopb_include_path "-I${NANOPB_GENERATE_CPP_RELPATH}")
  endif()

  if(DEFINED NANOPB_IMPORT_DIRS)
    foreach(DIR ${NANOPB_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(APPEND _nanopb_include_path "-I${ABS_PATH}")
    endforeach()
  endif()

  list(REMOVE_DUPLICATES _nanopb_include_path)

  set(GENERATOR_PATH ${CMAKE_CURRENT_BINARY_DIR}/nanopb/generator)

  set(NANOPB_GENERATOR_EXECUTABLE ${GENERATOR_PATH}/nanopb_generator.py)
  if (CMAKE_HOST_WIN32)
    set(NANOPB_GENERATOR_PLUGIN ${GENERATOR_PATH}/protoc-gen-nanopb.bat)
  else()
    set(NANOPB_GENERATOR_PLUGIN ${GENERATOR_PATH}/protoc-gen-nanopb)
  endif()

  set(GENERATOR_CORE_DIR ${GENERATOR_PATH}/proto)
  set(GENERATOR_CORE_SRC
      ${GENERATOR_CORE_DIR}/nanopb.proto)

  add_custom_command(
      OUTPUT ${NANOPB_GENERATOR_EXECUTABLE} ${GENERATOR_CORE_SRC}
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ARGS ${NANOPB_GENERATOR_SOURCE_DIR} ${GENERATOR_PATH}
      VERBATIM)

  set(GENERATOR_CORE_PYTHON_SRC)
  foreach(FIL ${GENERATOR_CORE_SRC})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(FIL_WE ${FIL} NAME_WE)

      set(output "${GENERATOR_CORE_DIR}/${FIL_WE}_pb2.py")
      set(GENERATOR_CORE_PYTHON_SRC ${GENERATOR_CORE_PYTHON_SRC} ${output})
      add_custom_command(
        OUTPUT ${output}
        COMMAND ${CUSTOM_COMMAND_PREFIX} ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS -I${GENERATOR_PATH}/proto
          --python_out=${GENERATOR_CORE_DIR} ${ABS_FIL}
        DEPENDS ${ABS_FIL}
        VERBATIM)
  endforeach()

  if(NANOPB_GENERATE_CPP_RELPATH)
      get_filename_component(ABS_ROOT ${NANOPB_GENERATE_CPP_RELPATH} ABSOLUTE)
  endif()
  foreach(FIL ${NANOPB_GENERATE_CPP_UNPARSED_ARGUMENTS})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    get_filename_component(FIL_DIR ${FIL} PATH)
    set(FIL_PATH_REL)
    if(ABS_ROOT)

      string(FIND ${ABS_FIL} ${ABS_ROOT} LOC)
      if (${LOC} EQUAL 0)
        string(REPLACE "${ABS_ROOT}/" "" FIL_REL ${ABS_FIL})
        get_filename_component(FIL_PATH_REL ${FIL_REL} PATH)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL})
      endif()
    endif()
    if(NOT FIL_PATH_REL)
      set(FIL_PATH_REL ".")
    endif()

    list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.c")
    list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.h")

    set(NANOPB_PLUGIN_OPTIONS)
    set(NANOPB_OPTIONS_DIRS)

    get_filename_component(ABS_OPT_FIL ${FIL_DIR}/${FIL_WE}.options ABSOLUTE)
    if(EXISTS ${ABS_OPT_FIL})

        get_filename_component(options_dir ${ABS_OPT_FIL} DIRECTORY)
        list(APPEND NANOPB_OPTIONS_DIRS ${options_dir})
    else()
        set(ABS_OPT_FIL)
    endif()

    foreach(depends_file ${NANOPB_DEPENDS})
        get_filename_component(ext ${depends_file} EXT)
        if(ext STREQUAL ".options")
            get_filename_component(depends_dir ${depends_file} DIRECTORY)
            list(APPEND NANOPB_OPTIONS_DIRS ${depends_dir})
        endif()
    endforeach()

    if(NANOPB_OPTIONS_DIRS)
        list(REMOVE_DUPLICATES NANOPB_OPTIONS_DIRS)
    endif()

    foreach(options_path ${NANOPB_OPTIONS_DIRS})
        set(NANOPB_PLUGIN_OPTIONS "${NANOPB_PLUGIN_OPTIONS} -I${options_path}")
    endforeach()

    if(NANOPB_OPTIONS)
        set(NANOPB_PLUGIN_OPTIONS "${NANOPB_PLUGIN_OPTIONS} ${NANOPB_OPTIONS}")
    endif()

    set(NANOPB_OUT "${CMAKE_CURRENT_BINARY_DIR}")

    execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --version OUTPUT_VARIABLE PROTOC_VERSION_STRING OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[(0-9)].*.[(0-9)].*.[(0-9)].*$" PROTOC_VERSION "${PROTOC_VERSION_STRING}")

    if(PROTOC_VERSION AND PROTOC_VERSION VERSION_LESS "3.6.0")

        string(REGEX MATCH ":" HAS_COLON_IN_PATH ${NANOPB_PLUGIN_OPTIONS} ${NANOPB_OUT})
        if(HAS_COLON_IN_PATH)
          message(FATAL_ERROR "Your path includes a ':' character used as an option separator for nanopb. Upgrade to protoc version >= 3.6.0 or use a different path.")
        endif()
        set(NANOPB_OPT_STRING "--nanopb_out=${NANOPB_PLUGIN_OPTIONS}:${NANOPB_OUT}")
    else()
      set(NANOPB_OPT_STRING "--nanopb_opt=${NANOPB_PLUGIN_OPTIONS}" "--nanopb_out=${NANOPB_OUT}")
    endif()

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.c"
             "${CMAKE_CURRENT_BINARY_DIR}/${FIL_PATH_REL}/${FIL_WE}.pb.h"
      COMMAND ${CUSTOM_COMMAND_PREFIX} ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS -I${GENERATOR_PATH} -I${GENERATOR_CORE_DIR}
           -I${CMAKE_CURRENT_BINARY_DIR} ${_nanopb_include_path}
           --plugin=protoc-gen-nanopb=${NANOPB_GENERATOR_PLUGIN}
           ${NANOPB_OPT_STRING}
           ${ABS_FIL}
      DEPENDS ${ABS_FIL} ${GENERATOR_CORE_PYTHON_SRC}
           ${ABS_OPT_FIL} ${NANOPB_DEPENDS}
      COMMENT "Running C++ protocol buffer compiler using nanopb plugin on ${FIL}"
      VERBATIM )

  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} ${NANOPB_SRCS} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} ${NANOPB_HDRS} PARENT_SCOPE)

  if(MSVC)
      unset(CUSTOM_COMMAND_PREFIX)
  endif()

endfunction()

if(NOT DEFINED NANOPB_GENERATE_CPP_APPEND_PATH)
  set(NANOPB_GENERATE_CPP_APPEND_PATH TRUE)
endif()

if(NOT DEFINED NANOPB_SRC_ROOT_FOLDER)
  get_filename_component(NANOPB_SRC_ROOT_FOLDER
                         ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
endif()

find_path(NANOPB_INCLUDE_DIRS
    pb.h
    PATHS ${NANOPB_SRC_ROOT_FOLDER}
    NO_CMAKE_FIND_ROOT_PATH
)
mark_as_advanced(NANOPB_INCLUDE_DIRS)

set(NANOPB_SRCS)
set(NANOPB_HDRS)
list(APPEND _nanopb_srcs pb_decode.c pb_encode.c pb_common.c)
list(APPEND _nanopb_hdrs pb_decode.h pb_encode.h pb_common.h pb.h)

foreach(FIL ${_nanopb_srcs})
  find_file(${FIL}__nano_pb_file NAMES ${FIL} PATHS ${NANOPB_SRC_ROOT_FOLDER} ${NANOPB_INCLUDE_DIRS} NO_CMAKE_FIND_ROOT_PATH)
  list(APPEND NANOPB_SRCS "${${FIL}__nano_pb_file}")
  mark_as_advanced(${FIL}__nano_pb_file)
endforeach()

foreach(FIL ${_nanopb_hdrs})
  find_file(${FIL}__nano_pb_file NAMES ${FIL} PATHS ${NANOPB_INCLUDE_DIRS} NO_CMAKE_FIND_ROOT_PATH)
  mark_as_advanced(${FIL}__nano_pb_file)
  list(APPEND NANOPB_HDRS "${${FIL}__nano_pb_file}")
endforeach()

find_program(PROTOBUF_PROTOC_EXECUTABLE
    NAMES protoc
    DOC "The Google Protocol Buffers Compiler"
    PATHS
    ${PROTOBUF_SRC_ROOT_FOLDER}/vsprojects/Release
    ${PROTOBUF_SRC_ROOT_FOLDER}/vsprojects/Debug
    ${NANOPB_SRC_ROOT_FOLDER}/generator-bin
    ${NANOPB_SRC_ROOT_FOLDER}/generator
)
mark_as_advanced(PROTOBUF_PROTOC_EXECUTABLE)

find_path(NANOPB_GENERATOR_SOURCE_DIR
    NAMES nanopb_generator.py
    DOC "nanopb generator source"
    PATHS
    ${NANOPB_SRC_ROOT_FOLDER}/generator
    NO_CMAKE_FIND_ROOT_PATH
)
mark_as_advanced(NANOPB_GENERATOR_SOURCE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nanopb DEFAULT_MSG
  NANOPB_INCLUDE_DIRS
  NANOPB_SRCS NANOPB_HDRS
  NANOPB_GENERATOR_SOURCE_DIR
  PROTOBUF_PROTOC_EXECUTABLE
  )
