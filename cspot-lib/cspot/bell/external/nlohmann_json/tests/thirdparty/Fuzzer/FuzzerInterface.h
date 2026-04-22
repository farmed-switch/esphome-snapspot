

#ifndef LLVM_FUZZER_INTERFACE_H
#define LLVM_FUZZER_INTERFACE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);

int LLVMFuzzerInitialize(int *argc, char ***argv);

size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size, size_t MaxSize,
                               unsigned int Seed);

size_t LLVMFuzzerCustomCrossOver(const uint8_t *Data1, size_t Size1,
                                 const uint8_t *Data2, size_t Size2,
                                 uint8_t *Out, size_t MaxOutSize,
                                 unsigned int Seed);

size_t LLVMFuzzerMutate(uint8_t *Data, size_t Size, size_t MaxSize);

#ifdef __cplusplus
}
#endif

#endif
