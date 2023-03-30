#ifndef DONUT_APPLICATION_EVENT_HPP
#define DONUT_APPLICATION_EVENT_HPP

#include <donut/Variant.hpp>
#include <donut/application/Input.hpp>

#include <cstdint>     // std::int16_t, std::int32_t, std::uint8_t, std::uint16_t, std::uint32_t
#include <glm/glm.hpp> // glm::...
#include <string>      // std::string

namespace donut {
namespace application {

/** Event base. */
struct EventBase {};

/** Application Event base. */
struct ApplicationEventBase : EventBase {};

/** Window Event base. */
struct WindowEventBase : EventBase {
	std::uint32_t windowId; ///< Unique identifier of the window.
};

/** Input Event base. */
struct InputEventBase : EventBase {
	std::uint32_t windowId; ///< Unique identifier of the window that this event belongs to, if any.
};

/** Keyboard key Event base. */
struct KeyEventBase : InputEventBase {
	/** Bit set of key modifiers. */
	using KeyModifiers = std::uint16_t;

	/** Key modifier bits. */
	enum KeyModifier : KeyModifiers {
		MODIFIER_NONE = 0,              ///< No modifiers.
		MODIFIER_LSHIFT = 1 << 0,       ///< Left shift.
		MODIFIER_RSHIFT = 1 << 1,       ///< Right shift.
		MODIFIER_LCTRL = 1 << 6,        ///< Left control.
		MODIFIER_RCTRL = 1 << 7,        ///< Right control.
		MODIFIER_LALT = 1 << 8,         ///< Left alt.
		MODIFIER_RALT = 1 << 9,         ///< Right alt.
		MODIFIER_LSUPER = 1 << 10,      ///< Left super.
		MODIFIER_RSUPER = 1 << 11,      ///< Right super.
		MODIFIER_NUM_LOCK = 1 << 12,    ///< Number lock.
		MODIFIER_CAPS_LOCK = 1 << 13,   ///< Capital lock.
		MODIFIER_MODE = 1 << 14,        ///< Mode.
		MODIFIER_SCROLL_LOCK = 1 << 15, ///< Scroll lock.

		MODIFIER_CTRL = MODIFIER_LCTRL | MODIFIER_RCTRL,    ///< Left or right control.
		MODIFIER_SHIFT = MODIFIER_LSHIFT | MODIFIER_RSHIFT, ///< Left or right shift.
		MODIFIER_ALT = MODIFIER_LALT | MODIFIER_RALT,       ///< Left or right alt.
		MODIFIER_SUPER = MODIFIER_LSUPER | MODIFIER_RSUPER, ///< Left or right super.
	};

	Input physicalKeyInput;    ///< Physical key input identifier.
	int symbolicKeyCode;       ///< Symbolic identifier of the key.
	KeyModifiers keyModifiers; ///< Current key modifiers.
};

/** Text input Event base. */
struct TextInputEventBase : InputEventBase {
	std::string text; ///< The text being edited.
};

/** Mouse Event base. */
struct MouseEventBase : InputEventBase {
	std::uint32_t mouseId;          ///< Unique identifier of the mouse instance.
	glm::ivec2 mousePosition;       ///< Current mouse position relative to the window.
	glm::ivec2 relativeMouseMotion; ///< Position offset relative to the previous position.
};

/** Mouse button Event base. */
struct MouseButtonEventBase : MouseEventBase {
	Input physicalMouseButtonInput; ///< Physical button input identifier.
	std::uint8_t clickCount;        ///< Number of consecutive clicks within a short time interval.
};

/** Controller Event base. */
struct ControllerEventBase : InputEventBase {
	std::int32_t controllerId; ///< Unique identifier of the controller instance.
};

/** Controller axis Event base. */
struct ControllerAxisEventBase : ControllerEventBase {
	/**
     * Controller axis enumeration.
     */
	enum class ControllerAxis : std::uint8_t {
		LEFT_STICK_X = 0,  ///< Horizontal movement of the left analog stick.
		LEFT_STICK_Y = 1,  ///< Vertical movement of the left analog stick.
		RIGHT_STICK_X = 2, ///< Horizontal movement of the right analog stick.
		RIGHT_STICK_Y = 3, ///< Vertical movement of the right analog stick.
		LEFT_TRIGGER = 4,  ///< Movement of the left trigger.
		RIGHT_TRIGGER = 5, ///< Movement of the right trigger.
		INVALID = 255,     ///< Invalid axis.
	};

