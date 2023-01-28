#ifndef DONUT_MODULES_FWD_APPLICATION_HPP
#define DONUT_MODULES_FWD_APPLICATION_HPP

#include <cstdint> // std::uint8_t

union SDL_Event;

namespace donut {
namespace application {

struct TickInfo;
struct FrameInfo;
struct ApplicationOptions;
class Application;

struct Error;

using Event = SDL_Event;

enum class Input : std::uint8_t;

struct InputManagerOptions;
class InputManager;

} // namespace application
} // namespace donut

#endif
