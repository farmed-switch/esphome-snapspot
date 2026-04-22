#pragma once

#include <iostream>
#include <memory>

namespace bell {
class AudioContainer;
}

namespace bell::AudioContainers {
std::unique_ptr<bell::AudioContainer> guessAudioContainer(std::istream& istr);
}
