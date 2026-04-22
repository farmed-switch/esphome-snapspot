

#include "FuzzerCorpus.h"
#include "FuzzerDefs.h"
#include "FuzzerDictionary.h"
#include "FuzzerExtFunctions.h"
#include "FuzzerIO.h"
#include "FuzzerTracePC.h"
#include "FuzzerValueBitMap.h"
#include <map>
#include <sanitizer/coverage_interface.h>
#include <set>
#include <sstream>

namespace fuzzer {

TracePC TPC;

void TracePC::HandleTrace(uint32_t *Guard, uintptr_t PC) {
  uint32_t Idx = *Guard;
  if (!Idx) return;
  PCs[Idx % kNumPCs] = PC;
  Counters[Idx % kNumCounters]++;
}

size_t TracePC::GetTotalPCCoverage() {
  size_t Res = 0;
  for (size_t i = 1; i < GetNumPCs(); i++)
    if (PCs[i])
      Res++;
  return Res;
}

void TracePC::HandleInit(uint32_t *Start, uint32_t *Stop) {
  if (Start == Stop || *Start) return;
  assert(NumModules < sizeof(Modules) / sizeof(Modules[0]));
  for (uint32_t *P = Start; P < Stop; P++)
    *P = ++NumGuards;
  Modules[NumModules].Start = Start;
  Modules[NumModules].Stop = Stop;
  NumModules++;
}

void TracePC::PrintModuleInfo() {
  Printf("INFO: Loaded %zd modules (%zd guards): ", NumModules, NumGuards);
  for (size_t i = 0; i < NumModules; i++)
    Printf("[%p, %p), ", Modules[i].Start, Modules[i].Stop);
  Printf("\n");
}

void TracePC::HandleCallerCallee(uintptr_t Caller, uintptr_t Callee) {
  const uintptr_t kBits = 12;
  const uintptr_t kMask = (1 << kBits) - 1;
  uintptr_t Idx = (Caller & kMask) | ((Callee & kMask) << kBits);
  HandleValueProfile(Idx);
}

static bool IsInterestingCoverageFile(std::string &File) {
  if (File.find("compiler-rt/lib/") != std::string::npos)
    return false;
  if (File.find("/usr/lib/") != std::string::npos)
    return false;
  if (File.find("/usr/include/") != std::string::npos)
    return false;
  if (File == "<null>")
    return false;
  return true;
}

void TracePC::PrintNewPCs() {
  if (DoPrintNewPCs) {
    if (!PrintedPCs)
      PrintedPCs = new std::set<uintptr_t>;
    for (size_t i = 1; i < GetNumPCs(); i++)
      if (PCs[i] && PrintedPCs->insert(PCs[i]).second)
        PrintPC("\tNEW_PC: %p %F %L\n", "\tNEW_PC: %p\n", PCs[i]);
  }
}

void TracePC::PrintCoverage() {
  if (!EF->__sanitizer_symbolize_pc ||
      !EF->__sanitizer_get_module_and_offset_for_pc) {
    Printf("INFO: __sanitizer_symbolize_pc or "
           "__sanitizer_get_module_and_offset_for_pc is not available,"
           " not printing coverage\n");
    return;
  }
  std::map<std::string, std::vector<uintptr_t>> CoveredPCsPerModule;
  std::map<std::string, uintptr_t> ModuleOffsets;
  std::set<std::string> CoveredDirs, CoveredFiles, CoveredFunctions,
      CoveredLines;
  Printf("COVERAGE:\n");
  for (size_t i = 1; i < GetNumPCs(); i++) {
    if (!PCs[i]) continue;
    std::string FileStr = DescribePC("%s", PCs[i]);
    if (!IsInterestingCoverageFile(FileStr)) continue;
    std::string FixedPCStr = DescribePC("%p", PCs[i]);
    std::string FunctionStr = DescribePC("%F", PCs[i]);
    std::string LineStr = DescribePC("%l", PCs[i]);
    char ModulePathRaw[4096] = "";
    void *OffsetRaw = nullptr;
    if (!EF->__sanitizer_get_module_and_offset_for_pc(
            reinterpret_cast<void *>(PCs[i]), ModulePathRaw,
            sizeof(ModulePathRaw), &OffsetRaw))
      continue;
    std::string Module = ModulePathRaw;
    uintptr_t FixedPC = std::stol(FixedPCStr, 0, 16);
    uintptr_t PcOffset = reinterpret_cast<uintptr_t>(OffsetRaw);
    ModuleOffsets[Module] = FixedPC - PcOffset;
    CoveredPCsPerModule[Module].push_back(PcOffset);
    CoveredFunctions.insert(FunctionStr);
    CoveredFiles.insert(FileStr);
    CoveredDirs.insert(DirName(FileStr));
    if (!CoveredLines.insert(FileStr + ":" + LineStr).second)
      continue;
    Printf("COVERED: %s %s:%s\n", FunctionStr.c_str(),
           FileStr.c_str(), LineStr.c_str());
  }

  std::string CoveredDirsStr;
  for (auto &Dir : CoveredDirs) {
    if (!CoveredDirsStr.empty())
      CoveredDirsStr += ",";
    CoveredDirsStr += Dir;
  }
  Printf("COVERED_DIRS: %s\n", CoveredDirsStr.c_str());

  for (auto &M : CoveredPCsPerModule) {
    std::set<std::string> UncoveredFiles, UncoveredFunctions;
    std::map<std::string, std::set<int> > UncoveredLines;
    auto &ModuleName = M.first;
    auto &CoveredOffsets = M.second;
    uintptr_t ModuleOffset = ModuleOffsets[ModuleName];
    std::sort(CoveredOffsets.begin(), CoveredOffsets.end());
    Printf("MODULE_WITH_COVERAGE: %s\n", ModuleName.c_str());

    std::string Cmd = "objdump -d " + ModuleName +
        " | grep 'call.*__sanitizer_cov_trace_pc_guard' | awk -F: '{print $1}'";
    std::string SanCovOutput;
    if (!ExecuteCommandAndReadOutput(Cmd, &SanCovOutput)) {
      Printf("INFO: Command failed: %s\n", Cmd.c_str());
      continue;
    }
    std::istringstream ISS(SanCovOutput);
    std::string S;
    while (std::getline(ISS, S, '\n')) {
      uintptr_t PcOffset = std::stol(S, 0, 16);
      if (!std::binary_search(CoveredOffsets.begin(), CoveredOffsets.end(),
                              PcOffset)) {
        uintptr_t PC = ModuleOffset + PcOffset;
        auto FileStr = DescribePC("%s", PC);
        if (!IsInterestingCoverageFile(FileStr)) continue;
        if (CoveredFiles.count(FileStr) == 0) {
          UncoveredFiles.insert(FileStr);
          continue;
        }
        auto FunctionStr = DescribePC("%F", PC);
        if (CoveredFunctions.count(FunctionStr) == 0) {
          UncoveredFunctions.insert(FunctionStr);
          continue;
        }
        std::string LineStr = DescribePC("%l", PC);
        uintptr_t Line = std::stoi(LineStr);
        std::string FileLineStr = FileStr + ":" + LineStr;
        if (CoveredLines.count(FileLineStr) == 0)
          UncoveredLines[FunctionStr + " " + FileStr].insert(Line);
      }
    }
    for (auto &FileLine: UncoveredLines)
      for (int Line : FileLine.second)
        Printf("UNCOVERED_LINE: %s:%d\n", FileLine.first.c_str(), Line);
    for (auto &Func : UncoveredFunctions)
      Printf("UNCOVERED_FUNC: %s\n", Func.c_str());
    for (auto &File : UncoveredFiles)
      Printf("UNCOVERED_FILE: %s\n", File.c_str());
  }
}

void TracePC::DumpCoverage() {
  __sanitizer_dump_coverage(PCs, GetNumPCs());
}

ATTRIBUTE_NO_SANITIZE_MEMORY
void TracePC::AddValueForMemcmp(void *caller_pc, const void *s1, const void *s2,
                              size_t n) {
  if (!n) return;
  size_t Len = std::min(n, (size_t)32);
  const uint8_t *A1 = reinterpret_cast<const uint8_t *>(s1);
  const uint8_t *A2 = reinterpret_cast<const uint8_t *>(s2);
  size_t I = 0;
  for (; I < Len; I++)
    if (A1[I] != A2[I])
      break;
  size_t PC = reinterpret_cast<size_t>(caller_pc);
  size_t Idx = I;

  TPC.HandleValueProfile((PC & 4095) | (Idx << 12));
}

ATTRIBUTE_NO_SANITIZE_MEMORY
void TracePC::AddValueForStrcmp(void *caller_pc, const char *s1, const char *s2,
                              size_t n) {
  if (!n) return;
  size_t Len = std::min(n, (size_t)32);
  const uint8_t *A1 = reinterpret_cast<const uint8_t *>(s1);
  const uint8_t *A2 = reinterpret_cast<const uint8_t *>(s2);
  size_t I = 0;
  for (; I < Len; I++)
    if (A1[I] != A2[I] || A1[I] == 0)
      break;
  size_t PC = reinterpret_cast<size_t>(caller_pc);
  size_t Idx = I;

  TPC.HandleValueProfile((PC & 4095) | (Idx << 12));
}

template <class T>
ATTRIBUTE_TARGET_POPCNT
#ifdef __clang__
__attribute__((always_inline))
#endif
void TracePC::HandleCmp(void *PC, T Arg1, T Arg2) {
  uintptr_t PCuint = reinterpret_cast<uintptr_t>(PC);
  uint64_t ArgXor = Arg1 ^ Arg2;
  uint64_t ArgDistance = __builtin_popcountl(ArgXor) + 1;
  uintptr_t Idx = ((PCuint & 4095) + 1) * ArgDistance;
  if (sizeof(T) == 4)
      TORC4.Insert(ArgXor, Arg1, Arg2);
  else if (sizeof(T) == 8)
      TORC8.Insert(ArgXor, Arg1, Arg2);
  HandleValueProfile(Idx);
}

}