	ControllerAxis axis;    ///< Physical controller axis.
	std::int16_t axisValue; ///< Current axis value in the range [-32768, 32767].
};

/** Controller button Event base. */
struct ControllerButtonEventBase : ControllerEventBase {
	Input physicalControllerButtonInput; ///< Physical button input identifier.
};

/** Touch Event base. */
struct TouchEventBase : InputEventBase {
	std::int64_t touchDeviceId;               ///< Touch device identifier.
	std::int64_t fingerId;                    ///< Finger identifier.
	glm::vec2 normalizedFingerPosition;       ///< Current finger position, normalized to the range [0, 1].
	glm::vec2 relativeNormalizedFingerMotion; ///< Finger offset relative to the previous position, normalized to the range [-1, 1].
	float normalizedFingerPressure;           ///< Amount of pressure applied, normalized to the range [0, 1].
};

/** Keyboard keymap Event base. */
struct KeymapEventBase : EventBase {};

/** Clipboard Event base. */
struct ClipboardEventBase : EventBase {};

/** Drop Event base. */
struct DropEventBase : EventBase {
	std::uint32_t windowId; ///< Unique identifier of the window that was dropped onto, if any.
};

/** Application is being terminated by the OS. */
struct ApplicationTerminatingEvent : ApplicationEventBase {};

/** Application is low on memory. */
struct ApplicationLowMemoryEvent : ApplicationEventBase {};

/** Application is about to be paused by the OS. */
struct ApplicationPausingEvent : ApplicationEventBase {};

/** Application was paused by the OS. */
struct ApplicationPausedEvent : ApplicationEventBase {};

/** Application is about to be unpaused by the OS. */
struct ApplicationUnpausingEvent : ApplicationEventBase {};

/** Application was unpaused by the OS. */
struct ApplicationUnpausedEvent : ApplicationEventBase {};

/** Window was shown. */
struct WindowShownEvent : WindowEventBase {};

/** Window was hidden. */
struct WindowHiddenEvent : WindowEventBase {};

/** Window was exposed. */
struct WindowExposedEvent : WindowEventBase {};

/** Window was moved. */
struct WindowMovedEvent : WindowEventBase {
	glm::ivec2 windowPosition; ///< Current window position.
};

/** Window size was changed. */
struct WindowSizeChangedEvent : WindowEventBase {
	glm::ivec2 windowSize; ///< Current window size.
};

/** Window was minimized. */
struct WindowMinimizedEvent : WindowEventBase {};

/** Window was maximized. */
struct WindowMaximizedEvent : WindowEventBase {};

/** Window was restored. */
struct WindowRestoredEvent : WindowEventBase {};

/** Window gained mouse focus. */
struct WindowMouseFocusGainedEvent : WindowEventBase {};

/** Window lost mouse focus. */
struct WindowMouseFocusLostEvent : WindowEventBase {};

/** Window gained keyboard focus. */
struct WindowKeyboardFocusGainedEvent : WindowEventBase {};

/** Window lost keyboard focus. */
struct WindowKeyboardFocusLostEvent : WindowEventBase {};

/** Window was requested to close. */
struct WindowCloseRequestedEvent : WindowEventBase {};

/** Window was moved to a new display. */
struct WindowDisplayChangedEvent : WindowEventBase {
	int newDisplayIndex; ///< New display index.
};

/** Keyboard key was pressed. */
struct KeyPressedEvent : KeyEventBase {};

/** Keyboard key was held, causing a repeat press. */
struct KeyPressRepeatedEvent : KeyEventBase {};

/** Keyboard key was released. */
struct KeyReleasedEvent : KeyEventBase {};

/** Text input was edited. */
struct TextInputEditedEvent : TextInputEventBase {
	std::int32_t textCursorOffset;          ///< The cursor offset of the start of the selected text.
	std::int32_t textCursorSelectionLength; ///< The length of the current text selection, if any.
};

/** Text input was submitted. */
struct TextInputSubmittedEvent : TextInputEventBase {};

/** Mouse was moved. */
struct MouseMovedEvent : MouseEventBase {};

/** Mouse button was pressed. */
struct MouseButtonPressedEvent : MouseButtonEventBase {};

/** Mouse button was released. */
struct MouseButtonReleasedEvent : MouseButtonEventBase {};

/** Mouse wheel was scrolled. */
struct MouseWheelScrolledEvent : MouseEventBase {
	glm::ivec2 scrollTicks; ///< Number of scrolls horizontally/vertically.
	glm::vec2 scrollAmount; ///< Amount scrolled horizontally/vertically, with floating-point precision.
};

/** Controller was added. */
struct ControllerAddedEvent : ControllerEventBase {};

/** Controller was removed. */
struct ControllerRemovedEvent : ControllerEventBase {};

/** Controller was remapped. */
struct ControllerRemappedEvent : ControllerEventBase {};

/** Controller axis was moved. */
struct ControllerAxisMovedEvent : ControllerAxisEventBase {};

/** Controller button was pressed. */
struct ControllerButtonPressedEvent : ControllerButtonEventBase {};

/** Controller button was released. */
struct ControllerButtonReleasedEvent : ControllerButtonEventBase {};

/** Touch was moved. */
struct TouchMovedEvent : TouchEventBase {};

/** Touch was pressed. */
struct TouchPressedEvent : TouchEventBase {};

/** Touch was released. */
struct TouchReleasedEvent : TouchEventBase {};

/** Keyboard keymap was changed. */
struct KeymapChangedEvent : KeymapEventBase {};

/** Clipboard was updated. */
struct ClipboardUpdatedEvent : ClipboardEventBase {};

/** File was dropped. */
struct DropFileEvent : DropEventBase {
	std::string droppedFilepath; ///< Filepath of the dropped file.
};

/** Text was dropped. */
struct DropTextEvent : DropEventBase {
	std::string droppedText; ///< Dropped text.
};

/** Drop was started. */
struct DropStartedEvent : DropEventBase {};

/** Drop was completed. */
struct DropCompletedEvent : DropEventBase {};

/**
 * Data structure containing information about an Application event.
 *
 * Instances of this type are generated by an Application when certain events
 * occur in the application environment, typically as a result of user input.
 * These events should mainly be forwarded to subsystems such as an InputManager
 * for processing and usually do not need to be handled manually.
 */
struct Event
	: Variant<                            //
		  ApplicationTerminatingEvent,    //
		  ApplicationLowMemoryEvent,      //
		  ApplicationPausingEvent,        //
		  ApplicationPausedEvent,         //
		  ApplicationUnpausingEvent,      //
		  ApplicationUnpausedEvent,       //
		  WindowShownEvent,               //
		  WindowHiddenEvent,              //
		  WindowExposedEvent,             //
		  WindowMovedEvent,               //
		  WindowSizeChangedEvent,         //
		  WindowMinimizedEvent,           //
		  WindowMaximizedEvent,           //
		  WindowRestoredEvent,            //
		  WindowMouseFocusGainedEvent,    //
		  WindowMouseFocusLostEvent,      //
		  WindowKeyboardFocusGainedEvent, //
		  WindowKeyboardFocusLostEvent,   //
		  WindowCloseRequestedEvent,      //
		  WindowDisplayChangedEvent,      //
		  KeyPressedEvent,                //
		  KeyPressRepeatedEvent,          //
		  KeyReleasedEvent,               //
		  TextInputEditedEvent,           //
		  TextInputSubmittedEvent,        //
		  MouseMovedEvent,                //
		  MouseButtonPressedEvent,        //
		  MouseButtonReleasedEvent,       //
		  MouseWheelScrolledEvent,        //
		  ControllerAddedEvent,           //
		  ControllerRemovedEvent,         //
		  ControllerRemappedEvent,        //
		  ControllerAxisMovedEvent,       //
		  ControllerButtonPressedEvent,   //
		  ControllerButtonReleasedEvent,  //
		  TouchMovedEvent,                //
		  TouchPressedEvent,              //
		  TouchReleasedEvent,             //
		  KeymapChangedEvent,             //
		  ClipboardUpdatedEvent,          //
		  DropFileEvent,                  //
		  DropTextEvent,                  //
		  DropStartedEvent,               //
		  DropCompletedEvent> {};

} // namespace application
} // namespace donut

#endif
