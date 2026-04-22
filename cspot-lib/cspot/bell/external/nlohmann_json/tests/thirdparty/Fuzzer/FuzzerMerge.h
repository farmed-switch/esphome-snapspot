

#ifndef LLVM_FUZZER_MERGE_H
#define LLVM_FUZZER_MERGE_H

#include "FuzzerDefs.h"

#include <istream>
#include <set>

namespace fuzzer {

struct MergeFileInfo {
  std::string Name;
  size_t Size = 0;
  std::vector<uint32_t> Features;
};

struct Merger {
  std::vector<MergeFileInfo> Files;
  size_t NumFilesInFirstCorpus = 0;
  size_t FirstNotProcessedFile = 0;
  std::string LastFailure;

  bool Parse(std::istream &IS, bool ParseCoverage);
  bool Parse(const std::string &Str, bool ParseCoverage);
  void ParseOrExit(std::istream &IS, bool ParseCoverage);
  size_t Merge(std::vector<std::string> *NewFiles);
};

}

#endif
