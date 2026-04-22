

#ifndef LLVM_FUZZER_EXT_FUNCTIONS_H
#define LLVM_FUZZER_EXT_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

namespace fuzzer {

struct ExternalFunctions {

  ExternalFunctions();

#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  RETURN_TYPE(*NAME) FUNC_SIG = nullptr

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
};
}

#endif
