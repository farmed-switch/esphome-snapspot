cmake_minimum_required(VERSION 3.16)

set(_SP_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(_SP_LIB "${_SP_LIB_DIR}/libspotifyconnect-idf${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}-${IDF_TARGET}.a")
set(_CP_LIB "${_SP_LIB_DIR}/libcspot-idf${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}-${IDF_TARGET}.a")

if(NOT EXISTS "${_SP_LIB}")
    file(GLOB _SP_CANDIDATES "${_SP_LIB_DIR}/libspotifyconnect-*-${IDF_TARGET}.a")
    list(SORT _SP_CANDIDATES ORDER DESCENDING)
    list(GET _SP_CANDIDATES 0 _SP_LIB)
endif()

if(NOT EXISTS "${_CP_LIB}")
    file(GLOB _CP_CANDIDATES "${_SP_LIB_DIR}/libcspot-*-${IDF_TARGET}.a")
    list(SORT _CP_CANDIDATES ORDER DESCENDING)
    list(GET _CP_CANDIDATES 0 _CP_LIB)
endif()

string(FIND "${CMAKE_CURRENT_BINARY_DIR}" "/bootloader/" _bootloader_pos)
if(_bootloader_pos EQUAL -1)
    add_library(spotifyconnect_prebuilt STATIC IMPORTED GLOBAL)
    set_target_properties(spotifyconnect_prebuilt PROPERTIES IMPORTED_LOCATION "${_SP_LIB}")

    add_library(cspot_prebuilt STATIC IMPORTED GLOBAL)
    set_target_properties(cspot_prebuilt PROPERTIES IMPORTED_LOCATION "${_CP_LIB}")

    idf_component_register(SRCS "spotify_connect_stub.c" INCLUDE_DIRS ".")

    target_link_libraries(${COMPONENT_LIB} INTERFACE spotifyconnect_prebuilt cspot_prebuilt)
    target_link_options(${COMPONENT_LIB} INTERFACE
        "-Wl,-u,_ZTVN7esphome15spotify_connect23SpotifyConnectComponentE"
    )
else()
    idf_component_register()
endif()
