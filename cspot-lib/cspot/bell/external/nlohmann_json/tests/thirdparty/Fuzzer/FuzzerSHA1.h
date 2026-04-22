

#ifndef LLVM_FUZZER_SHA1_H
#define LLVM_FUZZER_SHA1_H

#include "FuzzerDefs.h"
#include <cstddef>
#include <stdint.h>

namespace fuzzer {

static const int kSHA1NumBytes = 20;

void ComputeSHA1(const uint8_t *Data, size_t Len, uint8_t *Out);

std::string Sha1ToString(const uint8_t Sha1[kSHA1NumBytes]);

std::string Hash(const Unit &U);

}

#endif
