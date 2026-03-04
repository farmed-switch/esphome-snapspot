# ---- Bootloader detection diagnostics ----
# This message block will appear in CI logs and tell us EXACTLY what
# variables look like during the bootloader sub-build.
message(STATUS "snapspot CMakeLists: CMAKE_PROJECT_NAME='${CMAKE_PROJECT_NAME}'")
message(STATUS "snapspot CMakeLists: CMAKE_BINARY_DIR='${CMAKE_BINARY_DIR}'")
message(STATUS "snapspot CMakeLists: CMAKE_CURRENT_BINARY_DIR='${CMAKE_CURRENT_BINARY_DIR}'")
idf_build_get_property(idf_build_dir BUILD_DIR)
message(STATUS "snapspot CMakeLists: idf BUILD_DIR='${idf_build_dir}'")

# Detect bootloader build using multiple methods since pioarduino may not name
# the sub-project "bootloader" like vanilla IDF does.
set(_SNAPSPOT_IS_BOOTLOADER FALSE)
if(CMAKE_PROJECT_NAME STREQUAL "bootloader")
    set(_SNAPSPOT_IS_BOOTLOADER TRUE)
    message(STATUS "snapspot: bootloader detected via CMAKE_PROJECT_NAME")
endif()
if(CMAKE_BINARY_DIR MATCHES "bootloader")
    set(_SNAPSPOT_IS_BOOTLOADER TRUE)
    message(STATUS "snapspot: bootloader detected via CMAKE_BINARY_DIR path")
endif()
if(CMAKE_CURRENT_BINARY_DIR MATCHES "bootloader")
    set(_SNAPSPOT_IS_BOOTLOADER TRUE)
    message(STATUS "snapspot: bootloader detected via CMAKE_CURRENT_BINARY_DIR path")
endif()

if(_SNAPSPOT_IS_BOOTLOADER)
    message(STATUS "snapspot: skipping component (bootloader build)")
    idf_component_register()  # CONFIG_ONLY — no sources, no libs
    return()
endif()

idf_component_register(
    SRCS "snapspot_stub.c"
    INCLUDE_DIRS "."
)

# Only link prebuilt archives for the firmware build, never for the bootloader.
if(NOT _SNAPSPOT_IS_BOOTLOADER)
    set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

    if(IDF_VERSION_MAJOR EQUAL 5 AND IDF_VERSION_MINOR EQUAL 5)
        set(SNAPSPOT_LIB "${LIB_DIR}/libsnapspot-idf5.5.2-${IDF_TARGET}.a")
        set(CSPOT_LIB    "${LIB_DIR}/libcspot-idf5.5.2-${IDF_TARGET}.a")
    elseif(IDF_VERSION_MAJOR EQUAL 5 AND IDF_VERSION_MINOR EQUAL 4)
        set(SNAPSPOT_LIB "${LIB_DIR}/libsnapspot-idf5.4.0-${IDF_TARGET}.a")
        set(CSPOT_LIB    "${LIB_DIR}/libcspot-idf5.4.0-${IDF_TARGET}.a")
    else()
        file(GLOB SNAPSPOT_LIBS "${LIB_DIR}/libsnapspot-idf*.a")
        file(GLOB CSPOT_LIBS    "${LIB_DIR}/libcspot-idf*.a")
        list(SORT SNAPSPOT_LIBS ORDER DESCENDING)
        list(SORT CSPOT_LIBS    ORDER DESCENDING)
        list(GET SNAPSPOT_LIBS 0 SNAPSPOT_LIB)
        list(GET CSPOT_LIBS    0 CSPOT_LIB)
    endif()

    # target_link_libraries INTERFACE: used by native ESP-IDF (idf.py build) builds.
    # pioarduino ignores INTERFACE_LINK_LIBRARIES; for pioarduino the .a files are
    # injected via a PlatformIO extra_script in __init__.py (snapspot_link.py).
    target_link_libraries(${COMPONENT_LIB} INTERFACE
        ${SNAPSPOT_LIB}
        ${CSPOT_LIB}
    )
endif() # NOT bootloader
