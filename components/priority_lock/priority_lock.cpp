#include "priority_lock.h"
#include "esphome/core/log.h"

namespace esphome {
namespace priority_lock {

const char *const PriorityLockManager::TAG = "priority_lock";

void PriorityLockManager::setup() {
  this->mutex_ = xSemaphoreCreateMutex();
  if (this->mutex_ == nullptr) {
    ESP_LOGE(TAG, "Failed to create mutex");
    this->mark_failed();
    return;
  }
  ESP_LOGV(TAG, "PriorityLockManager ready");
}

void PriorityLockManager::dump_config() {
  ESP_LOGCONFIG(TAG, "Priority Lock Manager (v2.0):");
  ESP_LOGCONFIG(TAG, "  Registered sources: %u", (unsigned) this->sources_.size());
  for (const auto &s : this->sources_) {
    const bool owns = (this->current_owner_ != nullptr &&
                       std::strcmp(this->current_owner_, s.id) == 0);
    ESP_LOGCONFIG(TAG, "    - '%s' priority=%u%s", s.id, s.priority, owns ? " [OWNER]" : "");
  }
}

void PriorityLockManager::register_source(const char *id,
                                           uint8_t priority,
                                           std::function<void()> on_preempt,
                                           std::function<void()> on_resume) {
  xSemaphoreTake(this->mutex_, portMAX_DELAY);
  // Avoid duplicate registration.
  for (const auto &s : this->sources_) {
    if (std::strcmp(s.id, id) == 0) {
      xSemaphoreGive(this->mutex_);
      ESP_LOGW(TAG, "Source '%s' already registered — ignoring duplicate", id);
      return;
    }
  }
  this->sources_.push_back({id, priority, std::move(on_preempt), std::move(on_resume)});
  xSemaphoreGive(this->mutex_);
  ESP_LOGD(TAG, "Registered source '%s' with priority %u", id, priority);
}

bool PriorityLockManager::try_acquire(const char *id) {
  xSemaphoreTake(this->mutex_, portMAX_DELAY);
  if (this->current_owner_ != nullptr) {
    // v2.0: no preemption — we only give it if already the owner (idempotent).
    const bool already_own = std::strcmp(this->current_owner_, id) == 0;
    xSemaphoreGive(this->mutex_);
    if (!already_own) {
      ESP_LOGD(TAG, "try_acquire('%s') denied — '%s' holds the lock", id, this->current_owner_);
    }
    return already_own;
  }
  this->current_owner_ = id;
  xSemaphoreGive(this->mutex_);
  ESP_LOGD(TAG, "try_acquire('%s') granted", id);
  return true;
}

void PriorityLockManager::release(const char *id) {
  xSemaphoreTake(this->mutex_, portMAX_DELAY);
  if (this->current_owner_ == nullptr || std::strcmp(this->current_owner_, id) != 0) {
    xSemaphoreGive(this->mutex_);
    ESP_LOGW(TAG, "release('%s') called but '%s' is the owner — ignoring", id,
             this->current_owner_ ? this->current_owner_ : "(none)");
    return;
  }
  this->current_owner_ = nullptr;
  xSemaphoreGive(this->mutex_);
  ESP_LOGD(TAG, "release('%s') — lock now free", id);
}

}  // namespace priority_lock
}  // namespace esphome
