

#ifndef CIVETWEB_HEADER_INCLUDED
#define CIVETWEB_HEADER_INCLUDED

#define CIVETWEB_VERSION "1.16"
#define CIVETWEB_VERSION_MAJOR (1)
#define CIVETWEB_VERSION_MINOR (16)
#define CIVETWEB_VERSION_PATCH (0)

#ifndef CIVETWEB_API
#if defined(_WIN32)
#if defined(CIVETWEB_DLL_EXPORTS)
#define CIVETWEB_API __declspec(dllexport)
#elif defined(CIVETWEB_DLL_IMPORTS)
#define CIVETWEB_API __declspec(dllimport)
#else
#define CIVETWEB_API
#endif
#elif __GNUC__ >= 4
#define CIVETWEB_API __attribute__((visibility("default")))
#else
#define CIVETWEB_API
#endif
#endif

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	MG_FEATURES_DEFAULT = 0x0u,

	MG_FEATURES_FILES = 0x1u,

	MG_FEATURES_TLS = 0x2u,
	MG_FEATURES_SSL = 0x2u,

	MG_FEATURES_CGI = 0x4u,

	MG_FEATURES_IPV6 = 0x8u,

	MG_FEATURES_WEBSOCKET = 0x10u,

	MG_FEATURES_LUA = 0x20u,

	MG_FEATURES_SSJS = 0x40u,

	MG_FEATURES_CACHE = 0x80u,

	MG_FEATURES_STATS = 0x100u,

	MG_FEATURES_COMPRESSION = 0x200u,

	MG_FEATURES_HTTP2 = 0x400u,

	MG_FEATURES_X_DOMAIN_SOCKET = 0x800u,

	MG_FEATURES_ALL = 0xFFFFu
};

CIVETWEB_API unsigned mg_init_library(unsigned features);

CIVETWEB_API unsigned mg_exit_library(void);

struct mg_context;
struct mg_connection;

#define MG_MAX_HEADERS (64)

struct mg_header {
	const char *name;
	const char *value;
};

struct mg_request_info {
	const char *request_method;
	const char *request_uri;
	const char *local_uri_raw;
	const char *local_uri;
	const char *http_version;
	const char *query_string;
	const char *remote_user;
	char remote_addr[48];

	long long content_length;
	int remote_port;
	int server_port;
	int is_ssl;
	void *user_data;
	void *conn_data;

	int num_headers;
	struct mg_header
	    http_headers[MG_MAX_HEADERS];

	struct mg_client_cert *client_cert;

	const char *acceptedWebSocketSubprotocol;
};

struct mg_response_info {
	int status_code;
	const char *status_text;
	const char *http_version;

	long long content_length;

	int num_headers;
	struct mg_header
	    http_headers[MG_MAX_HEADERS];
};

struct mg_client_cert {
	void *peer_cert;
	const char *subject;
	const char *issuer;
	const char *serial;
	const char *finger;
};

struct mg_callbacks {

	int (*begin_request)(struct mg_connection *);

	void (*end_request)(const struct mg_connection *, int reply_status_code);

	int (*log_message)(const struct mg_connection *, const char *message);

	int (*log_access)(const struct mg_connection *, const char *message);

	int (*init_ssl)(void *ssl_ctx, void *user_data);

	int (*init_ssl_domain)(const char *server_domain,
	                       void *ssl_ctx,
	                       void *user_data);

	int (*external_ssl_ctx)(void **ssl_ctx, void *user_data);

	int (*external_ssl_ctx_domain)(const char *server_domain,
	                               void **ssl_ctx,
	                               void *user_data);

#if defined(MG_EXPERIMENTAL_INTERFACES)

	int (*websocket_data)(struct mg_connection *,
	                      int bits,
	                      char *data,
	                      size_t data_len);
#endif

	void (*connection_close)(const struct mg_connection *);

	void (*connection_closed)(const struct mg_connection *);

	void (*init_lua)(const struct mg_connection *conn,
	                 void *lua_context,
	                 unsigned context_flags);
	void (*exit_lua)(const struct mg_connection *conn,
	                 void *lua_context,
	                 unsigned context_flags);

	int (*http_error)(struct mg_connection *conn,
	                  int status,
	                  const char *errmsg);

	void (*init_context)(const struct mg_context *ctx);

	void (*exit_context)(const struct mg_context *ctx);

	void *(*init_thread)(const struct mg_context *ctx, int thread_type);

	void (*exit_thread)(const struct mg_context *ctx,
	                    int thread_type,
	                    void *thread_pointer);

	int (*init_connection)(const struct mg_connection *conn, void **conn_data);
};

