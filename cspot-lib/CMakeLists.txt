# ============================================================
# CSpot Component - Spotify Connect for ESP32
# ============================================================


set(CSPOT_ROOT "cspot")
set(BELL_ROOT "${CSPOT_ROOT}/bell")

file(GLOB CSPOT_SRCS "${CSPOT_ROOT}/src/*.cpp")
file(GLOB BELL_UTIL_SRCS "${BELL_ROOT}/main/utilities/*.cpp" "${BELL_ROOT}/main/utilities/*.c")
file(GLOB BELL_PLATFORM_SRCS "${BELL_ROOT}/main/platform/esp/*.cpp")

set(BELL_IO_SRCS
    "${BELL_ROOT}/main/io/CircularBuffer.cpp"
    "${BELL_ROOT}/main/io/HTTPClient.cpp"
    "${BELL_ROOT}/main/io/SocketStream.cpp"
    "${BELL_ROOT}/main/io/TLSSocket.cpp"
    "${BELL_ROOT}/main/io/BinaryStream.cpp"
    "${BELL_ROOT}/main/io/URLParser.cpp"
    "${BELL_ROOT}/main/io/X509Bundle.cpp"
    "${BELL_ROOT}/main/io/picohttpparser.c"
)

set(NANOPB_SRCS
    "${BELL_ROOT}/external/nanopb/pb_common.c"
    "${BELL_ROOT}/external/nanopb/pb_encode.c"
    "${BELL_ROOT}/external/nanopb/pb_decode.c"
)

set(CJSON_SRCS "${BELL_ROOT}/external/cJSON/cJSON.c")

# Tremor (Vorbis decoder) sources
set(TREMOR_SRCS
    "${BELL_ROOT}/external/tremor/bitwise.c"
    "${BELL_ROOT}/external/tremor/codebook.c"
    "${BELL_ROOT}/external/tremor/dsp.c"
    "${BELL_ROOT}/external/tremor/floor0.c"
    "${BELL_ROOT}/external/tremor/floor1.c"
    "${BELL_ROOT}/external/tremor/floor_lookup.c"
    "${BELL_ROOT}/external/tremor/framing.c"
    "${BELL_ROOT}/external/tremor/info.c"
    "${BELL_ROOT}/external/tremor/mapping0.c"
    "${BELL_ROOT}/external/tremor/tremor_mdct.c"
    "${BELL_ROOT}/external/tremor/misc.c"
    "${BELL_ROOT}/external/tremor/res012.c"
    "${BELL_ROOT}/external/tremor/vorbisfile.c"
)

# fmt (formatting library) sources
set(FMT_SRCS
    "${BELL_ROOT}/external/fmt/src/format.cc"
    "${BELL_ROOT}/external/fmt/src/os.cc"
)

# Protobuf files (hardcoded)
set(PROTO_SRCS
    "${CSPOT_ROOT}/protobuf/authentication.pb.c"
    "${CSPOT_ROOT}/protobuf/keyexchange.pb.c"
    "${CSPOT_ROOT}/protobuf/login5.pb.c"
    "${CSPOT_ROOT}/protobuf/mercury.pb.c"
    "${CSPOT_ROOT}/protobuf/metadata.pb.c"
    "${CSPOT_ROOT}/protobuf/spirc.pb.c"
)

idf_component_register(
    SRCS 
        cspot_wrapper.cpp
        ${CSPOT_SRCS}
        ${BELL_UTIL_SRCS}
        ${BELL_IO_SRCS}
        ${BELL_PLATFORM_SRCS}
        ${NANOPB_SRCS}
        ${CJSON_SRCS}
        ${TREMOR_SRCS}
        ${FMT_SRCS}
        ${PROTO_SRCS}
    INCLUDE_DIRS
        "."
        "${CSPOT_ROOT}"
        "${CSPOT_ROOT}/include"
        "${CSPOT_ROOT}/protobuf"
        "${BELL_ROOT}/main/utilities/include"
        "${BELL_ROOT}/main/io/include"
        "${BELL_ROOT}/main/platform"
        "${BELL_ROOT}/main/audio-sinks/include"
        "${BELL_ROOT}/external/nanopb"
        "${BELL_ROOT}/external/cJSON"
        "${BELL_ROOT}/external/tremor"
        "${BELL_ROOT}/external/fmt/include"
    PRIV_INCLUDE_DIRS
        "${CSPOT_ROOT}/src"
        "${BELL_ROOT}/main"
    REQUIRES
        mbedtls
        nvs_flash
        esp_http_client
        esp_timer
        json
        mdns
        pthread
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC
    PB_ENABLE_MALLOC
    PB_FIELD_32BIT
    BELL_ONLY_CJSON
    BELL_DISABLE_CODECS
    BELL_DISABLE_REGEX
    ESP_PLATFORM
)

# C++ specific flags (only for .cpp files)
target_compile_options(${COMPONENT_LIB} PRIVATE 
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>
    $<$<COMPILE_LANGUAGE:CXX>:-fexceptions>
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-missing-field-initializers
    -Wno-type-limits
    -Wno-sign-compare
    -Wno-dangling-reference
)

# Suppress Tremor (C library) warnings treated as errors
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error
    -Wno-error=misleading-indentation
    -Wno-error=maybe-uninitialized
    -Wno-maybe-uninitialized
    -Wno-misleading-indentation
    -Wno-char-subscripts
    -Wno-error=char-subscripts
    -Wno-error=dangling-reference
    -Wno-unused-label
    -Wno-error=unused-label
    -Wno-implicit-fallthrough
    -Wno-error=implicit-fallthrough
)
