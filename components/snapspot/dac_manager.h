#pragma once

#include "esphome/core/component.h"
#include "esphome/components/speaker/speaker.h"

#ifdef USE_AUDIO_DAC
#include "esphome/components/audio_dac/audio_dac.h"
#endif

namespace esphome {
namespace snapspot {

class DacManager : public Component {
 public:
  void set_output_speaker(speaker::Speaker *speaker) { this->output_speaker_ = speaker; }
#ifdef USE_AUDIO_DAC
  void set_audio_dac(audio_dac::AudioDac *dac) { this->audio_dac_ = dac; }
#endif

  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

 protected:
  speaker::Speaker *output_speaker_{nullptr};
#ifdef USE_AUDIO_DAC
  audio_dac::AudioDac *audio_dac_{nullptr};
#endif
  bool was_running_{false};
};

}
}