CIVETWEB_API struct mg_context *mg_start(const struct mg_callbacks *callbacks,
                                         void *user_data,
                                         const char **configuration_options);

CIVETWEB_API void mg_stop(struct mg_context *);

CIVETWEB_API int mg_start_domain(struct mg_context *ctx,
                                 const char **configuration_options);

typedef int (*mg_request_handler)(struct mg_connection *conn, void *cbdata);

CIVETWEB_API void mg_set_request_handler(struct mg_context *ctx,
                                         const char *uri,
                                         mg_request_handler handler,
                                         void *cbdata);

typedef int (*mg_websocket_connect_handler)(const struct mg_connection *,
                                            void *);
typedef void (*mg_websocket_ready_handler)(struct mg_connection *, void *);
typedef int (*mg_websocket_data_handler)(struct mg_connection *,
                                         int,
                                         char *,
                                         size_t,
                                         void *);
typedef void (*mg_websocket_close_handler)(const struct mg_connection *,
                                           void *);

struct mg_websocket_subprotocols {
	int nb_subprotocols;
	const char **subprotocols;
};

CIVETWEB_API void
mg_set_websocket_handler(struct mg_context *ctx,
                         const char *uri,
                         mg_websocket_connect_handler connect_handler,
                         mg_websocket_ready_handler ready_handler,
                         mg_websocket_data_handler data_handler,
                         mg_websocket_close_handler close_handler,
                         void *cbdata);

CIVETWEB_API void mg_set_websocket_handler_with_subprotocols(
    struct mg_context *ctx,
    const char *uri,
    struct mg_websocket_subprotocols *subprotocols,
    mg_websocket_connect_handler connect_handler,
    mg_websocket_ready_handler ready_handler,
    mg_websocket_data_handler data_handler,
    mg_websocket_close_handler close_handler,
    void *cbdata);

typedef int (*mg_authorization_handler)(struct mg_connection *conn,
                                        void *cbdata);

CIVETWEB_API void mg_set_auth_handler(struct mg_context *ctx,
                                      const char *uri,
                                      mg_authorization_handler handler,
                                      void *cbdata);

CIVETWEB_API const char *mg_get_option(const struct mg_context *ctx,
                                       const char *name);

CIVETWEB_API struct mg_context *
mg_get_context(const struct mg_connection *conn);

CIVETWEB_API void *mg_get_user_data(const struct mg_context *ctx);

CIVETWEB_API void *mg_get_user_context_data(const struct mg_connection *conn);

CIVETWEB_API void *mg_get_thread_pointer(const struct mg_connection *conn);

CIVETWEB_API void mg_set_user_connection_data(const struct mg_connection *conn,
                                              void *data);

CIVETWEB_API void *
mg_get_user_connection_data(const struct mg_connection *conn);

CIVETWEB_API int
mg_get_request_link(const struct mg_connection *conn, char *buf, size_t buflen);

struct mg_option {
	const char *name;
	int type;
	const char *default_value;
};

enum {
	MG_CONFIG_TYPE_UNKNOWN = 0x0,
	MG_CONFIG_TYPE_NUMBER = 0x1,
	MG_CONFIG_TYPE_STRING = 0x2,
	MG_CONFIG_TYPE_FILE = 0x3,
	MG_CONFIG_TYPE_DIRECTORY = 0x4,
	MG_CONFIG_TYPE_BOOLEAN = 0x5,
	MG_CONFIG_TYPE_EXT_PATTERN = 0x6,
	MG_CONFIG_TYPE_STRING_LIST = 0x7,
	MG_CONFIG_TYPE_STRING_MULTILINE = 0x8,
	MG_CONFIG_TYPE_YES_NO_OPTIONAL = 0x9
};

CIVETWEB_API const struct mg_option *mg_get_valid_options(void);

struct mg_server_port {
	int protocol;
	int port;
	int is_ssl;
	int is_redirect;
	int _reserved1;
	int _reserved2;
	int _reserved3;
	int _reserved4;
};

#define mg_server_ports mg_server_port

CIVETWEB_API int mg_get_server_ports(const struct mg_context *ctx,
                                     int size,
                                     struct mg_server_port *ports);

CIVETWEB_API int mg_modify_passwords_file(const char *passwords_file_name,
                                          const char *realm,
                                          const char *user,
                                          const char *password);

CIVETWEB_API int mg_modify_passwords_file_ha1(const char *passwords_file_name,
                                              const char *realm,
                                              const char *user,
                                              const char *ha1);

CIVETWEB_API const struct mg_request_info *
mg_get_request_info(const struct mg_connection *);

