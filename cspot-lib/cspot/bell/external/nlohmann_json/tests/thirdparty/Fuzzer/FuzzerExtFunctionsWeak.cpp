

#include "FuzzerDefs.h"
#if LIBFUZZER_LINUX

#include "FuzzerExtFunctions.h"
#include "FuzzerIO.h"

extern "C" {

#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  __attribute__((weak)) RETURN_TYPE NAME FUNC_SIG

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
}

using namespace fuzzer;

static void CheckFnPtr(void *FnPtr, const char *FnName, bool WarnIfMissing) {
  if (FnPtr == nullptr && WarnIfMissing) {
    Printf("WARNING: Failed to find function \"%s\".\n", FnName);
  }
}

namespace fuzzer {

ExternalFunctions::ExternalFunctions() {
#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  this->NAME = ::NAME;                                                         \
  CheckFnPtr((void *)::NAME, #NAME, WARN);

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
}

}

#endif
