#pragma once

#include "esphome/core/component.h"
#include "shared_audio_eq.h"

namespace esphome {
namespace shared_audio {

// Minimal ESPHome component wrapper for the shared_audio 18-band graphic EQ.
// Instantiated by to_code() so that ESPHome emits the correct #include for
// this header in main.cpp — which in turn pulls in shared_audio_eq.h and its
// extern "C" declarations (eq_init, set_eq_band, apply_eq_preset, enable_eq).
// This makes all four functions available to every lambda in the user's YAML
// without any per-lambda extern declaration.
class SharedAudioEQ : public Component {
 public:
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void setup() override {}
  void dump_config() override;
};

}  // namespace shared_audio
}  // namespace esphome