CIVETWEB_API const struct mg_response_info *
mg_get_response_info(const struct mg_connection *);

CIVETWEB_API int mg_write(struct mg_connection *, const void *buf, size_t len);

CIVETWEB_API int mg_websocket_write(struct mg_connection *conn,
                                    int opcode,
                                    const char *data,
                                    size_t data_len);

CIVETWEB_API int mg_websocket_client_write(struct mg_connection *conn,
                                           int opcode,
                                           const char *data,
                                           size_t data_len);

CIVETWEB_API void mg_lock_connection(struct mg_connection *conn);
CIVETWEB_API void mg_unlock_connection(struct mg_connection *conn);

CIVETWEB_API void mg_lock_context(struct mg_context *ctx);
CIVETWEB_API void mg_unlock_context(struct mg_context *ctx);

enum {
	MG_WEBSOCKET_OPCODE_CONTINUATION = 0x0,
	MG_WEBSOCKET_OPCODE_TEXT = 0x1,
	MG_WEBSOCKET_OPCODE_BINARY = 0x2,
	MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE = 0x8,
	MG_WEBSOCKET_OPCODE_PING = 0x9,
	MG_WEBSOCKET_OPCODE_PONG = 0xa
};

#undef PRINTF_FORMAT_STRING
#if defined(_MSC_VER) && _MSC_VER >= 1400
#include <sal.h>
#if defined(_MSC_VER) && _MSC_VER > 1400
#define PRINTF_FORMAT_STRING(s) _Printf_format_string_ s
#else
#define PRINTF_FORMAT_STRING(s) __format_string s
#endif
#else
#define PRINTF_FORMAT_STRING(s) s
#endif

#ifdef __GNUC__
#define PRINTF_ARGS(x, y) __attribute__((format(printf, x, y)))
#else
#define PRINTF_ARGS(x, y)
#endif

CIVETWEB_API int mg_printf(struct mg_connection *,
                           PRINTF_FORMAT_STRING(const char *fmt),
                           ...) PRINTF_ARGS(2, 3);

CIVETWEB_API int mg_send_chunk(struct mg_connection *conn,
                               const char *chunk,
                               unsigned int chunk_len);

CIVETWEB_API void mg_send_file(struct mg_connection *conn, const char *path);

CIVETWEB_API int mg_send_file_body(struct mg_connection *conn,
                                   const char *path);

CIVETWEB_API int mg_send_http_error(struct mg_connection *conn,
                                    int status_code,
                                    PRINTF_FORMAT_STRING(const char *fmt),
                                    ...) PRINTF_ARGS(3, 4);

CIVETWEB_API int mg_send_http_ok(struct mg_connection *conn,
                                 const char *mime_type,
                                 long long content_length);

CIVETWEB_API int mg_send_http_redirect(struct mg_connection *conn,
                                       const char *target_url,
                                       int redirect_code);

CIVETWEB_API int
mg_send_digest_access_authentication_request(struct mg_connection *conn,
                                             const char *realm);

CIVETWEB_API int
mg_check_digest_access_authentication(struct mg_connection *conn,
                                      const char *realm,
                                      const char *filename);

CIVETWEB_API void mg_send_mime_file(struct mg_connection *conn,
                                    const char *path,
                                    const char *mime_type);

CIVETWEB_API void mg_send_mime_file2(struct mg_connection *conn,
                                     const char *path,
                                     const char *mime_type,
                                     const char *additional_headers);

CIVETWEB_API long long mg_store_body(struct mg_connection *conn,
                                     const char *path);

CIVETWEB_API int mg_read(struct mg_connection *, void *buf, size_t len);

CIVETWEB_API const char *mg_get_header(const struct mg_connection *,
                                       const char *name);

CIVETWEB_API int mg_get_var(const char *data,
                            size_t data_len,
                            const char *var_name,
                            char *dst,
                            size_t dst_len);

CIVETWEB_API int mg_get_var2(const char *data,
                             size_t data_len,
                             const char *var_name,
                             char *dst,
                             size_t dst_len,
                             size_t occurrence);

CIVETWEB_API int mg_split_form_urlencoded(char *data,
                                          struct mg_header *form_fields,
                                          unsigned num_form_fields);

CIVETWEB_API int mg_get_cookie(const char *cookie,
                               const char *var_name,
                               char *buf,
                               size_t buf_len);

CIVETWEB_API struct mg_connection *
mg_download(const char *host,
            int port,
            int use_ssl,
            char *error_buffer,
            size_t error_buffer_size,
            PRINTF_FORMAT_STRING(const char *request_fmt),
            ...) PRINTF_ARGS(6, 7);

