#pragma once

#include <memory>

#include "AudioContainer.h"
#include "BaseCodec.h"
#include "CodecType.h"

namespace bell {
class AudioCodecs {
 public:
  static std::shared_ptr<BaseCodec> getCodec(AudioCodec type);
  static std::shared_ptr<BaseCodec> getCodec(AudioContainer* container);
  static void addCodec(AudioCodec type,
                       const std::shared_ptr<BaseCodec>& codec);
};
}
