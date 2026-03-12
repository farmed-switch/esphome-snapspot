#include "SharedAudioEQ.h"
#include "esphome/core/log.h"

namespace esphome {
namespace shared_audio {

static const char *const TAG = "shared_audio";

void SharedAudioEQ::dump_config() {
  ESP_LOGCONFIG(TAG, "Shared Audio: 18-band graphic EQ ready");
}

}  // namespace shared_audio
}  // namespace esphome
