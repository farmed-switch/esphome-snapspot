

#ifndef PB_H_INCLUDED
#define PB_H_INCLUDED

#define NANOPB_VERSION "nanopb-0.4.6-dev"

#ifdef PB_SYSTEM_HEADER
#include PB_SYSTEM_HEADER
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#ifdef PB_ENABLE_MALLOC
#include <stdlib.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(PB_NO_PACKED_STRUCTS)

#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed
#elif defined(__GNUC__) || defined(__clang__)

#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed __attribute__((packed))
#elif defined(__ICCARM__) || defined(__CC_ARM)

#   define PB_PACKED_STRUCT_START _Pragma("pack(push, 1)")
#   define PB_PACKED_STRUCT_END _Pragma("pack(pop)")
#   define pb_packed
#elif defined(_MSC_VER) && (_MSC_VER >= 1500)

#   define PB_PACKED_STRUCT_START __pragma(pack(push, 1))
#   define PB_PACKED_STRUCT_END __pragma(pack(pop))
#   define pb_packed
#else

#   define PB_PACKED_STRUCT_START
#   define PB_PACKED_STRUCT_END
#   define pb_packed
#endif

#ifndef PB_LITTLE_ENDIAN_8BIT
#if ((defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || \
     (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || \
      defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) || \
      defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) || \
      defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM)) \
     && CHAR_BIT == 8
#define PB_LITTLE_ENDIAN_8BIT 1
#endif
#endif

#ifndef PB_UNUSED
#define PB_UNUSED(x) (void)(x)
#endif

#ifndef PB_PROGMEM
#ifdef __AVR__
#include <avr/pgmspace.h>
#define PB_PROGMEM             PROGMEM
#define PB_PROGMEM_READU32(x)  pgm_read_dword(&x)
#else
#define PB_PROGMEM
#define PB_PROGMEM_READU32(x)  (x)
#endif
#endif

#ifndef PB_NO_STATIC_ASSERT
#  ifndef PB_STATIC_ASSERT
#    if defined(__ICCARM__)

#      define PB_STATIC_ASSERT(COND,MSG) static_assert(COND,#MSG);
#    elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L

#      define PB_STATIC_ASSERT(COND,MSG) _Static_assert(COND,#MSG);
#    else

#      define PB_STATIC_ASSERT(COND,MSG) typedef char PB_STATIC_ASSERT_MSG(MSG, __LINE__, __COUNTER__)[(COND)?1:-1];
#      define PB_STATIC_ASSERT_MSG(MSG, LINE, COUNTER) PB_STATIC_ASSERT_MSG_(MSG, LINE, COUNTER)
#      define PB_STATIC_ASSERT_MSG_(MSG, LINE, COUNTER) pb_static_assertion_##MSG##_##LINE##_##COUNTER
#    endif
#  endif
#else

#  define PB_STATIC_ASSERT(COND,MSG)
#endif

#ifndef PB_MAX_REQUIRED_FIELDS
#define PB_MAX_REQUIRED_FIELDS 64
#endif

#if PB_MAX_REQUIRED_FIELDS < 64
#error You should not lower PB_MAX_REQUIRED_FIELDS from the default value (64).
#endif

#ifdef PB_WITHOUT_64BIT
#ifdef PB_CONVERT_DOUBLE_FLOAT

#undef PB_CONVERT_DOUBLE_FLOAT
#endif
#endif

typedef uint_least8_t pb_type_t;

#define PB_LTYPE_BOOL    0x00U
#define PB_LTYPE_VARINT  0x01U
#define PB_LTYPE_UVARINT 0x02U
#define PB_LTYPE_SVARINT 0x03U
#define PB_LTYPE_FIXED32 0x04U
#define PB_LTYPE_FIXED64 0x05U

#define PB_LTYPE_LAST_PACKABLE 0x05U

#define PB_LTYPE_BYTES 0x06U

