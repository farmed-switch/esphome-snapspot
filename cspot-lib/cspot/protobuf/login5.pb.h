

#ifndef PB_COMPONENTS_CSPOT_CSPOT_LIB_CSPOT_PROTOBUF_LOGIN5_PB_H_INCLUDED
#define PB_COMPONENTS_CSPOT_CSPOT_LIB_CSPOT_PROTOBUF_LOGIN5_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

typedef enum _LoginError {
    LoginError_UNKNOWN_ERROR = 0,
    LoginError_INVALID_CREDENTIALS = 1,
    LoginError_BAD_REQUEST = 2,
    LoginError_UNSUPPORTED_LOGIN_PROTOCOL = 3,
    LoginError_TIMEOUT = 4,
    LoginError_UNKNOWN_IDENTIFIER = 5,
    LoginError_TOO_MANY_ATTEMPTS = 6,
    LoginError_INVALID_PHONENUMBER = 7,
    LoginError_TRY_AGAIN_LATER = 8
} LoginError;

typedef struct _ClientInfo {
    pb_callback_t client_id;
    pb_callback_t device_id;
} ClientInfo;

typedef struct _StoredCredential {
    pb_callback_t username;
    pb_callback_t data;
} StoredCredential;

typedef struct _LoginOk {
    char *access_token;
    bool has_access_token_expires_in;
    int32_t access_token_expires_in;
} LoginOk;

typedef struct _LoginRequest {
    ClientInfo client_info;
    pb_size_t which_login_method;
    union {
        StoredCredential stored_credential;
    } login_method;
} LoginRequest;

typedef struct _LoginResponse {
    pb_size_t which_response;
    union {
        LoginOk ok;
        LoginError error;
    } response;
} LoginResponse;

#define _LoginError_MIN LoginError_UNKNOWN_ERROR
#define _LoginError_MAX LoginError_TRY_AGAIN_LATER
#define _LoginError_ARRAYSIZE ((LoginError)(LoginError_TRY_AGAIN_LATER+1))

#ifdef __cplusplus
extern "C" {
#endif

#define StoredCredential_init_default            {{{NULL}, NULL}, {{NULL}, NULL}}
#define ClientInfo_init_default                  {{{NULL}, NULL}, {{NULL}, NULL}}
#define LoginRequest_init_default                {ClientInfo_init_default, 0, {StoredCredential_init_default}}
#define LoginOk_init_default                     {NULL, false, 0}
#define LoginResponse_init_default               {0, {LoginOk_init_default}}
#define StoredCredential_init_zero               {{{NULL}, NULL}, {{NULL}, NULL}}
#define ClientInfo_init_zero                     {{{NULL}, NULL}, {{NULL}, NULL}}
#define LoginRequest_init_zero                   {ClientInfo_init_zero, 0, {StoredCredential_init_zero}}
#define LoginOk_init_zero                        {NULL, false, 0}
#define LoginResponse_init_zero                  {0, {LoginOk_init_zero}}

#define ClientInfo_client_id_tag                 1
#define ClientInfo_device_id_tag                 2
#define StoredCredential_username_tag            1
#define StoredCredential_data_tag                2
#define LoginOk_access_token_tag                 2
#define LoginOk_access_token_expires_in_tag      4
#define LoginRequest_client_info_tag             1
#define LoginRequest_stored_credential_tag       100
#define LoginResponse_ok_tag                     1
#define LoginResponse_error_tag                  2

#define StoredCredential_FIELDLIST(X, a) \
X(a, CALLBACK, REQUIRED, STRING,   username,          1) \
X(a, CALLBACK, REQUIRED, BYTES,    data,              2)
#define StoredCredential_CALLBACK pb_default_field_callback
#define StoredCredential_DEFAULT NULL

#define ClientInfo_FIELDLIST(X, a) \
X(a, CALLBACK, REQUIRED, STRING,   client_id,         1) \
X(a, CALLBACK, REQUIRED, STRING,   device_id,         2)
#define ClientInfo_CALLBACK pb_default_field_callback
#define ClientInfo_DEFAULT NULL

#define LoginRequest_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, MESSAGE,  client_info,       1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (login_method,stored_credential,login_method.stored_credential), 100)
#define LoginRequest_CALLBACK NULL
#define LoginRequest_DEFAULT NULL
#define LoginRequest_client_info_MSGTYPE ClientInfo
#define LoginRequest_login_method_stored_credential_MSGTYPE StoredCredential

#define LoginOk_FIELDLIST(X, a) \
X(a, POINTER,  REQUIRED, STRING,   access_token,      2) \
X(a, STATIC,   OPTIONAL, INT32,    access_token_expires_in,   4)
#define LoginOk_CALLBACK NULL
#define LoginOk_DEFAULT NULL

#define LoginResponse_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (response,ok,response.ok),   1) \
X(a, STATIC,   ONEOF,    UENUM,    (response,error,response.error),   2)
#define LoginResponse_CALLBACK NULL
#define LoginResponse_DEFAULT NULL
#define LoginResponse_response_ok_MSGTYPE LoginOk

extern const pb_msgdesc_t StoredCredential_msg;
extern const pb_msgdesc_t ClientInfo_msg;
extern const pb_msgdesc_t LoginRequest_msg;
extern const pb_msgdesc_t LoginOk_msg;
extern const pb_msgdesc_t LoginResponse_msg;

#define StoredCredential_fields &StoredCredential_msg
#define ClientInfo_fields &ClientInfo_msg
#define LoginRequest_fields &LoginRequest_msg
#define LoginOk_fields &LoginOk_msg
#define LoginResponse_fields &LoginResponse_msg

#ifdef __cplusplus
}
#endif

#endif
