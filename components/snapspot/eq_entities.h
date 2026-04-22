#pragma once

#include "esphome/core/component.h"
#include "esphome/core/preferences.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/select/select.h"

namespace esphome {
namespace shared_audio {

class SharedAudioEQ;

class EQBandNumber : public number::Number, public Component {
 public:
  void set_band_index(int idx) { band_index_ = idx; }
  void set_eq_parent(SharedAudioEQ *p) { parent_ = p; }
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;

 protected:
  void control(float value) override;
  int band_index_{0};
  SharedAudioEQ *parent_{nullptr};
  ESPPreferenceObject pref_;
};

class EQEnableSwitch : public switch_::Switch, public Component {
 public:
  void set_eq_parent(SharedAudioEQ *p) { parent_ = p; }
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override {}

 protected:
  void write_state(bool state) override;
  SharedAudioEQ *parent_{nullptr};
};

class EQPresetSelect : public select::Select, public Component {
 public:
  void set_eq_parent(SharedAudioEQ *p) { parent_ = p; }
  float get_setup_priority() const override { return setup_priority::DATA; }
  void setup() override;

 protected:
  void control(const std::string &value) override;
  SharedAudioEQ *parent_{nullptr};
  ESPPreferenceObject pref_;
};

}
}