#define PB_LTYPE_STRING 0x07U

#define PB_LTYPE_SUBMESSAGE 0x08U

#define PB_LTYPE_SUBMSG_W_CB 0x09U

#define PB_LTYPE_EXTENSION 0x0AU

#define PB_LTYPE_FIXED_LENGTH_BYTES 0x0BU

#define PB_LTYPES_COUNT 0x0CU
#define PB_LTYPE_MASK 0x0FU

#define PB_HTYPE_REQUIRED 0x00U
#define PB_HTYPE_OPTIONAL 0x10U
#define PB_HTYPE_SINGULAR 0x10U
#define PB_HTYPE_REPEATED 0x20U
#define PB_HTYPE_FIXARRAY 0x20U
#define PB_HTYPE_ONEOF    0x30U
#define PB_HTYPE_MASK     0x30U

#define PB_ATYPE_STATIC   0x00U
#define PB_ATYPE_POINTER  0x80U
#define PB_ATYPE_CALLBACK 0x40U
#define PB_ATYPE_MASK     0xC0U

#define PB_ATYPE(x) ((x) & PB_ATYPE_MASK)
#define PB_HTYPE(x) ((x) & PB_HTYPE_MASK)
#define PB_LTYPE(x) ((x) & PB_LTYPE_MASK)
#define PB_LTYPE_IS_SUBMSG(x) (PB_LTYPE(x) == PB_LTYPE_SUBMESSAGE || \
                               PB_LTYPE(x) == PB_LTYPE_SUBMSG_W_CB)

#if defined(PB_FIELD_32BIT)
    typedef uint32_t pb_size_t;
    typedef int32_t pb_ssize_t;
#else
    typedef uint_least16_t pb_size_t;
    typedef int_least16_t pb_ssize_t;
#endif
#define PB_SIZE_MAX ((pb_size_t)-1)

typedef uint_least8_t pb_byte_t;

typedef struct pb_istream_s pb_istream_t;
typedef struct pb_ostream_s pb_ostream_t;
typedef struct pb_field_iter_s pb_field_iter_t;

typedef struct pb_msgdesc_s pb_msgdesc_t;
struct pb_msgdesc_s {
    const uint32_t *field_info;
    const pb_msgdesc_t * const * submsg_info;
    const pb_byte_t *default_value;

    bool (*field_callback)(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_iter_t *field);

    pb_size_t field_count;
    pb_size_t required_field_count;
    pb_size_t largest_tag;
};

struct pb_field_iter_s {
    const pb_msgdesc_t *descriptor;
    void *message;

    pb_size_t index;
    pb_size_t field_info_index;
    pb_size_t required_field_index;
    pb_size_t submessage_index;

    pb_size_t tag;
    pb_size_t data_size;
    pb_size_t array_size;
    pb_type_t type;

    void *pField;
    void *pData;
    void *pSize;

    const pb_msgdesc_t *submsg_desc;
};

typedef pb_field_iter_t pb_field_t;

#ifndef PB_WITHOUT_64BIT
PB_STATIC_ASSERT(sizeof(int64_t) == 2 * sizeof(int32_t), INT64_T_WRONG_SIZE)
PB_STATIC_ASSERT(sizeof(uint64_t) == 2 * sizeof(uint32_t), UINT64_T_WRONG_SIZE)
#endif

#define PB_BYTES_ARRAY_T(n) struct { pb_size_t size; pb_byte_t bytes[n]; }
#define PB_BYTES_ARRAY_T_ALLOCSIZE(n) ((size_t)n + offsetof(pb_bytes_array_t, bytes))

struct pb_bytes_array_s {
    pb_size_t size;
    pb_byte_t bytes[1];
};
typedef struct pb_bytes_array_s pb_bytes_array_t;

typedef struct pb_callback_s pb_callback_t;
struct pb_callback_s {

