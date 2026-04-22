#pragma once

#include <mbedtls/x509_crt.h>
#include <stddef.h>
#include <cstdint>
#include <vector>

#include "mbedtls/ssl.h"

namespace bell::X509Bundle {

int crtCheckCertificate(mbedtls_x509_crt* child, const uint8_t* pub_key_buf,
                        size_t pub_key_len);

int crtVerifyCallback(void* buf, mbedtls_x509_crt* crt, int depth,
                      uint32_t* flags);

void init(const uint8_t* x509_bundle, size_t bundle_size);

void attach(mbedtls_ssl_config* conf);

bool shouldVerify();
};