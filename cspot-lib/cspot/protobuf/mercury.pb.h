

#ifndef PB_COMPONENTS_CSPOT_CSPOT_LIB_CSPOT_PROTOBUF_MERCURY_PB_H_INCLUDED
#define PB_COMPONENTS_CSPOT_CSPOT_LIB_CSPOT_PROTOBUF_MERCURY_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

typedef struct _Header {
    bool has_uri;
    char uri[256];
    bool has_method;
    char method[64];
} Header;

#ifdef __cplusplus
extern "C" {
#endif

#define Header_init_default                      {false, "", false, ""}
#define Header_init_zero                         {false, "", false, ""}

#define Header_uri_tag                           1
#define Header_method_tag                        3

#define Header_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, STRING,   uri,               1) \
X(a, STATIC,   OPTIONAL, STRING,   method,            3)
#define Header_CALLBACK NULL
#define Header_DEFAULT NULL

extern const pb_msgdesc_t Header_msg;

#define Header_fields &Header_msg

#define Header_size                              323

#ifdef __cplusplus
}
#endif

#endif