    union {
        bool (*decode)(pb_istream_t *stream, const pb_field_t *field, void **arg);
        bool (*encode)(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
    } funcs;

    void *arg;
};

extern bool pb_default_field_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field);

typedef enum {
    PB_WT_VARINT = 0,
    PB_WT_64BIT  = 1,
    PB_WT_STRING = 2,
    PB_WT_32BIT  = 5,
    PB_WT_PACKED = 255
} pb_wire_type_t;

typedef struct pb_extension_type_s pb_extension_type_t;
typedef struct pb_extension_s pb_extension_t;
struct pb_extension_type_s {

    bool (*decode)(pb_istream_t *stream, pb_extension_t *extension,
                   uint32_t tag, pb_wire_type_t wire_type);

    bool (*encode)(pb_ostream_t *stream, const pb_extension_t *extension);

    const void *arg;
};

struct pb_extension_s {

    const pb_extension_type_t *type;

    void *dest;

    pb_extension_t *next;

    bool found;
};

#define pb_extension_init_zero {NULL,NULL,NULL,false}

#ifdef PB_ENABLE_MALLOC
#   ifndef pb_realloc
#       define pb_realloc(ptr, size) realloc(ptr, size)
#   endif
#   ifndef pb_free
#       define pb_free(ptr) free(ptr)
#   endif
#endif

#define PB_PROTO_HEADER_VERSION 40

#define pb_membersize(st, m) (sizeof ((st*)0)->m)

#define pb_arraysize(st, m) (pb_membersize(st, m) / pb_membersize(st, m[0]))

#define pb_delta(st, m1, m2) ((int)offsetof(st, m1) - (int)offsetof(st, m2))

#define PB_EXPAND(x) x

#define PB_BIND(msgname, structname, width) \
    const uint32_t structname ## _field_info[] PB_PROGMEM = \
    { \
        msgname ## _FIELDLIST(PB_GEN_FIELD_INFO_ ## width, structname) \
        0 \
    }; \
    const pb_msgdesc_t* const structname ## _submsg_info[] = \
    { \
        msgname ## _FIELDLIST(PB_GEN_SUBMSG_INFO, structname) \
        NULL \
    }; \
    const pb_msgdesc_t structname ## _msg = \
    { \
       structname ## _field_info, \
       structname ## _submsg_info, \
       msgname ## _DEFAULT, \
       msgname ## _CALLBACK, \
       0 msgname ## _FIELDLIST(PB_GEN_FIELD_COUNT, structname), \
       0 msgname ## _FIELDLIST(PB_GEN_REQ_FIELD_COUNT, structname), \
       0 msgname ## _FIELDLIST(PB_GEN_LARGEST_TAG, structname), \
    }; \
    msgname ## _FIELDLIST(PB_GEN_FIELD_INFO_ASSERT_ ## width, structname)

