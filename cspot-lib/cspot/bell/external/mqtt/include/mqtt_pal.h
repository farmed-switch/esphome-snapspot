#if !defined(__MQTT_PAL_H__)
#define __MQTT_PAL_H__

#if defined(__cplusplus)
extern "C" {
#endif
#include <sys/types.h>

#if defined(__unix__) || defined(__APPLE__) || defined(__NuttX__) || defined(ESP_PLATFORM)
    #include <limits.h>
    #include <string.h>
    #include <stdarg.h>
    #include <time.h>
    #include <arpa/inet.h>
    #include <pthread.h>
    #include <sys/socket.h>

    #define MQTT_PAL_HTONS(s) htons(s)
    #define MQTT_PAL_NTOHS(s) ntohs(s)

    #define MQTT_PAL_TIME() time(NULL)

    typedef time_t mqtt_pal_time_t;
    typedef pthread_mutex_t mqtt_pal_mutex_t;

    #define MQTT_PAL_MUTEX_INIT(mtx_ptr) pthread_mutex_init(mtx_ptr, NULL)
    #define MQTT_PAL_MUTEX_LOCK(mtx_ptr) pthread_mutex_lock(mtx_ptr)
    #define MQTT_PAL_MUTEX_UNLOCK(mtx_ptr) pthread_mutex_unlock(mtx_ptr)

    #if !defined(MQTT_USE_CUSTOM_SOCKET_HANDLE)
        #if defined(MQTT_USE_MBEDTLS)
            struct mbedtls_ssl_context;
            typedef struct mbedtls_ssl_context *mqtt_pal_socket_handle;
        #elif defined(MQTT_USE_WOLFSSL)
            #include <wolfssl/ssl.h>
            typedef WOLFSSL* mqtt_pal_socket_handle;
        #elif defined(MQTT_USE_BIO)
            #include <openssl/bio.h>
            typedef BIO* mqtt_pal_socket_handle;
        #elif defined(MQTT_USE_BEARSSL)
            #include <bearssl.h>

            typedef struct _bearssl_context {
                br_ssl_client_context sc;
                br_x509_minimal_context xc;
                br_sslio_context ioc;
                size_t ta_count;
                br_x509_trust_anchor *anchOut;
                int fd;
                int (*low_read)(void *read_context, unsigned char *buf, size_t len);
                int (*low_write)(void *write_context, const unsigned char *buf, size_t len);
            } bearssl_context;

            typedef bearssl_context* mqtt_pal_socket_handle;
        #else
            typedef int mqtt_pal_socket_handle;
        #endif
    #endif
#elif defined(_MSC_VER) || defined(WIN32)
    #include <limits.h>
    #include <winsock2.h>
    #include <windows.h>
    #include <time.h>
    #include <stdint.h>

    typedef SSIZE_T ssize_t;
    #define MQTT_PAL_HTONS(s) htons(s)
    #define MQTT_PAL_NTOHS(s) ntohs(s)

    #define MQTT_PAL_TIME() time(NULL)

    typedef time_t mqtt_pal_time_t;
    typedef CRITICAL_SECTION mqtt_pal_mutex_t;

    #define MQTT_PAL_MUTEX_INIT(mtx_ptr) InitializeCriticalSection(mtx_ptr)
    #define MQTT_PAL_MUTEX_LOCK(mtx_ptr) EnterCriticalSection(mtx_ptr)
    #define MQTT_PAL_MUTEX_UNLOCK(mtx_ptr) LeaveCriticalSection(mtx_ptr)

    #if !defined(MQTT_USE_CUSTOM_SOCKET_HANDLE)
        #if defined(MQTT_USE_BIO)
            #include <openssl/bio.h>
            typedef BIO* mqtt_pal_socket_handle;
        #else
            typedef SOCKET mqtt_pal_socket_handle;
        #endif
    #endif

#endif

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, size_t len, int flags);

ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, size_t bufsz, int flags);

#if defined(__cplusplus)
}
#endif

#endif