CIVETWEB_API void mg_close_connection(struct mg_connection *conn);

struct mg_form_data_handler {

	int (*field_found)(const char *key,
	                   const char *filename,
	                   char *path,
	                   size_t pathlen,
	                   void *user_data);

	int (*field_get)(const char *key,
	                 const char *value,
	                 size_t valuelen,
	                 void *user_data);

	int (*field_store)(const char *path, long long file_size, void *user_data);

	void *user_data;
};

enum {

	MG_FORM_FIELD_STORAGE_SKIP = 0x0,

	MG_FORM_FIELD_STORAGE_GET = 0x1,

	MG_FORM_FIELD_STORAGE_STORE = 0x2,

	MG_FORM_FIELD_STORAGE_ABORT = 0x10
};

enum {

	MG_FORM_FIELD_HANDLE_GET = 0x1,

	MG_FORM_FIELD_HANDLE_NEXT = 0x8,

	MG_FORM_FIELD_HANDLE_ABORT = 0x10
};

CIVETWEB_API int mg_handle_form_request(struct mg_connection *conn,
                                        struct mg_form_data_handler *fdh);

typedef void *(*mg_thread_func_t)(void *);
CIVETWEB_API int mg_start_thread(mg_thread_func_t f, void *p);

CIVETWEB_API const char *mg_get_builtin_mime_type(const char *file_name);

CIVETWEB_API const char *
mg_get_response_code_text(const struct mg_connection *conn, int response_code);

CIVETWEB_API const char *mg_version(void);

CIVETWEB_API int mg_url_decode(const char *src,
                               int src_len,
                               char *dst,
                               int dst_len,
                               int is_form_url_encoded);

CIVETWEB_API int mg_url_encode(const char *src, char *dst, size_t dst_len);

CIVETWEB_API int mg_base64_encode(const unsigned char *src,
                                  size_t src_len,
                                  char *dst,
                                  size_t *dst_len);

CIVETWEB_API int mg_base64_decode(const char *src,
                                  size_t src_len,
                                  unsigned char *dst,
                                  size_t *dst_len);

CIVETWEB_API char *mg_md5(char buf[33], ...);

#if !defined(MG_MATCH_CONTEXT_MAX_MATCHES)
#define MG_MATCH_CONTEXT_MAX_MATCHES (32)
#endif

struct mg_match_element {
	const char *str;
	size_t len;
};

struct mg_match_context {
	int case_sensitive;
	size_t num_matches;
	struct mg_match_element match[MG_MATCH_CONTEXT_MAX_MATCHES];
};

#if defined(MG_EXPERIMENTAL_INTERFACES)

CIVETWEB_API ptrdiff_t mg_match(const char *pat,
                                const char *str,
                                struct mg_match_context *mcx);
#endif

CIVETWEB_API void mg_cry(const struct mg_connection *conn,
                         PRINTF_FORMAT_STRING(const char *fmt),
                         ...) PRINTF_ARGS(2, 3);

CIVETWEB_API int mg_strcasecmp(const char *s1, const char *s2);
CIVETWEB_API int mg_strncasecmp(const char *s1, const char *s2, size_t len);

CIVETWEB_API struct mg_connection *
mg_connect_websocket_client(const char *host,
                            int port,
                            int use_ssl,
                            char *error_buffer,
                            size_t error_buffer_size,
                            const char *path,
                            const char *origin,
                            mg_websocket_data_handler data_func,
                            mg_websocket_close_handler close_func,
                            void *user_data);

CIVETWEB_API struct mg_connection *
mg_connect_websocket_client_extensions(const char *host,
                                       int port,
                                       int use_ssl,
                                       char *error_buffer,
                                       size_t error_buffer_size,
                                       const char *path,
                                       const char *origin,
                                       const char *extensions,
                                       mg_websocket_data_handler data_func,
                                       mg_websocket_close_handler close_func,
                                       void *user_data);

CIVETWEB_API struct mg_connection *mg_connect_client(const char *host,
                                                     int port,
                                                     int use_ssl,
                                                     char *error_buffer,
                                                     size_t error_buffer_size);

struct mg_client_options {
	const char *host;
	int port;
	const char *client_cert;
	const char *server_cert;
	const char *host_name;

};

CIVETWEB_API struct mg_connection *
mg_connect_client_secure(const struct mg_client_options *client_options,
                         char *error_buffer,
                         size_t error_buffer_size);

CIVETWEB_API struct mg_connection *mg_connect_websocket_client_secure(
    const struct mg_client_options *client_options,
    char *error_buffer,
    size_t error_buffer_size,
    const char *path,
    const char *origin,
    mg_websocket_data_handler data_func,
    mg_websocket_close_handler close_func,
    void *user_data);