#define PB_GEN_FIELD_COUNT(structname, atype, htype, ltype, fieldname, tag) +1
#define PB_GEN_REQ_FIELD_COUNT(structname, atype, htype, ltype, fieldname, tag) \
    + (PB_HTYPE_ ## htype == PB_HTYPE_REQUIRED)
#define PB_GEN_LARGEST_TAG(structname, atype, htype, ltype, fieldname, tag) \
    * 0 + tag

#define PB_GEN_FIELD_INFO_1(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_1(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_2(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_2(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_4(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_4(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_8(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_8(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_AUTO(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_AUTO2(PB_FIELDINFO_WIDTH_AUTO(_PB_ATYPE_ ## atype, _PB_HTYPE_ ## htype, _PB_LTYPE_ ## ltype), \
                   tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_FIELDINFO_AUTO2(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size)

#define PB_FIELDINFO_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_ ## width(tag, type, data_offset, data_size, size_offset, array_size)

#define PB_GEN_FIELD_INFO_ASSERT_1(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_1(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_2(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_2(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_4(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_4(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_8(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_8(tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_GEN_FIELD_INFO_ASSERT_AUTO(structname, atype, htype, ltype, fieldname, tag) \
    PB_FIELDINFO_ASSERT_AUTO2(PB_FIELDINFO_WIDTH_AUTO(_PB_ATYPE_ ## atype, _PB_HTYPE_ ## htype, _PB_LTYPE_ ## ltype), \
                   tag, PB_ATYPE_ ## atype | PB_HTYPE_ ## htype | PB_LTYPE_MAP_ ## ltype, \
                   PB_DATA_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_DATA_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_SIZE_OFFSET_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname), \
                   PB_ARRAY_SIZE_ ## atype(_PB_HTYPE_ ## htype, structname, fieldname))

#define PB_FIELDINFO_ASSERT_AUTO2(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_ASSERT_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size)

#define PB_FIELDINFO_ASSERT_AUTO3(width, tag, type, data_offset, data_size, size_offset, array_size) \
    PB_FIELDINFO_ASSERT_ ## width(tag, type, data_offset, data_size, size_offset, array_size)

#define PB_DATA_OFFSET_STATIC(htype, structname, fieldname) PB_DO ## htype(structname, fieldname)
#define PB_DATA_OFFSET_POINTER(htype, structname, fieldname) PB_DO ## htype(structname, fieldname)
#define PB_DATA_OFFSET_CALLBACK(htype, structname, fieldname) PB_DO ## htype(structname, fieldname)
#define PB_DO_PB_HTYPE_REQUIRED(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_SINGULAR(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_ONEOF(structname, fieldname) offsetof(structname, PB_ONEOF_NAME(FULL, fieldname))
#define PB_DO_PB_HTYPE_OPTIONAL(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_REPEATED(structname, fieldname) offsetof(structname, fieldname)
#define PB_DO_PB_HTYPE_FIXARRAY(structname, fieldname) offsetof(structname, fieldname)

#define PB_SIZE_OFFSET_STATIC(htype, structname, fieldname) PB_SO ## htype(structname, fieldname)
#define PB_SIZE_OFFSET_POINTER(htype, structname, fieldname) PB_SO_PTR ## htype(structname, fieldname)
#define PB_SIZE_OFFSET_CALLBACK(htype, structname, fieldname) PB_SO_CB ## htype(structname, fieldname)
#define PB_SO_PB_HTYPE_REQUIRED(structname, fieldname) 0
#define PB_SO_PB_HTYPE_SINGULAR(structname, fieldname) 0
#define PB_SO_PB_HTYPE_ONEOF(structname, fieldname) PB_SO_PB_HTYPE_ONEOF2(structname, PB_ONEOF_NAME(FULL, fieldname), PB_ONEOF_NAME(UNION, fieldname))
#define PB_SO_PB_HTYPE_ONEOF2(structname, fullname, unionname) PB_SO_PB_HTYPE_ONEOF3(structname, fullname, unionname)
#define PB_SO_PB_HTYPE_ONEOF3(structname, fullname, unionname) pb_delta(structname, fullname, which_ ## unionname)
#define PB_SO_PB_HTYPE_OPTIONAL(structname, fieldname) pb_delta(structname, fieldname, has_ ## fieldname)
#define PB_SO_PB_HTYPE_REPEATED(structname, fieldname) pb_delta(structname, fieldname, fieldname ## _count)
#define PB_SO_PB_HTYPE_FIXARRAY(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_REQUIRED(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_SINGULAR(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_ONEOF(structname, fieldname) PB_SO_PB_HTYPE_ONEOF(structname, fieldname)
#define PB_SO_PTR_PB_HTYPE_OPTIONAL(structname, fieldname) 0
#define PB_SO_PTR_PB_HTYPE_REPEATED(structname, fieldname) PB_SO_PB_HTYPE_REPEATED(structname, fieldname)
#define PB_SO_PTR_PB_HTYPE_FIXARRAY(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_REQUIRED(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_SINGULAR(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_ONEOF(structname, fieldname) PB_SO_PB_HTYPE_ONEOF(structname, fieldname)
#define PB_SO_CB_PB_HTYPE_OPTIONAL(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_REPEATED(structname, fieldname) 0
#define PB_SO_CB_PB_HTYPE_FIXARRAY(structname, fieldname) 0

#define PB_ARRAY_SIZE_STATIC(htype, structname, fieldname) PB_AS ## htype(structname, fieldname)
#define PB_ARRAY_SIZE_POINTER(htype, structname, fieldname) PB_AS_PTR ## htype(structname, fieldname)
#define PB_ARRAY_SIZE_CALLBACK(htype, structname, fieldname) 1
#define PB_AS_PB_HTYPE_REQUIRED(structname, fieldname) 1
#define PB_AS_PB_HTYPE_SINGULAR(structname, fieldname) 1
#define PB_AS_PB_HTYPE_OPTIONAL(structname, fieldname) 1
#define PB_AS_PB_HTYPE_ONEOF(structname, fieldname) 1
#define PB_AS_PB_HTYPE_REPEATED(structname, fieldname) pb_arraysize(structname, fieldname)
#define PB_AS_PB_HTYPE_FIXARRAY(structname, fieldname) pb_arraysize(structname, fieldname)
#define PB_AS_PTR_PB_HTYPE_REQUIRED(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_SINGULAR(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_OPTIONAL(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_ONEOF(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_REPEATED(structname, fieldname) 1
#define PB_AS_PTR_PB_HTYPE_FIXARRAY(structname, fieldname) pb_arraysize(structname, fieldname[0])

#define PB_DATA_SIZE_STATIC(htype, structname, fieldname) PB_DS ## htype(structname, fieldname)
#define PB_DATA_SIZE_POINTER(htype, structname, fieldname) PB_DS_PTR ## htype(structname, fieldname)
#define PB_DATA_SIZE_CALLBACK(htype, structname, fieldname) PB_DS_CB ## htype(structname, fieldname)
#define PB_DS_PB_HTYPE_REQUIRED(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_PB_HTYPE_SINGULAR(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_PB_HTYPE_OPTIONAL(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_PB_HTYPE_ONEOF(structname, fieldname) pb_membersize(structname, PB_ONEOF_NAME(FULL, fieldname))
#define PB_DS_PB_HTYPE_REPEATED(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PB_HTYPE_FIXARRAY(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_REQUIRED(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_SINGULAR(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_OPTIONAL(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_ONEOF(structname, fieldname) pb_membersize(structname, PB_ONEOF_NAME(FULL, fieldname)[0])
#define PB_DS_PTR_PB_HTYPE_REPEATED(structname, fieldname) pb_membersize(structname, fieldname[0])
#define PB_DS_PTR_PB_HTYPE_FIXARRAY(structname, fieldname) pb_membersize(structname, fieldname[0][0])
#define PB_DS_CB_PB_HTYPE_REQUIRED(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_SINGULAR(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_OPTIONAL(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_ONEOF(structname, fieldname) pb_membersize(structname, PB_ONEOF_NAME(FULL, fieldname))
#define PB_DS_CB_PB_HTYPE_REPEATED(structname, fieldname) pb_membersize(structname, fieldname)
#define PB_DS_CB_PB_HTYPE_FIXARRAY(structname, fieldname) pb_membersize(structname, fieldname)

#define PB_ONEOF_NAME(type, tuple) PB_EXPAND(PB_ONEOF_NAME_ ## type tuple)
#define PB_ONEOF_NAME_UNION(unionname,membername,fullname) unionname
#define PB_ONEOF_NAME_MEMBER(unionname,membername,fullname) membername
#define PB_ONEOF_NAME_FULL(unionname,membername,fullname) fullname

#define PB_GEN_SUBMSG_INFO(structname, atype, htype, ltype, fieldname, tag) \
    PB_SUBMSG_INFO_ ## htype(_PB_LTYPE_ ## ltype, structname, fieldname)

#define PB_SUBMSG_INFO_REQUIRED(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_SINGULAR(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_OPTIONAL(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_ONEOF(ltype, structname, fieldname) PB_SUBMSG_INFO_ONEOF2(ltype, structname, PB_ONEOF_NAME(UNION, fieldname), PB_ONEOF_NAME(MEMBER, fieldname))
#define PB_SUBMSG_INFO_ONEOF2(ltype, structname, unionname, membername) PB_SUBMSG_INFO_ONEOF3(ltype, structname, unionname, membername)
#define PB_SUBMSG_INFO_ONEOF3(ltype, structname, unionname, membername) PB_SI ## ltype(structname ## _ ## unionname ## _ ## membername ## _MSGTYPE)
#define PB_SUBMSG_INFO_REPEATED(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SUBMSG_INFO_FIXARRAY(ltype, structname, fieldname) PB_SI ## ltype(structname ## _ ## fieldname ## _MSGTYPE)
#define PB_SI_PB_LTYPE_BOOL(t)
#define PB_SI_PB_LTYPE_BYTES(t)
#define PB_SI_PB_LTYPE_DOUBLE(t)
#define PB_SI_PB_LTYPE_ENUM(t)
#define PB_SI_PB_LTYPE_UENUM(t)
#define PB_SI_PB_LTYPE_FIXED32(t)
#define PB_SI_PB_LTYPE_FIXED64(t)
#define PB_SI_PB_LTYPE_FLOAT(t)
#define PB_SI_PB_LTYPE_INT32(t)
#define PB_SI_PB_LTYPE_INT64(t)
#define PB_SI_PB_LTYPE_MESSAGE(t)  PB_SUBMSG_DESCRIPTOR(t)
#define PB_SI_PB_LTYPE_MSG_W_CB(t) PB_SUBMSG_DESCRIPTOR(t)
#define PB_SI_PB_LTYPE_SFIXED32(t)
#define PB_SI_PB_LTYPE_SFIXED64(t)
#define PB_SI_PB_LTYPE_SINT32(t)
#define PB_SI_PB_LTYPE_SINT64(t)
#define PB_SI_PB_LTYPE_STRING(t)
#define PB_SI_PB_LTYPE_UINT32(t)
#define PB_SI_PB_LTYPE_UINT64(t)
#define PB_SI_PB_LTYPE_EXTENSION(t)
#define PB_SI_PB_LTYPE_FIXED_LENGTH_BYTES(t)
#define PB_SUBMSG_DESCRIPTOR(t)    &(t ## _msg),

#define PB_FIELDINFO_1(tag, type, data_offset, data_size, size_offset, array_size) \
    (0 | (((tag) << 2) & 0xFF) | ((type) << 8) | (((uint32_t)(data_offset) & 0xFF) << 16) | \
     (((uint32_t)(size_offset) & 0x0F) << 24) | (((uint32_t)(data_size) & 0x0F) << 28)),

#define PB_FIELDINFO_2(tag, type, data_offset, data_size, size_offset, array_size) \
    (1 | (((tag) << 2) & 0xFF) | ((type) << 8) | (((uint32_t)(array_size) & 0xFFF) << 16) | (((uint32_t)(size_offset) & 0x0F) << 28)), \
    (((uint32_t)(data_offset) & 0xFFFF) | (((uint32_t)(data_size) & 0xFFF) << 16) | (((uint32_t)(tag) & 0x3c0) << 22)),

#define PB_FIELDINFO_4(tag, type, data_offset, data_size, size_offset, array_size) \
    (2 | (((tag) << 2) & 0xFF) | ((type) << 8) | (((uint32_t)(array_size) & 0xFFFF) << 16)), \
    ((uint32_t)(int_least8_t)(size_offset) | (((uint32_t)(tag) << 2) & 0xFFFFFF00)), \
    (data_offset), (data_size),

#define PB_FIELDINFO_8(tag, type, data_offset, data_size, size_offset, array_size) \
    (3 | (((tag) << 2) & 0xFF) | ((type) << 8)), \
    ((uint32_t)(int_least8_t)(size_offset) | (((uint32_t)(tag) << 2) & 0xFFFFFF00)), \
    (data_offset), (data_size), (array_size), 0, 0, 0,

#define PB_FITS(value,bits) ((uint32_t)(value) < ((uint32_t)1<<bits))
#define PB_FIELDINFO_ASSERT_1(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,6) && PB_FITS(data_offset,8) && PB_FITS(size_offset,4) && PB_FITS(data_size,4) && PB_FITS(array_size,1), FIELDINFO_DOES_NOT_FIT_width1_field ## tag)

#define PB_FIELDINFO_ASSERT_2(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,10) && PB_FITS(data_offset,16) && PB_FITS(size_offset,4) && PB_FITS(data_size,12) && PB_FITS(array_size,12), FIELDINFO_DOES_NOT_FIT_width2_field ## tag)

#ifndef PB_FIELD_32BIT

#define PB_FIELDINFO_ASSERT_4(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,16) && PB_FITS(data_offset,16) && PB_FITS((int_least8_t)size_offset,8) && PB_FITS(data_size,16) && PB_FITS(array_size,16), FIELDINFO_DOES_NOT_FIT_width4_field ## tag)

#define PB_FIELDINFO_ASSERT_8(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,16) && PB_FITS(data_offset,16) && PB_FITS((int_least8_t)size_offset,8) && PB_FITS(data_size,16) && PB_FITS(array_size,16), FIELDINFO_DOES_NOT_FIT_width8_field ## tag)
#else

#define PB_FIELDINFO_ASSERT_4(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,30) && PB_FITS(data_offset,31) && PB_FITS(size_offset,8) && PB_FITS(data_size,31) && PB_FITS(array_size,16), FIELDINFO_DOES_NOT_FIT_width4_field ## tag)

#define PB_FIELDINFO_ASSERT_8(tag, type, data_offset, data_size, size_offset, array_size) \
    PB_STATIC_ASSERT(PB_FITS(tag,30) && PB_FITS(data_offset,31) && PB_FITS(size_offset,8) && PB_FITS(data_size,31) && PB_FITS(array_size,31), FIELDINFO_DOES_NOT_FIT_width8_field ## tag)
#endif

#define PB_FIELDINFO_WIDTH_AUTO(atype, htype, ltype) PB_FI_WIDTH ## atype(htype, ltype)
#define PB_FI_WIDTH_PB_ATYPE_STATIC(htype, ltype) PB_FI_WIDTH ## htype(ltype)
#define PB_FI_WIDTH_PB_ATYPE_POINTER(htype, ltype) PB_FI_WIDTH ## htype(ltype)
#define PB_FI_WIDTH_PB_ATYPE_CALLBACK(htype, ltype) 2
#define PB_FI_WIDTH_PB_HTYPE_REQUIRED(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_SINGULAR(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_OPTIONAL(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_ONEOF(ltype) PB_FI_WIDTH ## ltype
#define PB_FI_WIDTH_PB_HTYPE_REPEATED(ltype) 2
#define PB_FI_WIDTH_PB_HTYPE_FIXARRAY(ltype) 2
#define PB_FI_WIDTH_PB_LTYPE_BOOL      1
#define PB_FI_WIDTH_PB_LTYPE_BYTES     2
#define PB_FI_WIDTH_PB_LTYPE_DOUBLE    1
#define PB_FI_WIDTH_PB_LTYPE_ENUM      1
#define PB_FI_WIDTH_PB_LTYPE_UENUM     1
#define PB_FI_WIDTH_PB_LTYPE_FIXED32   1
#define PB_FI_WIDTH_PB_LTYPE_FIXED64   1
#define PB_FI_WIDTH_PB_LTYPE_FLOAT     1
#define PB_FI_WIDTH_PB_LTYPE_INT32     1
#define PB_FI_WIDTH_PB_LTYPE_INT64     1
#define PB_FI_WIDTH_PB_LTYPE_MESSAGE   2
#define PB_FI_WIDTH_PB_LTYPE_MSG_W_CB  2
#define PB_FI_WIDTH_PB_LTYPE_SFIXED32  1
#define PB_FI_WIDTH_PB_LTYPE_SFIXED64  1
#define PB_FI_WIDTH_PB_LTYPE_SINT32    1
#define PB_FI_WIDTH_PB_LTYPE_SINT64    1
#define PB_FI_WIDTH_PB_LTYPE_STRING    2
#define PB_FI_WIDTH_PB_LTYPE_UINT32    1
#define PB_FI_WIDTH_PB_LTYPE_UINT64    1
#define PB_FI_WIDTH_PB_LTYPE_EXTENSION 1
#define PB_FI_WIDTH_PB_LTYPE_FIXED_LENGTH_BYTES 2

#define PB_LTYPE_MAP_BOOL               PB_LTYPE_BOOL
#define PB_LTYPE_MAP_BYTES              PB_LTYPE_BYTES
#define PB_LTYPE_MAP_DOUBLE             PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_ENUM               PB_LTYPE_VARINT
#define PB_LTYPE_MAP_UENUM              PB_LTYPE_UVARINT
#define PB_LTYPE_MAP_FIXED32            PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_FIXED64            PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_FLOAT              PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_INT32              PB_LTYPE_VARINT
#define PB_LTYPE_MAP_INT64              PB_LTYPE_VARINT
#define PB_LTYPE_MAP_MESSAGE            PB_LTYPE_SUBMESSAGE
#define PB_LTYPE_MAP_MSG_W_CB           PB_LTYPE_SUBMSG_W_CB
#define PB_LTYPE_MAP_SFIXED32           PB_LTYPE_FIXED32
#define PB_LTYPE_MAP_SFIXED64           PB_LTYPE_FIXED64
#define PB_LTYPE_MAP_SINT32             PB_LTYPE_SVARINT
#define PB_LTYPE_MAP_SINT64             PB_LTYPE_SVARINT
#define PB_LTYPE_MAP_STRING             PB_LTYPE_STRING
#define PB_LTYPE_MAP_UINT32             PB_LTYPE_UVARINT
#define PB_LTYPE_MAP_UINT64             PB_LTYPE_UVARINT
#define PB_LTYPE_MAP_EXTENSION          PB_LTYPE_EXTENSION
#define PB_LTYPE_MAP_FIXED_LENGTH_BYTES PB_LTYPE_FIXED_LENGTH_BYTES

#ifdef PB_NO_ERRMSG
#define PB_SET_ERROR(stream, msg) PB_UNUSED(stream)
#define PB_GET_ERROR(stream) "(errmsg disabled)"
#else
#define PB_SET_ERROR(stream, msg) (stream->errmsg = (stream)->errmsg ? (stream)->errmsg : (msg))
#define PB_GET_ERROR(stream) ((stream)->errmsg ? (stream)->errmsg : "(none)")
#endif

#define PB_RETURN_ERROR(stream, msg) return PB_SET_ERROR(stream, msg), false

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#if __cplusplus >= 201103L
#define PB_CONSTEXPR constexpr
#else
#define PB_CONSTEXPR
#endif

#if __cplusplus >= 201703L
#define PB_INLINE_CONSTEXPR inline constexpr
#else
#define PB_INLINE_CONSTEXPR PB_CONSTEXPR
#endif

namespace nanopb {

template <typename GenMessageT> struct MessageDescriptor;
}
#endif

#endif
