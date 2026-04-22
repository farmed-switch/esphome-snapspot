#pragma once
#include <atomic>
#include <cstddef>

namespace esphome::snapspot {

extern std::atomic<bool> cspot_active;
extern std::atomic<bool> cspot_shutdown_pending;
extern std::atomic<bool> cspot_tasks_exited;
extern std::atomic<bool> cspot_fully_stopped;
extern std::atomic<bool> audio_path_free;

struct CspotCallbacks {
  void *ctx{nullptr};
  void (*close_connection)(void *ctx){nullptr};
  bool (*are_tasks_exited)(void *ctx){nullptr};
  void (*finalize_cleanup)(void *ctx){nullptr};
  void (*force_cleanup)(void *ctx){nullptr};
};
extern CspotCallbacks cspot_callbacks;

}
