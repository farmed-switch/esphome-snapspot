#pragma once
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <functional>
#include <vector>
#include <cstring>

namespace esphome {
namespace priority_lock {

// ============================================================================
// PriorityLockManager  (v2.0 — no preemption)
//
// Allows multiple audio sources to register themselves with a priority level.
// Only one source may "own" the lock at a time. A lower priority number wins
// (1 = highest priority, e.g. TTS; 3 = lowest, e.g. Snapcast).
//
// v2.0 policy: no preemption. try_acquire() returns false if any source
// currently holds the lock, regardless of priority. A source must call
// release() before another may acquire.
//
// NOTE: v2.1 will add preemption (higher-priority source can evict current
// owner; on_preempt callback lets the owner clean up gracefully).
// ============================================================================
class PriorityLockManager : public Component {
 public:
  static const char *const TAG;

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::BUS; }

  // Register an audio source before trying to acquire the lock.
  //   id       – unique string identifier (e.g. "snapclient", "spotify")
  //   priority – lower number = higher priority (1 wins over 3)
  //   on_preempt / on_resume – reserved for v2.1 preemption; pass nullptr
  void register_source(const char *id, uint8_t priority,
                       std::function<void()> on_preempt,
                       std::function<void()> on_resume);

  // Try to acquire the lock for `id`. Returns true on success.
  // v2.0: returns false if any other source holds the lock.
  bool try_acquire(const char *id);

  // Release the lock held by `id`.  No-op if `id` is not the current owner.
  void release(const char *id);

  // Returns the id of the current owner, or nullptr if unlocked.
  const char *current_owner() const { return current_owner_; }

 protected:
  struct Source {
    const char *id;
    uint8_t priority;
    std::function<void()> on_preempt;
    std::function<void()> on_resume;
  };

  SemaphoreHandle_t mutex_{nullptr};
  std::vector<Source> sources_;
  const char *current_owner_{nullptr};
};

}  // namespace priority_lock
}  // namespace esphome
