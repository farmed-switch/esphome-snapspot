

#include "FuzzerInternal.h"
#include "FuzzerIO.h"
#include "FuzzerMerge.h"
#include "FuzzerTracePC.h"
#include "FuzzerUtil.h"

#include <fstream>
#include <iterator>
#include <sstream>

namespace fuzzer {

bool Merger::Parse(const std::string &Str, bool ParseCoverage) {
  std::istringstream SS(Str);
  return Parse(SS, ParseCoverage);
}

void Merger::ParseOrExit(std::istream &IS, bool ParseCoverage) {
  if (!Parse(IS, ParseCoverage)) {
    Printf("MERGE: failed to parse the control file (unexpected error)\n");
    exit(1);
  }
}

bool Merger::Parse(std::istream &IS, bool ParseCoverage) {
  LastFailure.clear();
  std::string Line;

  if (!std::getline(IS, Line, '\n')) return false;
  std::istringstream L1(Line);
  size_t NumFiles = 0;
  L1 >> NumFiles;
  if (NumFiles == 0 || NumFiles > 10000000) return false;

  if (!std::getline(IS, Line, '\n')) return false;
  std::istringstream L2(Line);
  NumFilesInFirstCorpus = NumFiles + 1;
  L2 >> NumFilesInFirstCorpus;
  if (NumFilesInFirstCorpus > NumFiles) return false;

  Files.resize(NumFiles);
  for (size_t i = 0; i < NumFiles; i++)
    if (!std::getline(IS, Files[i].Name, '\n'))
      return false;

  size_t ExpectedStartMarker = 0;
  const size_t kInvalidStartMarker = -1;
  size_t LastSeenStartMarker = kInvalidStartMarker;
  while (std::getline(IS, Line, '\n')) {
    std::istringstream ISS1(Line);
    std::string Marker;
    size_t N;
    ISS1 >> Marker;
    ISS1 >> N;
    if (Marker == "STARTED") {

      if (ExpectedStartMarker != N)
        return false;
      ISS1 >> Files[ExpectedStartMarker].Size;
      LastSeenStartMarker = ExpectedStartMarker;
      assert(ExpectedStartMarker < Files.size());
      ExpectedStartMarker++;
    } else if (Marker == "DONE") {

      size_t CurrentFileIdx = N;
      if (CurrentFileIdx != LastSeenStartMarker)
        return false;
      LastSeenStartMarker = kInvalidStartMarker;
      if (ParseCoverage) {
        auto &V = Files[CurrentFileIdx].Features;
        V.clear();
        while (ISS1 >> std::hex >> N)
          V.push_back(N);
        std::sort(V.begin(), V.end());
      }
    } else {
      return false;
    }
  }
  if (LastSeenStartMarker != kInvalidStartMarker)
    LastFailure = Files[LastSeenStartMarker].Name;

  FirstNotProcessedFile = ExpectedStartMarker;
  return true;
}

size_t Merger::Merge(std::vector<std::string> *NewFiles) {
  NewFiles->clear();
  assert(NumFilesInFirstCorpus <= Files.size());
  std::set<uint32_t> AllFeatures;

  for (size_t i = 0; i < NumFilesInFirstCorpus; i++) {
    auto &Cur = Files[i].Features;
    AllFeatures.insert(Cur.begin(), Cur.end());
  }
  size_t InitialNumFeatures = AllFeatures.size();

  for (size_t i = NumFilesInFirstCorpus; i < Files.size(); i++) {
    auto &Cur = Files[i].Features;
    std::vector<uint32_t> Tmp;
    std::set_difference(Cur.begin(), Cur.end(), AllFeatures.begin(),
                        AllFeatures.end(), std::inserter(Tmp, Tmp.begin()));
    Cur.swap(Tmp);
  }

  std::sort(Files.begin() + NumFilesInFirstCorpus, Files.end(),
            [&](const MergeFileInfo &a, const MergeFileInfo &b) -> bool {
              if (a.Size != b.Size)
                return a.Size < b.Size;
              return a.Features.size() > b.Features.size();
            });

  for (size_t i = NumFilesInFirstCorpus; i < Files.size(); i++) {
    auto &Cur = Files[i].Features;

    size_t OldSize = AllFeatures.size();
    AllFeatures.insert(Cur.begin(), Cur.end());
    if (AllFeatures.size() > OldSize)
      NewFiles->push_back(Files[i].Name);
  }
  return AllFeatures.size() - InitialNumFeatures;
}

void Fuzzer::CrashResistantMergeInternalStep(const std::string &CFPath) {
  Printf("MERGE-INNER: using the control file '%s'\n", CFPath.c_str());
  Merger M;
  std::ifstream IF(CFPath);
  M.ParseOrExit(IF, false);
  IF.close();
  if (!M.LastFailure.empty())
    Printf("MERGE-INNER: '%s' caused a failure at the previous merge step\n",
           M.LastFailure.c_str());

  Printf("MERGE-INNER: %zd total files;"
         " %zd processed earlier; will process %zd files now\n",
         M.Files.size(), M.FirstNotProcessedFile,
         M.Files.size() - M.FirstNotProcessedFile);

  std::ofstream OF(CFPath, std::ofstream::out | std::ofstream::app);
  for (size_t i = M.FirstNotProcessedFile; i < M.Files.size(); i++) {
    auto U = FileToVector(M.Files[i].Name);
    if (U.size() > MaxInputLen) {
      U.resize(MaxInputLen);
      U.shrink_to_fit();
    }
    std::ostringstream StartedLine;

    OF << "STARTED " << std::dec << i << " " << U.size() << "\n";
    OF.flush();

    TPC.ResetMaps();
    ExecuteCallback(U.data(), U.size());

    std::set<size_t> Features;
    TPC.CollectFeatures([&](size_t Feature) -> bool {
      Features.insert(Feature);
      return true;
    });

    TotalNumberOfRuns++;
    if (!(TotalNumberOfRuns & (TotalNumberOfRuns - 1)))
      PrintStats("pulse ");

    OF << "DONE " << i;
    for (size_t F : Features)
      OF << " " << std::hex << F;
    OF << "\n";
  }
}

void Fuzzer::CrashResistantMerge(const std::vector<std::string> &Args,
                                 const std::vector<std::string> &Corpora) {
  if (Corpora.size() <= 1) {
    Printf("Merge requires two or more corpus dirs\n");
    return;
  }
  std::vector<std::string> AllFiles;
  ListFilesInDirRecursive(Corpora[0], nullptr, &AllFiles,  true);
  size_t NumFilesInFirstCorpus = AllFiles.size();
  for (size_t i = 1; i < Corpora.size(); i++)
    ListFilesInDirRecursive(Corpora[i], nullptr, &AllFiles,  true);
  Printf("MERGE-OUTER: %zd files, %zd in the initial corpus\n",
         AllFiles.size(), NumFilesInFirstCorpus);
  std::string CFPath =
      "libFuzzerTemp." + std::to_string(GetPid()) + ".txt";

  RemoveFile(CFPath);
  std::ofstream ControlFile(CFPath);
  ControlFile << AllFiles.size() << "\n";
  ControlFile << NumFilesInFirstCorpus << "\n";
  for (auto &Path: AllFiles)
    ControlFile << Path << "\n";
  ControlFile.close();

  std::string BaseCmd = CloneArgsWithoutX(Args, "keep-all-flags");
  for (size_t i = 1; i <= AllFiles.size(); i++) {
    Printf("MERGE-OUTER: attempt %zd\n", i);
    auto ExitCode =
        ExecuteCommand(BaseCmd + " -merge_control_file=" + CFPath);
    if (!ExitCode) {
      Printf("MERGE-OUTER: succesfull in %zd attempt(s)\n", i);
      break;
    }
  }

  Merger M;
  std::ifstream IF(CFPath);
  M.ParseOrExit(IF, true);
  IF.close();
  std::vector<std::string> NewFiles;
  size_t NumNewFeatures = M.Merge(&NewFiles);
  Printf("MERGE-OUTER: %zd new files with %zd new features added\n",
         NewFiles.size(), NumNewFeatures);
  for (auto &F: NewFiles)
    WriteToOutputCorpus(FileToVector(F));

  RemoveFile(CFPath);
}

}
