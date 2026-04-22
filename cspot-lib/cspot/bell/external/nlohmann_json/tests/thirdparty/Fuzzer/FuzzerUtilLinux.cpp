

#include "FuzzerDefs.h"
#if LIBFUZZER_LINUX

#include <stdlib.h>

namespace fuzzer {

int ExecuteCommand(const std::string &Command) {
  return system(Command.c_str());
}

}

#endif
