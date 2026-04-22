#include "dac_manager.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snapspot {

static const char *const TAG = "snapspot.dac";

void DacManager::setup() {
#ifdef USE_AUDIO_DAC
  if (this->audio_dac_) {
    this->audio_dac_->set_mute_on();
    ESP_LOGI(TAG, "DAC muted at boot — will unmute when mixer starts");
  }
#endif
}

void DacManager::loop() {
#ifdef USE_AUDIO_DAC
  if (!this->audio_dac_ || !this->output_speaker_)
    return;

  bool running = this->output_speaker_->is_running();
  if (running && !this->was_running_) {
    this->audio_dac_->set_mute_off();
    ESP_LOGD(TAG, "Mixer running — DAC unmuted");
  } else if (!running && this->was_running_) {
    this->audio_dac_->set_mute_on();
    ESP_LOGD(TAG, "Mixer stopped — DAC muted");
  }
  this->was_running_ = running;
#endif
}

}
}
