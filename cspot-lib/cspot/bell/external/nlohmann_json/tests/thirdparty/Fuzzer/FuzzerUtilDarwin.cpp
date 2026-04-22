

#include "FuzzerDefs.h"
#if LIBFUZZER_APPLE

#include "FuzzerIO.h"
#include <mutex>
#include <signal.h>
#include <spawn.h>
#include <sys/wait.h>

extern "C" char **environ;

namespace fuzzer {

static std::mutex SignalMutex;

static int ActiveThreadCount = 0;
static struct sigaction OldSigIntAction;
static struct sigaction OldSigQuitAction;
static sigset_t OldBlockedSignalsSet;

int ExecuteCommand(const std::string &Command) {
  posix_spawnattr_t SpawnAttributes;
  if (posix_spawnattr_init(&SpawnAttributes))
    return -1;

  {
    std::lock_guard<std::mutex> Lock(SignalMutex);
    if (ActiveThreadCount == 0) {
      static struct sigaction IgnoreSignalAction;
      sigset_t BlockedSignalsSet;
      memset(&IgnoreSignalAction, 0, sizeof(IgnoreSignalAction));
      IgnoreSignalAction.sa_handler = SIG_IGN;

      if (sigaction(SIGINT, &IgnoreSignalAction, &OldSigIntAction) == -1) {
        Printf("Failed to ignore SIGINT\n");
        (void)posix_spawnattr_destroy(&SpawnAttributes);
        return -1;
      }
      if (sigaction(SIGQUIT, &IgnoreSignalAction, &OldSigQuitAction) == -1) {
        Printf("Failed to ignore SIGQUIT\n");

        (void)sigaction(SIGINT, &OldSigIntAction, NULL);
        (void)posix_spawnattr_destroy(&SpawnAttributes);
        return -1;
      }

      (void)sigemptyset(&BlockedSignalsSet);
      (void)sigaddset(&BlockedSignalsSet, SIGCHLD);
      if (sigprocmask(SIG_BLOCK, &BlockedSignalsSet, &OldBlockedSignalsSet) ==
          -1) {
        Printf("Failed to block SIGCHLD\n");

        (void)sigaction(SIGQUIT, &OldSigQuitAction, NULL);
        (void)sigaction(SIGINT, &OldSigIntAction, NULL);
        (void)posix_spawnattr_destroy(&SpawnAttributes);
        return -1;
      }
    }
    ++ActiveThreadCount;
  }

  sigset_t DefaultSigSet;
  (void)sigemptyset(&DefaultSigSet);
  (void)sigaddset(&DefaultSigSet, SIGQUIT);
  (void)sigaddset(&DefaultSigSet, SIGINT);
  (void)posix_spawnattr_setsigdefault(&SpawnAttributes, &DefaultSigSet);

  (void)posix_spawnattr_setsigmask(&SpawnAttributes, &OldBlockedSignalsSet);
  short SpawnFlags = POSIX_SPAWN_SETSIGDEF | POSIX_SPAWN_SETSIGMASK;
  (void)posix_spawnattr_setflags(&SpawnAttributes, SpawnFlags);

  pid_t Pid;
  char **Environ = environ;
  const char *CommandCStr = Command.c_str();
  const char *Argv[] = {"sh", "-c", CommandCStr, NULL};
  int ErrorCode = 0, ProcessStatus = 0;

  ErrorCode = posix_spawn(&Pid, "/bin/sh", NULL, &SpawnAttributes,
                          (char *const *)Argv, Environ);
  (void)posix_spawnattr_destroy(&SpawnAttributes);
  if (!ErrorCode) {
    pid_t SavedPid = Pid;
    do {

      Pid = waitpid(SavedPid, &ProcessStatus,  0);
    } while (Pid == -1 && errno == EINTR);
    if (Pid == -1) {

      ProcessStatus = -1;
    }
  } else if (ErrorCode == ENOMEM || ErrorCode == EAGAIN) {

    ProcessStatus = -1;
  } else {

    ProcessStatus = W_EXITCODE(127, 0);
  }

  {
    std::lock_guard<std::mutex> Lock(SignalMutex);
    --ActiveThreadCount;
    if (ActiveThreadCount == 0) {
      bool FailedRestore = false;
      if (sigaction(SIGINT, &OldSigIntAction, NULL) == -1) {
        Printf("Failed to restore SIGINT handling\n");
        FailedRestore = true;
      }
      if (sigaction(SIGQUIT, &OldSigQuitAction, NULL) == -1) {
        Printf("Failed to restore SIGQUIT handling\n");
        FailedRestore = true;
      }
      if (sigprocmask(SIG_BLOCK, &OldBlockedSignalsSet, NULL) == -1) {
        Printf("Failed to unblock SIGCHLD\n");
        FailedRestore = true;
      }
      if (FailedRestore)
        ProcessStatus = -1;
    }
  }
  return ProcessStatus;
}

}

#endif
