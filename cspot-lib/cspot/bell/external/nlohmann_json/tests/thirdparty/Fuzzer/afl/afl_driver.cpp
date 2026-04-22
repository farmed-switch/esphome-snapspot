

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>

#ifdef __linux__
#define LIBFUZZER_LINUX 1
#define LIBFUZZER_APPLE 0
#elif __APPLE__
#define LIBFUZZER_LINUX 0
#define LIBFUZZER_APPLE 1
#else
#error "Support for your platform has not been implemented"
#endif

#define CHECK_ERROR(cond, error_message)                                       \
  if (!(cond)) {                                                               \
    fprintf(stderr, (error_message));                                          \
    abort();                                                                   \
  }

extern "C" {
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);
__attribute__((weak)) int LLVMFuzzerInitialize(int *argc, char ***argv);
}

static volatile char AFL_PERSISTENT[] = "##SIG_AFL_PERSISTENT##";
extern "C" int __afl_persistent_loop(unsigned int);
static volatile char suppress_warning2 = AFL_PERSISTENT[0];

static volatile char AFL_DEFER_FORKSVR[] = "##SIG_AFL_DEFER_FORKSRV##";
extern "C" void  __afl_manual_init();
static volatile char suppress_warning1 = AFL_DEFER_FORKSVR[0];

static const size_t kMaxAflInputSize = 1 << 20;
static uint8_t AflInputBuf[kMaxAflInputSize];

static FILE *extra_stats_file = NULL;
static uint32_t previous_peak_rss = 0;
static time_t slowest_unit_time_secs = 0;
static const int kNumExtraStats = 2;
static const char *kExtraStatsFormatString = "peak_rss_mb            : %u\n"
                                             "slowest_unit_time_sec  : %u\n";

size_t GetPeakRSSMb() {
  struct rusage usage;
  if (getrusage(RUSAGE_SELF, &usage))
    return 0;
  if (LIBFUZZER_LINUX) {

    return usage.ru_maxrss >> 10;
  } else if (LIBFUZZER_APPLE) {

    return usage.ru_maxrss >> 20;
  }
  assert(0 && "GetPeakRSSMb() is not implemented for your platform");
  return 0;
}

static void SetSigaction(int signum,
                         void (*callback)(int, siginfo_t *, void *)) {
  struct sigaction sigact;
  memset(&sigact, 0, sizeof(sigact));
  sigact.sa_sigaction = callback;
  if (sigaction(signum, &sigact, 0)) {
    fprintf(stderr, "libFuzzer: sigaction failed with %d\n", errno);
    exit(1);
  }
}

static void write_extra_stats() {
  uint32_t peak_rss = GetPeakRSSMb();

  if (peak_rss < previous_peak_rss)
    peak_rss = previous_peak_rss;

  int chars_printed = fprintf(extra_stats_file, kExtraStatsFormatString,
                              peak_rss, slowest_unit_time_secs);

  CHECK_ERROR(chars_printed != 0, "Failed to write extra_stats_file");

  CHECK_ERROR(fclose(extra_stats_file) == 0,
              "Failed to close extra_stats_file");
}

static void crash_handler(int, siginfo_t *, void *) {

  static bool first_crash = true;
  CHECK_ERROR(first_crash,
              "Crashed in crash signal handler. This is a bug in the fuzzer.");

  first_crash = false;
  write_extra_stats();
}

static void maybe_initialize_extra_stats() {

  char *extra_stats_filename = getenv("AFL_DRIVER_EXTRA_STATS_FILENAME");
  if (!extra_stats_filename)
    return;

  extra_stats_file = fopen(extra_stats_filename, "r");

  if (extra_stats_file) {
    int matches = fscanf(extra_stats_file, kExtraStatsFormatString,
                         &previous_peak_rss, &slowest_unit_time_secs);

    CHECK_ERROR(matches == kNumExtraStats, "Extra stats file is corrupt");

    CHECK_ERROR(fclose(extra_stats_file) == 0, "Failed to close file");

    extra_stats_file = fopen(extra_stats_filename, "w");
    CHECK_ERROR(extra_stats_file,
                "Failed to open extra stats file for writing");
  } else {

    extra_stats_file = fopen(extra_stats_filename, "w+");
    CHECK_ERROR(extra_stats_file, "failed to create extra stats file");
  }

  int crash_signals[] = {SIGSEGV, SIGBUS, SIGABRT, SIGILL, SIGFPE,  SIGINT,
                         SIGTERM};

  const size_t num_signals = sizeof(crash_signals) / sizeof(crash_signals[0]);

  for (size_t idx = 0; idx < num_signals; idx++)
    SetSigaction(crash_signals[idx], crash_handler);

  atexit(write_extra_stats);
}

static void maybe_duplicate_stderr() {
  char* stderr_duplicate_filename =
      getenv("AFL_DRIVER_STDERR_DUPLICATE_FILENAME");

  if (!stderr_duplicate_filename)
    return;

  FILE* stderr_duplicate_stream =
      freopen(stderr_duplicate_filename, "a+", stderr);

  if (!stderr_duplicate_stream) {
    fprintf(
        stderr,
        "Failed to duplicate stderr to AFL_DRIVER_STDERR_DUPLICATE_FILENAME");
    abort();
  }
}

int main(int argc, char **argv) {
  fprintf(stderr, "======================= INFO =========================\n"
                  "This binary is built for AFL-fuzz.\n"
                  "To run the target function on a single input execute this:\n"
                  "  %s < INPUT_FILE\n"
                  "To run the fuzzing execute this:\n"
                  "  afl-fuzz [afl-flags] %s [N] "
                  "-- run N fuzzing iterations before "
                  "re-spawning the process (default: 1000)\n"
                  "======================================================\n",
          argv[0], argv[0]);
  if (LLVMFuzzerInitialize)
    LLVMFuzzerInitialize(&argc, &argv);

  maybe_duplicate_stderr();
  maybe_initialize_extra_stats();

  __afl_manual_init();

  int N = 1000;
  if (argc >= 2)
    N = atoi(argv[1]);
  assert(N > 0);
  time_t unit_time_secs;
  int num_runs = 0;
  while (__afl_persistent_loop(N)) {
    ssize_t n_read = read(0, AflInputBuf, kMaxAflInputSize);
    if (n_read > 0) {

      uint8_t *copy = new uint8_t[n_read];
      memcpy(copy, AflInputBuf, n_read);

      struct timeval unit_start_time;
      CHECK_ERROR(gettimeofday(&unit_start_time, NULL) == 0,
                  "Calling gettimeofday failed");

      num_runs++;
      LLVMFuzzerTestOneInput(copy, n_read);

      struct timeval unit_stop_time;
      CHECK_ERROR(gettimeofday(&unit_stop_time, NULL) == 0,
                  "Calling gettimeofday failed");

      unit_time_secs = unit_stop_time.tv_sec - unit_start_time.tv_sec;
      if (slowest_unit_time_secs < unit_time_secs)
        slowest_unit_time_secs = unit_time_secs;

      delete[] copy;
    }
  }
  fprintf(stderr, "%s: successfully executed %d input(s)\n", argv[0], num_runs);
}
