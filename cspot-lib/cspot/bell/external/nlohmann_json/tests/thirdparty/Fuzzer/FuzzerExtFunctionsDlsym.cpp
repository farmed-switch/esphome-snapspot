

#include "FuzzerDefs.h"
#if LIBFUZZER_APPLE

#include "FuzzerExtFunctions.h"
#include "FuzzerIO.h"
#include <dlfcn.h>

using namespace fuzzer;

template <typename T>
static T GetFnPtr(const char *FnName, bool WarnIfMissing) {
  dlerror();
  void *Fn = dlsym(RTLD_DEFAULT, FnName);
  if (Fn == nullptr) {
    if (WarnIfMissing) {
      const char *ErrorMsg = dlerror();
      Printf("WARNING: Failed to find function \"%s\".", FnName);
      if (ErrorMsg)
        Printf(" Reason %s.", ErrorMsg);
      Printf("\n");
    }
  }
  return reinterpret_cast<T>(Fn);
}

namespace fuzzer {

ExternalFunctions::ExternalFunctions() {
#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  this->NAME = GetFnPtr<decltype(ExternalFunctions::NAME)>(#NAME, WARN)

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
}

}

#endif
