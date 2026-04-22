#pragma once

#include "esphome/core/component.h"
#include "shared_audio_eq.h"
#include <vector>

namespace esphome {
namespace shared_audio {

class EQBandNumber;
class EQEnableSwitch;
class EQPresetSelect;

class SharedAudioEQ : public Component {
 public:
  float get_setup_priority() const override { return setup_priority::DATA - 1.0f; }
  void setup() override;
  void dump_config() override;

  void set_eq_switch(EQEnableSwitch *sw) { eq_switch_ = sw; }
  void set_eq_preset(EQPresetSelect *sel) { eq_preset_ = sel; }
  void add_band_number(EQBandNumber *num) { band_numbers_.push_back(num); }

  void notify_band_changed();
  bool preset_writing() const { return preset_writing_; }
  void set_preset_writing(bool v) { preset_writing_ = v; }

  std::vector<EQBandNumber *> &band_numbers() { return band_numbers_; }
  EQPresetSelect *eq_preset() { return eq_preset_; }

 protected:
  EQEnableSwitch *eq_switch_{nullptr};
  EQPresetSelect *eq_preset_{nullptr};
  std::vector<EQBandNumber *> band_numbers_;
  bool preset_writing_{false};
};

}
}
