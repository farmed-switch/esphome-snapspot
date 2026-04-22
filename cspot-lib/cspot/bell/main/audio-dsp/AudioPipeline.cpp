#include "AudioPipeline.h"

#include <type_traits>
#include <utility>

#include "AudioTransform.h"
#include "BellLogger.h"
#include "TransformConfig.h"

using namespace bell;

AudioPipeline::AudioPipeline(){

};

void AudioPipeline::addTransform(std::shared_ptr<AudioTransform> transform) {
  transforms.push_back(transform);
  recalculateHeadroom();
}

void AudioPipeline::recalculateHeadroom() {
  float headroom = 0.0f;

  for (auto transform : transforms) {
    if (headroom < transform->calculateHeadroom()) {
      headroom = transform->calculateHeadroom();
    }
  }

}

void AudioPipeline::volumeUpdated(int volume) {
  BELL_LOG(debug, "AudioPipeline", "Requested");
  std::scoped_lock lock(this->accessMutex);
  for (auto transform : transforms) {
    transform->config->currentVolume = volume;
    transform->reconfigure();
  }
  BELL_LOG(debug, "AudioPipeline", "Volume applied, DSP reconfigured");
}

std::unique_ptr<StreamInfo> AudioPipeline::process(
    std::unique_ptr<StreamInfo> data) {
  std::scoped_lock lock(this->accessMutex);
  for (auto& transform : transforms) {
    data = transform->process(std::move(data));
  }

  return data;
}