CIVETWEB_API struct mg_connection *
mg_connect_websocket_client_secure_extensions(
    const struct mg_client_options *client_options,
    char *error_buffer,
    size_t error_buffer_size,
    const char *path,
    const char *origin,
    const char *extensions,
    mg_websocket_data_handler data_func,
    mg_websocket_close_handler close_func,
    void *user_data);

#if defined(MG_LEGACY_INTERFACE)
enum { TIMEOUT_INFINITE = -1 };
#endif
enum { MG_TIMEOUT_INFINITE = -1 };

CIVETWEB_API int mg_get_response(struct mg_connection *conn,
                                 char *ebuf,
                                 size_t ebuf_len,
                                 int timeout);

CIVETWEB_API int mg_response_header_start(struct mg_connection *conn,
                                          int status);

CIVETWEB_API int mg_response_header_add(struct mg_connection *conn,
                                        const char *header,
                                        const char *value,
                                        int value_len);

CIVETWEB_API int mg_response_header_add_lines(struct mg_connection *conn,
                                              const char *http1_headers);

CIVETWEB_API int mg_response_header_send(struct mg_connection *conn);

CIVETWEB_API unsigned mg_check_feature(unsigned feature);

CIVETWEB_API int mg_get_system_info(char *buffer, int buflen);

CIVETWEB_API int
mg_get_context_info(const struct mg_context *ctx, char *buffer, int buflen);

CIVETWEB_API void mg_disable_connection_keep_alive(struct mg_connection *conn);

#if defined(MG_EXPERIMENTAL_INTERFACES)

CIVETWEB_API int mg_get_connection_info(const struct mg_context *ctx,
                                        int idx,
                                        char *buffer,
                                        int buflen);
#endif

struct mg_error_data {
	unsigned code;
	unsigned code_sub;
	char *text;
	size_t text_buffer_size;
};

enum {

	MG_ERROR_DATA_CODE_OK = 0u,

	MG_ERROR_DATA_CODE_INVALID_PARAM = 1u,

	MG_ERROR_DATA_CODE_INVALID_OPTION = 2u,

	MG_ERROR_DATA_CODE_INIT_TLS_FAILED = 3u,

	MG_ERROR_DATA_CODE_MISSING_OPTION = 4u,

	MG_ERROR_DATA_CODE_DUPLICATE_DOMAIN = 5u,

	MG_ERROR_DATA_CODE_OUT_OF_MEMORY = 6u,

	MG_ERROR_DATA_CODE_SERVER_STOPPED = 7u,

	MG_ERROR_DATA_CODE_INIT_LIBRARY_FAILED = 8u,

	MG_ERROR_DATA_CODE_OS_ERROR = 9u,

	MG_ERROR_DATA_CODE_INIT_PORTS_FAILED = 10u,

	MG_ERROR_DATA_CODE_INIT_USER_FAILED = 11u,

	MG_ERROR_DATA_CODE_INIT_ACL_FAILED = 12u,

	MG_ERROR_DATA_CODE_INVALID_PASS_FILE = 13u,

	MG_ERROR_DATA_CODE_SCRIPT_ERROR = 14u,

	MG_ERROR_DATA_CODE_HOST_NOT_FOUND = 15u,

	MG_ERROR_DATA_CODE_CONNECT_TIMEOUT = 16u,

	MG_ERROR_DATA_CODE_CONNECT_FAILED = 17u,

	MG_ERROR_DATA_CODE_TLS_CLIENT_CERT_ERROR = 18u,

	MG_ERROR_DATA_CODE_TLS_SERVER_CERT_ERROR = 19u,

	MG_ERROR_DATA_CODE_TLS_CONNECT_ERROR = 20u
};

struct mg_init_data {
	const struct mg_callbacks *callbacks;
	void *user_data;
	const char **configuration_options;
};

#if defined(MG_EXPERIMENTAL_INTERFACES)

CIVETWEB_API struct mg_connection *
mg_connect_client2(const char *host,
                   const char *protocol,
                   int port,
                   const char *path,
                   struct mg_init_data *init,
                   struct mg_error_data *error);

CIVETWEB_API int mg_get_response2(struct mg_connection *conn,
                                  struct mg_error_data *error,
                                  int timeout);
#endif

CIVETWEB_API struct mg_context *mg_start2(struct mg_init_data *init,
                                          struct mg_error_data *error);

CIVETWEB_API int mg_start_domain2(struct mg_context *ctx,
                                  const char **configuration_options,
                                  struct mg_error_data *error);

#ifdef __cplusplus
}
#endif

#endif