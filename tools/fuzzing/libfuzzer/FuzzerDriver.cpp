//===- FuzzerDriver.cpp - FuzzerDriver function and flags -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// FuzzerDriver and flag parsing.
//===----------------------------------------------------------------------===//

#include "FuzzerCommand.h"
#include "FuzzerCorpus.h"
#include "FuzzerFork.h"
#include "FuzzerIO.h"
#include "FuzzerInterface.h"
#include "FuzzerInternal.h"
#include "FuzzerMerge.h"
#include "FuzzerMutate.h"
#include "FuzzerPlatform.h"
#include "FuzzerRandom.h"
#include "FuzzerTracePC.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <fstream>

// This function should be present in the libFuzzer so that the client
// binary can test for its existence.
#if LIBFUZZER_MSVC
extern "C" void __libfuzzer_is_present() {}
#if defined(_M_IX86) || defined(__i386__)
#pragma comment(linker, "/include:___libfuzzer_is_present")
#else
#pragma comment(linker, "/include:__libfuzzer_is_present")
#endif
#else
extern "C" __attribute__((used)) void __libfuzzer_is_present() {}
#endif  // LIBFUZZER_MSVC

namespace fuzzer {

// Program arguments.
struct FlagDescription {
  const char *Name;
  const char *Description;
  int   Default;
  int   *IntFlag;
  const char **StrFlag;
  unsigned int *UIntFlag;
};

struct {
#define FUZZER_DEPRECATED_FLAG(Name)
#define FUZZER_FLAG_INT(Name, Default, Description) int Name;
#define FUZZER_FLAG_UNSIGNED(Name, Default, Description) unsigned int Name;
#define FUZZER_FLAG_STRING(Name, Description) const char *Name;
#include "FuzzerFlags.def"
#undef FUZZER_DEPRECATED_FLAG
#undef FUZZER_FLAG_INT
#undef FUZZER_FLAG_UNSIGNED
#undef FUZZER_FLAG_STRING
} Flags;

static const FlagDescription FlagDescriptions [] {
#define FUZZER_DEPRECATED_FLAG(Name)                                           \
  {#Name, "Deprecated; don't use", 0, nullptr, nullptr, nullptr},
#define FUZZER_FLAG_INT(Name, Default, Description)                            \
  {#Name, Description, Default, &Flags.Name, nullptr, nullptr},
#define FUZZER_FLAG_UNSIGNED(Name, Default, Description)                       \
  {#Name,   Description, static_cast<int>(Default),                            \
   nullptr, nullptr, &Flags.Name},
#define FUZZER_FLAG_STRING(Name, Description)                                  \
  {#Name, Description, 0, nullptr, &Flags.Name, nullptr},
#include "FuzzerFlags.def"
#undef FUZZER_DEPRECATED_FLAG
#undef FUZZER_FLAG_INT
#undef FUZZER_FLAG_UNSIGNED
#undef FUZZER_FLAG_STRING
};

static const size_t kNumFlags =
    sizeof(FlagDescriptions) / sizeof(FlagDescriptions[0]);

static Vector<std::string> *Inputs;
static std::string *ProgName;

static void PrintHelp() {
  Printf("Usage:\n");
  auto Prog = ProgName->c_str();
  Printf("\nTo run fuzzing pass 0 or more directories.\n");
  Printf("%s [-flag1=val1 [-flag2=val2 ...] ] [dir1 [dir2 ...] ]\n", Prog);

  Printf("\nTo run individual tests without fuzzing pass 1 or more files:\n");
  Printf("%s [-flag1=val1 [-flag2=val2 ...] ] file1 [file2 ...]\n", Prog);

  Printf("\nFlags: (strictly in form -flag=value)\n");
  size_t MaxFlagLen = 0;
  for (size_t F = 0; F < kNumFlags; F++)
    MaxFlagLen = std::max(strlen(FlagDescriptions[F].Name), MaxFlagLen);

  for (size_t F = 0; F < kNumFlags; F++) {
    const auto &D = FlagDescriptions[F];
    if (strstr(D.Description, "internal flag") == D.Description) continue;
    Printf(" %s", D.Name);
    for (size_t i = 0, n = MaxFlagLen - strlen(D.Name); i < n; i++)
      Printf(" ");
    Printf("\t");
    Printf("%d\t%s\n", D.Default, D.Description);
  }
  Printf("\nFlags starting with '--' will be ignored and "
            "will be passed verbatim to subprocesses.\n");
}

static const char *FlagValue(const char *Param, const char *Name) {
  size_t Len = strlen(Name);
  if (Param[0] == '-' && strstr(Param + 1, Name) == Param + 1 &&
      Param[Len + 1] == '=')
      return &Param[Len + 2];
  return nullptr;
}

// Avoid calling stol as it triggers a bug in clang/glibc build.
static long MyStol(const char *Str) {
  long Res = 0;
  long Sign = 1;
  if (*Str == '-') {
    Str++;
    Sign = -1;
  }
  for (size_t i = 0; Str[i]; i++) {
    char Ch = Str[i];
    if (Ch < '0' || Ch > '9')
      return Res;
    Res = Res * 10 + (Ch - '0');
  }
  return Res * Sign;
}

static bool ParseOneFlag(const char *Param) {
  if (Param[0] != '-') return false;
  if (Param[1] == '-') {
    static bool PrintedWarning = false;
    if (!PrintedWarning) {
      PrintedWarning = true;
      Printf("INFO: libFuzzer ignores flags that start with '--'\n");
    }
    for (size_t F = 0; F < kNumFlags; F++)
      if (FlagValue(Param + 1, FlagDescriptions[F].Name))
        Printf("WARNING: did you mean '%s' (single dash)?\n", Param + 1);
    return true;
  }
  for (size_t F = 0; F < kNumFlags; F++) {
    const char *Name = FlagDescriptions[F].Name;
    const char *Str = FlagValue(Param, Name);
    if (Str)  {
      if (FlagDescriptions[F].IntFlag) {
        int Val = MyStol(Str);
        *FlagDescriptions[F].IntFlag = Val;
        if (Flags.verbosity >= 2)
          Printf("Flag: %s %d\n", Name, Val);
        return true;
      } else if (FlagDescriptions[F].UIntFlag) {
        unsigned int Val = std::stoul(Str);
        *FlagDescriptions[F].UIntFlag = Val;
        if (Flags.verbosity >= 2)
          Printf("Flag: %s %u\n", Name, Val);
        return true;
      } else if (FlagDescriptions[F].StrFlag) {
        *FlagDescriptions[F].StrFlag = Str;
        if (Flags.verbosity >= 2)
          Printf("Flag: %s %s\n", Name, Str);
        return true;
      } else {  // Deprecated flag.
        Printf("Flag: %s: deprecated, don't use\n", Name);
        return true;
      }
    }
  }
  Printf("\n\nWARNING: unrecognized flag '%s'; "
         "use -help=1 to list all flags\n\n", Param);
  return true;
}

// We don't use any library to minimize dependencies.
static void ParseFlags(const Vector<std::string> &Args,
                       const ExternalFunctions *EF) {
  for (size_t F = 0; F < kNumFlags; F++) {
    if (FlagDescriptions[F].IntFlag)
      *FlagDescriptions[F].IntFlag = FlagDescriptions[F].Default;
    if (FlagDescriptions[F].UIntFlag)
      *FlagDescriptions[F].UIntFlag =
          static_cast<unsigned int>(FlagDescriptions[F].Default);
    if (FlagDescriptions[F].StrFlag)
      *FlagDescriptions[F].StrFlag = nullptr;
  }

  // Disable len_control by default, if LLVMFuzzerCustomMutator is used.
  if (EF->LLVMFuzzerCustomMutator) {
    Flags.len_control = 0;
    Printf("INFO: found LLVMFuzzerCustomMutator (%p). "
           "Disabling -len_control by default.\n", EF->LLVMFuzzerCustomMutator);
  }

  Inputs = new Vector<std::string>;
  for (size_t A = 1; A < Args.size(); A++) {
    if (ParseOneFlag(Args[A].c_str())) {
      if (Flags.ignore_remaining_args)
        break;
      continue;
    }
    Inputs->push_back(Args[A]);
  }
}

static std::mutex Mu;

static void PulseThread() {
  while (true) {
    SleepSeconds(600);
    std::lock_guard<std::mutex> Lock(Mu);
    Printf("pulse...\n");
  }
}

static void WorkerThread(const Command &BaseCmd, std::atomic<unsigned> *Counter,
                         unsigned NumJobs, std::atomic<bool> *HasErrors) {
  while (true) {
    unsigned C = (*Counter)++;
    if (C >= NumJobs) break;
    std::string Log = "fuzz-" + std::to_string(C) + ".log";
    Command Cmd(BaseCmd);
    Cmd.setOutputFile(Log);
    Cmd.combineOutAndErr();
    if (Flags.verbosity) {
      std::string CommandLine = Cmd.toString();
      Printf("%s\n", CommandLine.c_str());
    }
    int ExitCode = ExecuteCommand(Cmd);
    if (ExitCode != 0)
      *HasErrors = true;
    std::lock_guard<std::mutex> Lock(Mu);
    Printf("================== Job %u exited with exit code %d ============\n",
           C, ExitCode);
    fuzzer::CopyFileToErr(Log);
  }
}

std::string CloneArgsWithoutX(const Vector<std::string> &Args,
                              const char *X1, const char *X2) {
  std::string Cmd;
  for (auto &S : Args) {
    if (FlagValue(S.c_str(), X1) || FlagValue(S.c_str(), X2))
      continue;
    Cmd += S + " ";
  }
  return Cmd;
}

static int RunInMultipleProcesses(const Vector<std::string> &Args,
                                  unsigned NumWorkers, unsigned NumJobs) {
  std::atomic<unsigned> Counter(0);
  std::atomic<bool> HasErrors(false);
  Command Cmd(Args);
  Cmd.removeFlag("jobs");
  Cmd.removeFlag("workers");
  Vector<std::thread> V;
  std::thread Pulse(PulseThread);
  Pulse.detach();
  for (unsigned i = 0; i < NumWorkers; i++)
    V.push_back(std::thread(WorkerThread, std::ref(Cmd), &Counter, NumJobs, &HasErrors));
  for (auto &T : V)
    T.join();
  return HasErrors ? 1 : 0;
}

static void RssThread(Fuzzer *F, size_t RssLimitMb) {
  while (true) {
    SleepSeconds(1);
    size_t Peak = GetPeakRSSMb();
    if (Peak > RssLimitMb)
      F->RssLimitCallback();
  }
}

static void StartRssThread(Fuzzer *F, size_t RssLimitMb) {
  if (!RssLimitMb)
    return;
  std::thread T(RssThread, F, RssLimitMb);
  T.detach();
}

int RunOneTest(Fuzzer *F, const char *InputFilePath, size_t MaxLen) {
  Unit U = FileToVector(InputFilePath);
  if (MaxLen && MaxLen < U.size())
    U.resize(MaxLen);
  F->ExecuteCallback(U.data(), U.size());
  F->TryDetectingAMemoryLeak(U.data(), U.size(), true);
  return 0;
}

static bool AllInputsAreFiles() {
  if (Inputs->empty()) return false;
  for (auto &Path : *Inputs)
    if (!IsFile(Path))
      return false;
  return true;
}

static std::string GetDedupTokenFromCmdOutput(const std::string &S) {
  auto Beg = S.find("DEDUP_TOKEN:");
  if (Beg == std::string::npos)
    return "";
  auto End = S.find('\n', Beg);
  if (End == std::string::npos)
    return "";
  return S.substr(Beg, End - Beg);
}

int CleanseCrashInput(const Vector<std::string> &Args,
                       const FuzzingOptions &Options) {
  if (Inputs->size() != 1 || !Flags.exact_artifact_path) {
    Printf("ERROR: -cleanse_crash should be given one input file and"
          " -exact_artifact_path\n");
    return 1;
  }
  std::string InputFilePath = Inputs->at(0);
  std::string OutputFilePath = Flags.exact_artifact_path;
  Command Cmd(Args);
  Cmd.removeFlag("cleanse_crash");

  assert(Cmd.hasArgument(InputFilePath));
  Cmd.removeArgument(InputFilePath);

  auto TmpFilePath = TempPath("CleanseCrashInput", ".repro");
  Cmd.addArgument(TmpFilePath);
  Cmd.setOutputFile(getDevNull());
  Cmd.combineOutAndErr();

  std::string CurrentFilePath = InputFilePath;
  auto U = FileToVector(CurrentFilePath);
  size_t Size = U.size();

  const Vector<uint8_t> ReplacementBytes = {' ', 0xff};
  for (int NumAttempts = 0; NumAttempts < 5; NumAttempts++) {
    bool Changed = false;
    for (size_t Idx = 0; Idx < Size; Idx++) {
      Printf("CLEANSE[%d]: Trying to replace byte %zd of %zd\n", NumAttempts,
             Idx, Size);
      uint8_t OriginalByte = U[Idx];
      if (ReplacementBytes.end() != std::find(ReplacementBytes.begin(),
                                              ReplacementBytes.end(),
                                              OriginalByte))
        continue;
      for (auto NewByte : ReplacementBytes) {
        U[Idx] = NewByte;
        WriteToFile(U, TmpFilePath);
        auto ExitCode = ExecuteCommand(Cmd);
        RemoveFile(TmpFilePath);
        if (!ExitCode) {
          U[Idx] = OriginalByte;
        } else {
          Changed = true;
          Printf("CLEANSE: Replaced byte %zd with 0x%x\n", Idx, NewByte);
          WriteToFile(U, OutputFilePath);
          break;
        }
      }
    }
    if (!Changed) break;
  }
  return 0;
}

int MinimizeCrashInput(const Vector<std::string> &Args,
                       const FuzzingOptions &Options) {
  if (Inputs->size() != 1) {
    Printf("ERROR: -minimize_crash should be given one input file\n");
    return 1;
  }
  std::string InputFilePath = Inputs->at(0);
  Command BaseCmd(Args);
  BaseCmd.removeFlag("minimize_crash");
  BaseCmd.removeFlag("exact_artifact_path");
  assert(BaseCmd.hasArgument(InputFilePath));
  BaseCmd.removeArgument(InputFilePath);
  if (Flags.runs <= 0 && Flags.max_total_time == 0) {
    Printf("INFO: you need to specify -runs=N or "
           "-max_total_time=N with -minimize_crash=1\n"
           "INFO: defaulting to -max_total_time=600\n");
    BaseCmd.addFlag("max_total_time", "600");
  }

  BaseCmd.combineOutAndErr();

  std::string CurrentFilePath = InputFilePath;
  while (true) {
    Unit U = FileToVector(CurrentFilePath);
    Printf("CRASH_MIN: minimizing crash input: '%s' (%zd bytes)\n",
           CurrentFilePath.c_str(), U.size());

    Command Cmd(BaseCmd);
    Cmd.addArgument(CurrentFilePath);

    Printf("CRASH_MIN: executing: %s\n", Cmd.toString().c_str());
    std::string CmdOutput;
    bool Success = ExecuteCommand(Cmd, &CmdOutput);
    if (Success) {
      Printf("ERROR: the input %s did not crash\n", CurrentFilePath.c_str());
      return 1;
    }
    Printf("CRASH_MIN: '%s' (%zd bytes) caused a crash. Will try to minimize "
           "it further\n",
           CurrentFilePath.c_str(), U.size());
    auto DedupToken1 = GetDedupTokenFromCmdOutput(CmdOutput);
    if (!DedupToken1.empty())
      Printf("CRASH_MIN: DedupToken1: %s\n", DedupToken1.c_str());

    std::string ArtifactPath =
        Flags.exact_artifact_path
            ? Flags.exact_artifact_path
            : Options.ArtifactPrefix + "minimized-from-" + Hash(U);
    Cmd.addFlag("minimize_crash_internal_step", "1");
    Cmd.addFlag("exact_artifact_path", ArtifactPath);
    Printf("CRASH_MIN: executing: %s\n", Cmd.toString().c_str());
    CmdOutput.clear();
    Success = ExecuteCommand(Cmd, &CmdOutput);
    Printf("%s", CmdOutput.c_str());
    if (Success) {
      if (Flags.exact_artifact_path) {
        CurrentFilePath = Flags.exact_artifact_path;
        WriteToFile(U, CurrentFilePath);
      }
      Printf("CRASH_MIN: failed to minimize beyond %s (%d bytes), exiting\n",
             CurrentFilePath.c_str(), U.size());
      break;
    }
    auto DedupToken2 = GetDedupTokenFromCmdOutput(CmdOutput);
    if (!DedupToken2.empty())
      Printf("CRASH_MIN: DedupToken2: %s\n", DedupToken2.c_str());

    if (DedupToken1 != DedupToken2) {
      if (Flags.exact_artifact_path) {
        CurrentFilePath = Flags.exact_artifact_path;
        WriteToFile(U, CurrentFilePath);
      }
      Printf("CRASH_MIN: mismatch in dedup tokens"
             " (looks like a different bug). Won't minimize further\n");
      break;
    }

    CurrentFilePath = ArtifactPath;
    Printf("*********************************\n");
  }
  return 0;
}

int MinimizeCrashInputInternalStep(Fuzzer *F, InputCorpus *Corpus) {
  assert(Inputs->size() == 1);
  std::string InputFilePath = Inputs->at(0);
  Unit U = FileToVector(InputFilePath);
  Printf("INFO: Starting MinimizeCrashInputInternalStep: %zd\n", U.size());
  if (U.size() < 2) {
    Printf("INFO: The input is small enough, exiting\n");
    return 0;
  }
  F->SetMaxInputLen(U.size());
  F->SetMaxMutationLen(U.size() - 1);
  F->MinimizeCrashLoop(U);
  Printf("INFO: Done MinimizeCrashInputInternalStep, no crashes found\n");
  return 0;
}

int Merge(Fuzzer *F, FuzzingOptions &Options, const Vector<std::string> &Args,
           const Vector<std::string> &Corpora, const char *CFPathOrNull) {
  if (Corpora.size() < 2) {
    Printf("INFO: Merge requires two or more corpus dirs\n");
    return 0;
  }

  Vector<SizedFile> OldCorpus, NewCorpus;
  int Res = GetSizedFilesFromDir(Corpora[0], &OldCorpus);
  if (Res != 0)
    return Res;
  for (size_t i = 1; i < Corpora.size(); i++) {
    Res = GetSizedFilesFromDir(Corpora[i], &NewCorpus);
    if (Res != 0)
      return Res;
  }
  std::sort(OldCorpus.begin(), OldCorpus.end());
  std::sort(NewCorpus.begin(), NewCorpus.end());

  std::string CFPath = CFPathOrNull ? CFPathOrNull : TempPath("Merge", ".txt");
  Vector<std::string> NewFiles;
  Set<uint32_t> NewFeatures, NewCov;
  Res = CrashResistantMerge(Args, OldCorpus, NewCorpus, &NewFiles, {}, &NewFeatures,
                      {}, &NewCov, CFPath, true);
  if (Res != 0)
    return Res;

  if (F->isGracefulExitRequested())
    return 0;
  for (auto &Path : NewFiles)
    F->WriteToOutputCorpus(FileToVector(Path, Options.MaxLen));
  // We are done, delete the control file if it was a temporary one.
  if (!Flags.merge_control_file)
    RemoveFile(CFPath);

  return 0;
}

int AnalyzeDictionary(Fuzzer *F, const Vector<Unit>& Dict,
                      UnitVector& Corpus) {
  Printf("Started dictionary minimization (up to %d tests)\n",
         Dict.size() * Corpus.size() * 2);

  // Scores and usage count for each dictionary unit.
  Vector<int> Scores(Dict.size());
  Vector<int> Usages(Dict.size());

  Vector<size_t> InitialFeatures;
  Vector<size_t> ModifiedFeatures;
  for (auto &C : Corpus) {
    // Get coverage for the testcase without modifications.
    F->ExecuteCallback(C.data(), C.size());
    InitialFeatures.clear();
    TPC.CollectFeatures([&](size_t Feature) {
      InitialFeatures.push_back(Feature);
    });

    for (size_t i = 0; i < Dict.size(); ++i) {
      Vector<uint8_t> Data = C;
      auto StartPos = std::search(Data.begin(), Data.end(),
                                  Dict[i].begin(), Dict[i].end());
      // Skip dictionary unit, if the testcase does not contain it.
      if (StartPos == Data.end())
        continue;

      ++Usages[i];
      while (StartPos != Data.end()) {
        // Replace all occurrences of dictionary unit in the testcase.
        auto EndPos = StartPos + Dict[i].size();
        for (auto It = StartPos; It != EndPos; ++It)
          *It ^= 0xFF;

        StartPos = std::search(EndPos, Data.end(),
                               Dict[i].begin(), Dict[i].end());
      }

      // Get coverage for testcase with masked occurrences of dictionary unit.
      F->ExecuteCallback(Data.data(), Data.size());
      ModifiedFeatures.clear();
      TPC.CollectFeatures([&](size_t Feature) {
        ModifiedFeatures.push_back(Feature);
      });

      if (InitialFeatures == ModifiedFeatures)
        --Scores[i];
      else
        Scores[i] += 2;
    }
  }

  Printf("###### Useless dictionary elements. ######\n");
  for (size_t i = 0; i < Dict.size(); ++i) {
    // Dictionary units with positive score are treated as useful ones.
    if (Scores[i] > 0)
       continue;

    Printf("\"");
    PrintASCII(Dict[i].data(), Dict[i].size(), "\"");
    Printf(" # Score: %d, Used: %d\n", Scores[i], Usages[i]);
  }
  Printf("###### End of useless dictionary elements. ######\n");
  return 0;
}

int ParseSeedInuts(const char *seed_inputs, Vector<std::string> &Files) {
  // Parse -seed_inputs=file1,file2,... or -seed_inputs=@seed_inputs_file
  if (!seed_inputs) return 0;
  std::string SeedInputs;
  if (Flags.seed_inputs[0] == '@')
    SeedInputs = FileToString(Flags.seed_inputs + 1); // File contains list.
  else
    SeedInputs = Flags.seed_inputs; // seed_inputs contains the list.
  if (SeedInputs.empty()) {
    Printf("seed_inputs is empty or @file does not exist.\n");
    return 1;
  }
  // Parse SeedInputs.
  size_t comma_pos = 0;
  while ((comma_pos = SeedInputs.find_last_of(',')) != std::string::npos) {
    Files.push_back(SeedInputs.substr(comma_pos + 1));
    SeedInputs = SeedInputs.substr(0, comma_pos);
  }
  Files.push_back(SeedInputs);
  return 0;
}

static Vector<SizedFile> ReadCorpora(const Vector<std::string> &CorpusDirs,
    const Vector<std::string> &ExtraSeedFiles) {
  Vector<SizedFile> SizedFiles;
  size_t LastNumFiles = 0;
  for (auto &Dir : CorpusDirs) {
    GetSizedFilesFromDir(Dir, &SizedFiles);
    Printf("INFO: % 8zd files found in %s\n", SizedFiles.size() - LastNumFiles,
           Dir.c_str());
    LastNumFiles = SizedFiles.size();
  }
  for (auto &File : ExtraSeedFiles)
    if (auto Size = FileSize(File))
      SizedFiles.push_back({File, Size});
  return SizedFiles;
}

int FuzzerDriver(int *argc, char ***argv, UserCallback Callback) {
  using namespace fuzzer;
  assert(argc && argv && "Argument pointers cannot be nullptr");
  std::string Argv0((*argv)[0]);
  if (!EF)
    EF = new ExternalFunctions();
  if (EF->LLVMFuzzerInitialize)
    EF->LLVMFuzzerInitialize(argc, argv);
  if (EF->__msan_scoped_disable_interceptor_checks)
    EF->__msan_scoped_disable_interceptor_checks();
  const Vector<std::string> Args(*argv, *argv + *argc);
  assert(!Args.empty());
  ProgName = new std::string(Args[0]);
  if (Argv0 != *ProgName) {
    Printf("ERROR: argv[0] has been modified in LLVMFuzzerInitialize\n");
    return 1;
  }
  ParseFlags(Args, EF);
  if (Flags.help) {
    PrintHelp();
    return 0;
  }

  if (Flags.close_fd_mask & 2)
    DupAndCloseStderr();
  if (Flags.close_fd_mask & 1)
    CloseStdout();

  if (Flags.jobs > 0 && Flags.workers == 0) {
    Flags.workers = std::min(NumberOfCpuCores() / 2, Flags.jobs);
    if (Flags.workers > 1)
      Printf("Running %u workers\n", Flags.workers);
  }

  if (Flags.workers > 0 && Flags.jobs > 0)
    return RunInMultipleProcesses(Args, Flags.workers, Flags.jobs);

  FuzzingOptions Options;
  Options.Verbosity = Flags.verbosity;
  Options.MaxLen = Flags.max_len;
  Options.LenControl = Flags.len_control;
  Options.UnitTimeoutSec = Flags.timeout;
  Options.ErrorExitCode = Flags.error_exitcode;
  Options.TimeoutExitCode = Flags.timeout_exitcode;
  Options.IgnoreTimeouts = Flags.ignore_timeouts;
  Options.IgnoreOOMs = Flags.ignore_ooms;
  Options.IgnoreCrashes = Flags.ignore_crashes;
  Options.MaxTotalTimeSec = Flags.max_total_time;
  Options.DoCrossOver = Flags.cross_over;
  Options.MutateDepth = Flags.mutate_depth;
  Options.ReduceDepth = Flags.reduce_depth;
  Options.UseCounters = Flags.use_counters;
  Options.UseMemmem = Flags.use_memmem;
  Options.UseCmp = Flags.use_cmp;
  Options.UseValueProfile = Flags.use_value_profile;
  Options.Shrink = Flags.shrink;
  Options.ReduceInputs = Flags.reduce_inputs;
  Options.ShuffleAtStartUp = Flags.shuffle;
  Options.PreferSmall = Flags.prefer_small;
  Options.ReloadIntervalSec = Flags.reload;
  Options.OnlyASCII = Flags.only_ascii;
  Options.DetectLeaks = Flags.detect_leaks;
  Options.PurgeAllocatorIntervalSec = Flags.purge_allocator_interval;
  Options.TraceMalloc = Flags.trace_malloc;
  Options.RssLimitMb = Flags.rss_limit_mb;
  Options.MallocLimitMb = Flags.malloc_limit_mb;
  if (!Options.MallocLimitMb)
    Options.MallocLimitMb = Options.RssLimitMb;
  if (Flags.runs >= 0)
    Options.MaxNumberOfRuns = Flags.runs;
  if (!Inputs->empty() && !Flags.minimize_crash_internal_step)
    Options.OutputCorpus = (*Inputs)[0];
  Options.ReportSlowUnits = Flags.report_slow_units;
  if (Flags.artifact_prefix)
    Options.ArtifactPrefix = Flags.artifact_prefix;
  if (Flags.exact_artifact_path)
    Options.ExactArtifactPath = Flags.exact_artifact_path;
  Vector<Unit> Dictionary;
  if (Flags.dict)
    if (!ParseDictionaryFile(FileToString(Flags.dict), &Dictionary))
      return 1;
  if (Flags.verbosity > 0 && !Dictionary.empty())
    Printf("Dictionary: %zd entries\n", Dictionary.size());
  bool RunIndividualFiles = AllInputsAreFiles();
  Options.SaveArtifacts =
      !RunIndividualFiles || Flags.minimize_crash_internal_step;
  Options.PrintNewCovPcs = Flags.print_pcs;
  Options.PrintNewCovFuncs = Flags.print_funcs;
  Options.PrintFinalStats = Flags.print_final_stats;
  Options.PrintCorpusStats = Flags.print_corpus_stats;
  Options.PrintCoverage = Flags.print_coverage;
  if (Flags.exit_on_src_pos)
    Options.ExitOnSrcPos = Flags.exit_on_src_pos;
  if (Flags.exit_on_item)
    Options.ExitOnItem = Flags.exit_on_item;
  if (Flags.focus_function)
    Options.FocusFunction = Flags.focus_function;
  if (Flags.data_flow_trace)
    Options.DataFlowTrace = Flags.data_flow_trace;
  if (Flags.features_dir)
    Options.FeaturesDir = Flags.features_dir;
  if (Flags.collect_data_flow)
    Options.CollectDataFlow = Flags.collect_data_flow;
  if (Flags.stop_file)
    Options.StopFile = Flags.stop_file;
  Options.Entropic = Flags.entropic;
  Options.EntropicFeatureFrequencyThreshold =
      (size_t)Flags.entropic_feature_frequency_threshold;
  Options.EntropicNumberOfRarestFeatures =
      (size_t)Flags.entropic_number_of_rarest_features;
  if (Options.Entropic) {
    if (!Options.FocusFunction.empty()) {
      Printf("ERROR: The parameters `--entropic` and `--focus_function` cannot "
             "be used together.\n");
      return 1;
    }
    Printf("INFO: Running with entropic power schedule (0x%X, %d).\n",
           Options.EntropicFeatureFrequencyThreshold,
           Options.EntropicNumberOfRarestFeatures);
  }
  struct EntropicOptions Entropic;
  Entropic.Enabled = Options.Entropic;
  Entropic.FeatureFrequencyThreshold =
      Options.EntropicFeatureFrequencyThreshold;
  Entropic.NumberOfRarestFeatures = Options.EntropicNumberOfRarestFeatures;

  unsigned Seed = Flags.seed;
  // Initialize Seed.
  if (Seed == 0)
    Seed =
        std::chrono::system_clock::now().time_since_epoch().count() + GetPid();
  if (Flags.verbosity)
    Printf("INFO: Seed: %u\n", Seed);

  if (Flags.collect_data_flow && !Flags.fork && !Flags.merge) {
    if (RunIndividualFiles)
      return CollectDataFlow(Flags.collect_data_flow, Flags.data_flow_trace,
                        ReadCorpora({}, *Inputs));
    else
      return CollectDataFlow(Flags.collect_data_flow, Flags.data_flow_trace,
                        ReadCorpora(*Inputs, {}));
  }

  Random Rand(Seed);
  auto *MD = new MutationDispatcher(Rand, Options);
  auto *Corpus = new InputCorpus(Options.OutputCorpus, Entropic);
  auto *F = new Fuzzer(Callback, *Corpus, *MD, Options);

  for (auto &U: Dictionary)
    if (U.size() <= Word::GetMaxSize())
      MD->AddWordToManualDictionary(Word(U.data(), U.size()));

      // Threads are only supported by Chrome. Don't use them with emscripten
      // for now.
#if !LIBFUZZER_EMSCRIPTEN
  StartRssThread(F, Flags.rss_limit_mb);
#endif // LIBFUZZER_EMSCRIPTEN

  Options.HandleAbrt = Flags.handle_abrt;
  Options.HandleBus = Flags.handle_bus;
  Options.HandleFpe = Flags.handle_fpe;
  Options.HandleIll = Flags.handle_ill;
  Options.HandleInt = Flags.handle_int;
  Options.HandleSegv = Flags.handle_segv;
  Options.HandleTerm = Flags.handle_term;
  Options.HandleXfsz = Flags.handle_xfsz;
  Options.HandleUsr1 = Flags.handle_usr1;
  Options.HandleUsr2 = Flags.handle_usr2;
  SetSignalHandler(Options);

  std::atexit(Fuzzer::StaticExitCallback);

  if (Flags.minimize_crash)
    return MinimizeCrashInput(Args, Options);

  if (Flags.minimize_crash_internal_step)
    return MinimizeCrashInputInternalStep(F, Corpus);

  if (Flags.cleanse_crash)
    return CleanseCrashInput(Args, Options);

  if (RunIndividualFiles) {
    Options.SaveArtifacts = false;
    int Runs = std::max(1, Flags.runs);
    Printf("%s: Running %zd inputs %d time(s) each.\n", ProgName->c_str(),
           Inputs->size(), Runs);
    for (auto &Path : *Inputs) {
      auto StartTime = system_clock::now();
      Printf("Running: %s\n", Path.c_str());
      for (int Iter = 0; Iter < Runs; Iter++)
        RunOneTest(F, Path.c_str(), Options.MaxLen);
      auto StopTime = system_clock::now();
      auto MS = duration_cast<milliseconds>(StopTime - StartTime).count();
      Printf("Executed %s in %zd ms\n", Path.c_str(), (long)MS);
    }
    Printf("***\n"
           "*** NOTE: fuzzing was not performed, you have only\n"
           "***       executed the target code on a fixed set of inputs.\n"
           "***\n");
    F->PrintFinalStats();
    return 0;
  }

  if (Flags.fork)
    return FuzzWithFork(F->GetMD().GetRand(), Options, Args, *Inputs, Flags.fork);

  if (Flags.merge)
    return Merge(F, Options, Args, *Inputs, Flags.merge_control_file);

  if (Flags.merge_inner) {
    const size_t kDefaultMaxMergeLen = 1 << 20;
    if (Options.MaxLen == 0)
      F->SetMaxInputLen(kDefaultMaxMergeLen);
    assert(Flags.merge_control_file);
    return F->CrashResistantMergeInternalStep(Flags.merge_control_file);
  }

  if (Flags.analyze_dict) {
    size_t MaxLen = INT_MAX;  // Large max length.
    UnitVector InitialCorpus;
    for (auto &Inp : *Inputs) {
      Printf("Loading corpus dir: %s\n", Inp.c_str());
      ReadDirToVectorOfUnits(Inp.c_str(), &InitialCorpus, nullptr,
                             MaxLen, /*ExitOnError=*/false);
    }

    if (Dictionary.empty() || Inputs->empty()) {
      Printf("ERROR: can't analyze dict without dict and corpus provided\n");
      return 1;
    }
    if (AnalyzeDictionary(F, Dictionary, InitialCorpus)) {
      Printf("Dictionary analysis failed\n");
      return 1;
    }
    Printf("Dictionary analysis succeeded\n");
    return 0;
  }

  {
    Vector<std::string> Files;
    int Res = ParseSeedInuts(Flags.seed_inputs, Files);
    if (Res != 0)
      return Res;
    auto CorporaFiles = ReadCorpora(*Inputs, Files);
    Res = F->Loop(CorporaFiles);
    if (Res != 0)
      return Res;
    if (F->isGracefulExitRequested())
      return 0;
  }

  if (Flags.verbosity)
    Printf("Done %zd runs in %zd second(s)\n", F->getTotalNumberOfRuns(),
           F->secondsSinceProcessStartUp());
  F->PrintFinalStats();

  return 0;  // Don't let F destroy itself.
}

extern "C" ATTRIBUTE_INTERFACE int
LLVMFuzzerRunDriver(int *argc, char ***argv,
                    int (*UserCb)(const uint8_t *Data, size_t Size)) {
  return FuzzerDriver(argc, argv, UserCb);
}

// Storage for global ExternalFunctions object.
ExternalFunctions *EF = nullptr;

}  // namespace fuzzer