extern "C" {
__attribute__((visibility("default")))
void __sanitizer_cov_trace_pc_guard(uint32_t *Guard) {
  uintptr_t PC = (uintptr_t)__builtin_return_address(0);
  fuzzer::TPC.HandleTrace(Guard, PC);
}

__attribute__((visibility("default")))
void __sanitizer_cov_trace_pc_guard_init(uint32_t *Start, uint32_t *Stop) {
  fuzzer::TPC.HandleInit(Start, Stop);
}

__attribute__((visibility("default")))
void __sanitizer_cov_trace_pc_indir(uintptr_t Callee) {
  uintptr_t PC = (uintptr_t)__builtin_return_address(0);
  fuzzer::TPC.HandleCallerCallee(PC, Callee);
}

__attribute__((visibility("default")))
void __sanitizer_cov_trace_cmp8(uint64_t Arg1, uint64_t Arg2) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Arg1, Arg2);
}
__attribute__((visibility("default")))
void __sanitizer_cov_trace_cmp4(uint32_t Arg1, uint32_t Arg2) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Arg1, Arg2);
}
__attribute__((visibility("default")))
void __sanitizer_cov_trace_cmp2(uint16_t Arg1, uint16_t Arg2) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Arg1, Arg2);
}
__attribute__((visibility("default")))
void __sanitizer_cov_trace_cmp1(uint8_t Arg1, uint8_t Arg2) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Arg1, Arg2);
}

