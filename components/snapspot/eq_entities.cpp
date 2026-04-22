#include "eq_entities.h"
#include "SharedAudioEQ.h"
#include "shared_audio_eq.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome {
namespace shared_audio {

static const char *const TAG = "eq_entities";

void EQBandNumber::setup() {
  float value;
  this->pref_ = this->make_entity_preference<float>();
  if (!this->pref_.load(&value)) {
    value = 0.0f;
  }
  this->publish_state(value);
}

void EQBandNumber::control(float value) {
  set_eq_band(this->band_index_, value);
  this->publish_state(value);
  this->pref_.save(&value);

  if (this->parent_ != nullptr) {
    this->parent_->notify_band_changed();
  }
}

void EQEnableSwitch::write_state(bool state) {
  if (state) {
    eq_set_mode(15);
    enable_eq(true);
    ESP_LOGI(TAG, "Software EQ enabled (15-band ISO)");
  } else {
    enable_eq(false);
    ESP_LOGI(TAG, "Software EQ disabled");
  }
  this->publish_state(state);
}

void EQPresetSelect::setup() {
  size_t index;
  this->pref_ = this->make_entity_preference<size_t>();
  if (this->pref_.load(&index) && index < this->size()) {
    auto val = this->at(index);
    if (val.has_value()) {
      this->publish_state(val.value());
      return;
    }
  }
  if (this->size() > 0) {
    auto val = this->at(0);
    if (val.has_value())
      this->publish_state(val.value());
  }
}

void EQPresetSelect::control(const std::string &value) {

  static const float BASS_BOOST[]     = {10, 8, 6, 3,-2,-8,-13,-15,-15,-15,-15,-15,-15,-15,-15};
  static const float TREBLE_BOOST[]   = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 4};
  static const float LOUDNESS[]       = { 5, 4, 3, 1, 0,-1,-2,-1, 0, 0, 1, 2, 3, 4, 3};
  static const float VOCAL_CLARITY[]  = { 0, 0, 0, 0, 0,-1, 0, 1, 2, 3, 2, 1, 0, 0, 0};
  static const float SMALL_SPEAKER[]  = {-6,-4,-2, 0, 1, 1, 2, 2, 2, 2, 3, 3, 4, 3, 2};
  static const float NIGHT_MODE[]     = {-4,-3,-2,-1, 0, 0, 1, 1, 1, 0, 0,-1,-2,-3,-4};

  const float *gains = nullptr;
  bool bypass = false;
  bool do_enable = true;

  if (value == "Flat") {
    bypass = true;
    do_enable = false;
  } else if (value == "Bass Boost") {
    gains = BASS_BOOST;
  } else if (value == "Treble Boost") {
    gains = TREBLE_BOOST;
  } else if (value == "Loudness") {
    gains = LOUDNESS;
  } else if (value == "Vocal Clarity") {
    gains = VOCAL_CLARITY;
  } else if (value == "Small Speaker") {
    gains = SMALL_SPEAKER;
  } else if (value == "Night Mode") {
    gains = NIGHT_MODE;
  } else if (value == "Custom") {

    this->publish_state(value);
    auto idx = this->active_index();
    if (idx.has_value()) { size_t i = idx.value(); this->pref_.save(&i); }
    return;
  } else {
    ESP_LOGW(TAG, "Unknown preset: %s", value.c_str());
    return;
  }

  if (this->parent_ == nullptr) {
    this->publish_state(value);
    auto idx = this->active_index();
    if (idx.has_value()) { size_t i = idx.value(); this->pref_.save(&i); }
    return;
  }

  this->parent_->set_preset_writing(true);

  auto &bands = this->parent_->band_numbers();
  for (size_t i = 0; i < bands.size() && i < 15; i++) {
    float g = (gains != nullptr) ? gains[i] : 0.0f;
    bands[i]->make_call().set_value(g).perform();
  }

  this->parent_->set_preset_writing(false);

  if (bypass) {
    enable_eq(false);
  } else if (do_enable) {
    enable_eq(true);
  }

  this->publish_state(value);
  auto idx = this->active_index();
  if (idx.has_value()) { size_t i = idx.value(); this->pref_.save(&i); }
  ESP_LOGI(TAG, "Applied preset: %s", value.c_str());
}

}
}
