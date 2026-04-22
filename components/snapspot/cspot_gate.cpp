#include "cspot_gate.h"
namespace esphome::snapspot {
std::atomic<bool> cspot_active{false};
std::atomic<bool> cspot_shutdown_pending{false};
std::atomic<bool> cspot_tasks_exited{true};
std::atomic<bool> cspot_fully_stopped{true};
std::atomic<bool> audio_path_free{false};
CspotCallbacks cspot_callbacks{};
}