__attribute__((visibility("default")))
void __sanitizer_cov_trace_switch(uint64_t Val, uint64_t *Cases) {

  static thread_local size_t Counter;
  Counter++;
  uint64_t N = Cases[0];
  uint64_t *Vals = Cases + 2;
  char *PC = (char*)__builtin_return_address(0);

  size_t Nlog = sizeof(long) * 8 - __builtin_clzl((long)N);
  size_t PowerOfTwoGeN = 1U << Nlog;
  assert(PowerOfTwoGeN >= N);
  size_t Idx = Counter & (PowerOfTwoGeN - 1);
  if (Idx >= N)
    Idx -= N;
  assert(Idx < N);
  uint64_t TwoIn32 = 1ULL << 32;
  if ((Val | Vals[Idx]) < TwoIn32)
    fuzzer::TPC.HandleCmp(PC + Idx, static_cast<uint32_t>(Val),
                          static_cast<uint32_t>(Vals[Idx]));
  else
    fuzzer::TPC.HandleCmp(PC + Idx, Val, Vals[Idx]);
}

__attribute__((visibility("default")))
void __sanitizer_cov_trace_div4(uint32_t Val) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Val, (uint32_t)0);
}
__attribute__((visibility("default")))
void __sanitizer_cov_trace_div8(uint64_t Val) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Val, (uint64_t)0);
}
__attribute__((visibility("default")))
void __sanitizer_cov_trace_gep(uintptr_t Idx) {
  fuzzer::TPC.HandleCmp(__builtin_return_address(0), Idx, (uintptr_t)0);
}

}
