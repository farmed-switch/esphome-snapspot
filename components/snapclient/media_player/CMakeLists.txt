cmake_minimum_required(VERSION 3.16)

set(_SC_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(_SC_LIB "${_SC_LIB_DIR}/libsnapclient-idf${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}-${IDF_TARGET}.a")

if(NOT EXISTS "${_SC_LIB}")
    file(GLOB _SC_CANDIDATES "${_SC_LIB_DIR}/libsnapclient-*-${IDF_TARGET}.a")
    list(SORT _SC_CANDIDATES ORDER DESCENDING)
    list(GET _SC_CANDIDATES 0 _SC_LIB)
endif()

string(FIND "${CMAKE_CURRENT_BINARY_DIR}" "/bootloader/" _bootloader_pos)
if(_bootloader_pos EQUAL -1)
    add_library(snapclient_prebuilt STATIC IMPORTED GLOBAL)
    set_target_properties(snapclient_prebuilt PROPERTIES IMPORTED_LOCATION "${_SC_LIB}")

    idf_component_register(SRCS "snapclient_stub.c" INCLUDE_DIRS ".")

    target_link_libraries(${COMPONENT_LIB} INTERFACE snapclient_prebuilt)
    target_link_options(${COMPONENT_LIB} INTERFACE
        "-Wl,-u,_ZTVN7esphome10snapclient19SnapClientComponentE"
    )
else()
    idf_component_register()
endif()
