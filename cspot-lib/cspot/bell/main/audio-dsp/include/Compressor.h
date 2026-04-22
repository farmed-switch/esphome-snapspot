#pragma once

#include <math.h>
#include <stdint.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AudioTransform.h"
#include "StreamInfo.h"
#include "TransformConfig.h"

#define pow10f(x) expf(2.302585092994046f * x)

float log2f_approx(float X);

#define log10f_fast(x) (log2f_approx(x) * 0.3010299956639812f)

namespace bell {
class Compressor : public bell::AudioTransform {
 private:
  std::vector<int> channels;
  std::vector<float> tmp;

  std::map<std::string, float> paramCache;

  float attack;
  float release;
  float threshold;
  float factor;
  float clipLimit;
  float makeupGain;

  float lastLoudness = -100.0f;

  float sampleRate = 44100;

 public:
  Compressor();
  ~Compressor(){};

  void configure(std::vector<int> channels, float attack, float release,
                 float threshold, float factor, float makeupGain);

  void sumChannels(std::unique_ptr<StreamInfo>& data);
  void calLoudness();
  void calGain();

  void applyGain(std::unique_ptr<StreamInfo>& data);

  void reconfigure() override {
    std::scoped_lock lock(this->accessMutex);
    auto newChannels = config->getChannels();

    float newAttack = config->getFloat("attack");
    float newRelease = config->getFloat("release");
    float newThreshold = config->getFloat("threshold");
    float newFactor = config->getFloat("factor");
    float newMakeupGain = config->getFloat("makeup_gain");

    if (paramCache["attack"] == newAttack &&
        paramCache["release"] == newRelease &&
        paramCache["threshold"] == newThreshold &&
        paramCache["factor"] == newFactor &&
        paramCache["makeup_gain"] == newMakeupGain) {
      return;
    } else {

      paramCache["attack"] = newAttack;
      paramCache["release"] = newRelease;
      paramCache["threshold"] = newThreshold;
      paramCache["factor"] = newFactor;
      paramCache["makeup_gain"] = newMakeupGain;
    }

    this->configure(newChannels, newAttack, newRelease, newThreshold, newFactor,
                    newMakeupGain);
  }

  std::unique_ptr<StreamInfo> process(
      std::unique_ptr<StreamInfo> data) override;
  void sampleRateChanged(uint32_t sampleRate) override {
    this->sampleRate = sampleRate;
  };
};
};
