#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <variant>
namespace cspot {
struct TrackInfo {
  std::string name;
  std::string album;
  std::string artist;
  std::string imageUrl;
  std::string trackId;
  uint32_t duration = 0;
  uint32_t number = 0;
  uint32_t discNumber = 0;
};
class SpircHandler {
 public:
  enum class EventType {
    PLAY_PAUSE, VOLUME, TRACK_INFO, DISC, NEXT, PREV, SEEK, DEPLETED, FLUSH, PLAYBACK_START
  };
  typedef std::variant<TrackInfo, int, bool> EventData;
  struct Event {
    EventType eventType;
    EventData data;
  };
  typedef std::function<void(std::unique_ptr<Event>)> EventHandler;
  virtual ~SpircHandler() = default;
};
}
