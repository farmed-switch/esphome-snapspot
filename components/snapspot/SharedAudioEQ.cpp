#include "SharedAudioEQ.h"
#include "eq_entities.h"
#include "esphome/core/log.h"

namespace esphome {
namespace shared_audio {

static const char *const TAG = "shared_audio";

void SharedAudioEQ::setup() {

  if (eq_switch_ != nullptr && eq_switch_->state) {
    eq_set_mode(15);
    for (size_t i = 0; i < band_numbers_.size(); i++) {
      set_eq_band(i, band_numbers_[i]->state);
    }
    enable_eq(true);
    ESP_LOGI(TAG, "EQ restored from NVS: ON, %d bands", (int)band_numbers_.size());
  } else if (eq_switch_ != nullptr) {
    enable_eq(false);
    ESP_LOGI(TAG, "EQ restored from NVS: OFF");
  } else {

    ESP_LOGD(TAG, "Shared Audio EQ ready (no software EQ controls)");
  }
}

void SharedAudioEQ::dump_config() {
  ESP_LOGCONFIG(TAG, "Shared Audio EQ:");
  ESP_LOGCONFIG(TAG, "  Software EQ controls: %s",
                eq_switch_ != nullptr ? "yes" : "no");
  ESP_LOGCONFIG(TAG, "  Band count: %d", (int)band_numbers_.size());
}

void SharedAudioEQ::notify_band_changed() {
  if (!preset_writing_ && eq_preset_ != nullptr) {
    if (eq_preset_->active_index().has_value()) {
      auto current = eq_preset_->at(eq_preset_->active_index().value());
      if (current.has_value() && current.value() != "Custom") {
        eq_preset_->make_call().set_option("Custom").perform();
      }
    }
  }
}

}
}
