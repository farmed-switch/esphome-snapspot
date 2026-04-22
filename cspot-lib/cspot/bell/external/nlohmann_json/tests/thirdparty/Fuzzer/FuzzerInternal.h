

#ifndef LLVM_FUZZER_INTERNAL_H
#define LLVM_FUZZER_INTERNAL_H

#include "FuzzerDefs.h"
#include "FuzzerExtFunctions.h"
#include "FuzzerInterface.h"
#include "FuzzerOptions.h"
#include "FuzzerSHA1.h"
#include "FuzzerValueBitMap.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <string.h>

namespace fuzzer {

using namespace std::chrono;

class Fuzzer {
public:

  struct Coverage {
    Coverage() { Reset(); }

    void Reset() {
      BlockCoverage = 0;
      CallerCalleeCoverage = 0;
      CounterBitmapBits = 0;
      CounterBitmap.clear();
      VPMap.Reset();
    }

    size_t BlockCoverage;
    size_t CallerCalleeCoverage;

    size_t CounterBitmapBits;
    std::vector<uint8_t> CounterBitmap;
    ValueBitMap VPMap;
  };

  Fuzzer(UserCallback CB, InputCorpus &Corpus, MutationDispatcher &MD,
         FuzzingOptions Options);
  ~Fuzzer();
  void Loop();
  void MinimizeCrashLoop(const Unit &U);
  void ShuffleAndMinimize(UnitVector *V);
  void InitializeTraceState();
  void RereadOutputCorpus(size_t MaxSize);

  size_t secondsSinceProcessStartUp() {
    return duration_cast<seconds>(system_clock::now() - ProcessStartTime)
        .count();
  }

  bool TimedOut() {
    return Options.MaxTotalTimeSec > 0 &&
           secondsSinceProcessStartUp() >
               static_cast<size_t>(Options.MaxTotalTimeSec);
  }

  size_t execPerSec() {
    size_t Seconds = secondsSinceProcessStartUp();
    return Seconds ? TotalNumberOfRuns / Seconds : 0;
  }

  size_t getTotalNumberOfRuns() { return TotalNumberOfRuns; }

  static void StaticAlarmCallback();
  static void StaticCrashSignalCallback();
  static void StaticInterruptCallback();

  void ExecuteCallback(const uint8_t *Data, size_t Size);
  size_t RunOne(const uint8_t *Data, size_t Size);

  void Merge(const std::vector<std::string> &Corpora);
  void CrashResistantMerge(const std::vector<std::string> &Args,
                           const std::vector<std::string> &Corpora);
  void CrashResistantMergeInternalStep(const std::string &ControlFilePath);

  UnitVector FindExtraUnits(const UnitVector &Initial, const UnitVector &Extra);
  MutationDispatcher &GetMD() { return MD; }
  void PrintFinalStats();
  void SetMaxInputLen(size_t MaxInputLen);
  void SetMaxMutationLen(size_t MaxMutationLen);
  void RssLimitCallback();

  void ResetCoverage();

  bool InFuzzingThread() const { return IsMyThread; }
  size_t GetCurrentUnitInFuzzingThead(const uint8_t **Data) const;
  void TryDetectingAMemoryLeak(const uint8_t *Data, size_t Size,
                               bool DuringInitialCorpusExecution);

  void HandleMalloc(size_t Size);

private:
  void AlarmCallback();
  void CrashCallback();
  void InterruptCallback();
  void MutateAndTestOne();
  void ReportNewCoverage(InputInfo *II, const Unit &U);
  size_t RunOne(const Unit &U) { return RunOne(U.data(), U.size()); }
  void WriteToOutputCorpus(const Unit &U);
  void WriteUnitToFileWithPrefix(const Unit &U, const char *Prefix);
  void PrintStats(const char *Where, const char *End = "\n", size_t Units = 0);
  void PrintStatusForNewUnit(const Unit &U);
  void ShuffleCorpus(UnitVector *V);
  void AddToCorpus(const Unit &U);
  void CheckExitOnSrcPosOrItem();

  void StartTraceRecording();

  void StopTraceRecording();

  void SetDeathCallback();
  static void StaticDeathCallback();
  void DumpCurrentUnit(const char *Prefix);
  void DeathCallback();

  void ResetEdgeCoverage();
  void ResetCounters();
  void PrepareCounters(Fuzzer::Coverage *C);
  bool RecordMaxCoverage(Fuzzer::Coverage *C);

  void AllocateCurrentUnitData();
  uint8_t *CurrentUnitData = nullptr;
  std::atomic<size_t> CurrentUnitSize;
  uint8_t BaseSha1[kSHA1NumBytes];
  bool RunningCB = false;

  size_t TotalNumberOfRuns = 0;
  size_t NumberOfNewUnitsAdded = 0;

  bool HasMoreMallocsThanFrees = false;
  size_t NumberOfLeakDetectionAttempts = 0;

  UserCallback CB;
  InputCorpus &Corpus;
  MutationDispatcher &MD;
  FuzzingOptions Options;

  system_clock::time_point ProcessStartTime = system_clock::now();
  system_clock::time_point UnitStartTime, UnitStopTime;
  long TimeOfLongestUnitInSeconds = 0;
  long EpochOfLastReadOfOutputCorpus = 0;

  Coverage MaxCoverage;

  size_t MaxInputLen = 0;
  size_t MaxMutationLen = 0;

  static thread_local bool IsMyThread;

  bool InMergeMode = false;
};

};

#endif
