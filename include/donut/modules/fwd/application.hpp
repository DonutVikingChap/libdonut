#ifndef DONUT_MODULES_FWD_APPLICATION_HPP
#define DONUT_MODULES_FWD_APPLICATION_HPP

#include <cstdint> // std::uint8_t

namespace donut {
namespace application {

struct TickInfo;
struct FrameInfo;
struct ApplicationOptions;
class Application;

struct Error;

struct EventBase;
struct ApplicationEventBase;
struct WindowEventBase;
struct InputEventBase;
struct KeyEventBase;
struct TextEventBase;
struct MouseEventBase;
struct MouseButtonEventBase;
struct ControllerEventBase;
struct ControllerAxisEventBase;
struct ControllerButtonEventBase;
struct TouchEventBase;
struct KeymapEventBase;
struct ClipboardEventBase;
struct DropEventBase;
struct ApplicationTerminatingEvent;
struct ApplicationLowMemoryEvent;
struct ApplicationPausingEvent;
struct ApplicationPausedEvent;
struct ApplicationUnpausingEvent;
struct ApplicationUnpausedEvent;
struct WindowShownEvent;
struct WindowHiddenEvent;
struct WindowExposedEvent;
struct WindowMovedEvent;
struct WindowSizeChangedEvent;
struct WindowMinimizedEvent;
struct WindowMaximizedEvent;
struct WindowRestoredEvent;
struct WindowMouseFocusGainedEvent;
struct WindowMouseFocusLostEvent;
struct WindowKeyboardFocusGainedEvent;
struct WindowKeyboardFocusLostEvent;
struct WindowCloseRequestedEvent;
struct WindowDisplayChangedEvent;
struct KeyPressedEvent;
struct KeyPressRepeatedEvent;
struct KeyReleasedEvent;
struct TextInputEditedEvent;
struct TextInputSubmittedEvent;
struct MouseMovedEvent;
struct MouseButtonPressedEvent;
struct MouseButtonReleasedEvent;
struct MouseWheelScrolledEvent;
struct ControllerAddedEvent;
struct ControllerRemovedEvent;
struct ControllerRemappedEvent;
struct ControllerAxisMovedEvent;
struct ControllerButtonPressedEvent;
struct ControllerButtonReleasedEvent;
struct TouchMovedEvent;
struct TouchPressedEvent;
struct TouchReleasedEvent;
struct KeymapChangedEvent;
struct ClipboardUpdatedEvent;
struct DropFileEvent;
struct DropTextEvent;
struct DropStartedEvent;
struct DropCompletedEvent;
struct Event;

enum class Input : std::uint8_t;

struct InputManagerOptions;
class InputManager;

} // namespace application
} // namespace donut

#endif
