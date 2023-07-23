#include <donut/events/Error.hpp>
#include <donut/events/Event.hpp>
#include <donut/events/EventPump.hpp>
#include <donut/events/Input.hpp>
#include <donut/math.hpp>

#include <SDL.h>    // SDL..., Uint8
#include <array>    // std::array
#include <format>   // std::format
#include <limits>   // std::numeric_limits
#include <optional> // std::optional
#include <span>     // std::span
#include <string>   // std::string
#include <utility>  // std::move

namespace donut::events {

namespace {

constexpr std::array<Input, SDL_NUM_SCANCODES> KEY_SCANCODE_MAP = [] {
	std::array<Input, SDL_NUM_SCANCODES> result{};
	result[SDL_SCANCODE_A] = Input::KEY_A;
	result[SDL_SCANCODE_B] = Input::KEY_B;
	result[SDL_SCANCODE_C] = Input::KEY_C;
	result[SDL_SCANCODE_D] = Input::KEY_D;
	result[SDL_SCANCODE_E] = Input::KEY_E;
	result[SDL_SCANCODE_F] = Input::KEY_F;
	result[SDL_SCANCODE_G] = Input::KEY_G;
	result[SDL_SCANCODE_H] = Input::KEY_H;
	result[SDL_SCANCODE_I] = Input::KEY_I;
	result[SDL_SCANCODE_J] = Input::KEY_J;
	result[SDL_SCANCODE_K] = Input::KEY_K;
	result[SDL_SCANCODE_L] = Input::KEY_L;
	result[SDL_SCANCODE_M] = Input::KEY_M;
	result[SDL_SCANCODE_N] = Input::KEY_N;
	result[SDL_SCANCODE_O] = Input::KEY_O;
	result[SDL_SCANCODE_P] = Input::KEY_P;
	result[SDL_SCANCODE_Q] = Input::KEY_Q;
	result[SDL_SCANCODE_R] = Input::KEY_R;
	result[SDL_SCANCODE_S] = Input::KEY_S;
	result[SDL_SCANCODE_T] = Input::KEY_T;
	result[SDL_SCANCODE_U] = Input::KEY_U;
	result[SDL_SCANCODE_V] = Input::KEY_V;
	result[SDL_SCANCODE_W] = Input::KEY_W;
	result[SDL_SCANCODE_X] = Input::KEY_X;
	result[SDL_SCANCODE_Y] = Input::KEY_Y;
	result[SDL_SCANCODE_Z] = Input::KEY_Z;
	result[SDL_SCANCODE_1] = Input::KEY_1;
	result[SDL_SCANCODE_2] = Input::KEY_2;
	result[SDL_SCANCODE_3] = Input::KEY_3;
	result[SDL_SCANCODE_4] = Input::KEY_4;
	result[SDL_SCANCODE_5] = Input::KEY_5;
	result[SDL_SCANCODE_6] = Input::KEY_6;
	result[SDL_SCANCODE_7] = Input::KEY_7;
	result[SDL_SCANCODE_8] = Input::KEY_8;
	result[SDL_SCANCODE_9] = Input::KEY_9;
	result[SDL_SCANCODE_0] = Input::KEY_0;
	result[SDL_SCANCODE_ESCAPE] = Input::KEY_ESCAPE;
	result[SDL_SCANCODE_LCTRL] = Input::KEY_LCTRL;
	result[SDL_SCANCODE_RCTRL] = Input::KEY_RCTRL;
	result[SDL_SCANCODE_LSHIFT] = Input::KEY_LSHIFT;
	result[SDL_SCANCODE_RSHIFT] = Input::KEY_RSHIFT;
	result[SDL_SCANCODE_LALT] = Input::KEY_LALT;
	result[SDL_SCANCODE_RALT] = Input::KEY_RALT;
	result[SDL_SCANCODE_MENU] = Input::KEY_MENU;
	result[SDL_SCANCODE_LEFTBRACKET] = Input::KEY_LEFT_BRACKET;
	result[SDL_SCANCODE_RIGHTBRACKET] = Input::KEY_RIGHT_BRACKET;
	result[SDL_SCANCODE_SEMICOLON] = Input::KEY_SEMICOLON;
	result[SDL_SCANCODE_COMMA] = Input::KEY_COMMA;
	result[SDL_SCANCODE_PERIOD] = Input::KEY_PERIOD;
	result[SDL_SCANCODE_APOSTROPHE] = Input::KEY_APOSTROPHE;
	result[SDL_SCANCODE_SLASH] = Input::KEY_SLASH;
	result[SDL_SCANCODE_BACKSLASH] = Input::KEY_BACKSLASH;
	result[SDL_SCANCODE_GRAVE] = Input::KEY_GRAVE;
	result[SDL_SCANCODE_EQUALS] = Input::KEY_EQUALS;
	result[SDL_SCANCODE_MINUS] = Input::KEY_MINUS;
	result[SDL_SCANCODE_SPACE] = Input::KEY_SPACE;
	result[SDL_SCANCODE_RETURN] = Input::KEY_RETURN;
	result[SDL_SCANCODE_BACKSPACE] = Input::KEY_BACKSPACE;
	result[SDL_SCANCODE_TAB] = Input::KEY_TAB;
	result[SDL_SCANCODE_PAGEUP] = Input::KEY_PAGE_UP;
	result[SDL_SCANCODE_PAGEDOWN] = Input::KEY_PAGE_DOWN;
	result[SDL_SCANCODE_END] = Input::KEY_END;
	result[SDL_SCANCODE_HOME] = Input::KEY_HOME;
	result[SDL_SCANCODE_INSERT] = Input::KEY_INSERT;
	result[SDL_SCANCODE_DELETE] = Input::KEY_DELETE;
	result[SDL_SCANCODE_UP] = Input::KEY_ARROW_UP;
	result[SDL_SCANCODE_DOWN] = Input::KEY_ARROW_DOWN;
	result[SDL_SCANCODE_LEFT] = Input::KEY_ARROW_LEFT;
	result[SDL_SCANCODE_RIGHT] = Input::KEY_ARROW_RIGHT;
	result[SDL_SCANCODE_KP_PLUS] = Input::KEY_NUMPAD_PLUS;
	result[SDL_SCANCODE_KP_MINUS] = Input::KEY_NUMPAD_MINUS;
	result[SDL_SCANCODE_KP_MULTIPLY] = Input::KEY_NUMPAD_MULTIPLY;
	result[SDL_SCANCODE_KP_DIVIDE] = Input::KEY_NUMPAD_DIVIDE;
	result[SDL_SCANCODE_KP_1] = Input::KEY_NUMPAD_1;
	result[SDL_SCANCODE_KP_2] = Input::KEY_NUMPAD_2;
	result[SDL_SCANCODE_KP_3] = Input::KEY_NUMPAD_3;
	result[SDL_SCANCODE_KP_4] = Input::KEY_NUMPAD_4;
	result[SDL_SCANCODE_KP_5] = Input::KEY_NUMPAD_5;
	result[SDL_SCANCODE_KP_6] = Input::KEY_NUMPAD_6;
	result[SDL_SCANCODE_KP_7] = Input::KEY_NUMPAD_7;
	result[SDL_SCANCODE_KP_8] = Input::KEY_NUMPAD_8;
	result[SDL_SCANCODE_KP_9] = Input::KEY_NUMPAD_9;
	result[SDL_SCANCODE_KP_0] = Input::KEY_NUMPAD_0;
	result[SDL_SCANCODE_F1] = Input::KEY_F1;
	result[SDL_SCANCODE_F2] = Input::KEY_F2;
	result[SDL_SCANCODE_F3] = Input::KEY_F3;
	result[SDL_SCANCODE_F4] = Input::KEY_F4;
	result[SDL_SCANCODE_F5] = Input::KEY_F5;
	result[SDL_SCANCODE_F6] = Input::KEY_F6;
	result[SDL_SCANCODE_F7] = Input::KEY_F7;
	result[SDL_SCANCODE_F8] = Input::KEY_F8;
	result[SDL_SCANCODE_F9] = Input::KEY_F9;
	result[SDL_SCANCODE_F10] = Input::KEY_F10;
	result[SDL_SCANCODE_F11] = Input::KEY_F11;
	result[SDL_SCANCODE_F12] = Input::KEY_F12;
	result[SDL_SCANCODE_F13] = Input::KEY_F13;
	result[SDL_SCANCODE_F14] = Input::KEY_F14;
	result[SDL_SCANCODE_F15] = Input::KEY_F15;
	result[SDL_SCANCODE_PRINTSCREEN] = Input::KEY_PRINT_SCREEN;
	result[SDL_SCANCODE_SCROLLLOCK] = Input::KEY_SCROLL_LOCK;
	result[SDL_SCANCODE_PAUSE] = Input::KEY_PAUSE;
	return result;
}();

constexpr std::array<Input, std::numeric_limits<Uint8>::max()> MOUSE_BUTTON_MAP = [] {
	std::array<Input, std::numeric_limits<Uint8>::max()> result{};
	result[SDL_BUTTON_LEFT] = Input::MOUSE_BUTTON_LEFT;
	result[SDL_BUTTON_RIGHT] = Input::MOUSE_BUTTON_RIGHT;
	result[SDL_BUTTON_MIDDLE] = Input::MOUSE_BUTTON_MIDDLE;
	result[SDL_BUTTON_X1] = Input::MOUSE_BUTTON_BACK;
	result[SDL_BUTTON_X2] = Input::MOUSE_BUTTON_FORWARD;
	return result;
}();

constexpr std::array<Input, SDL_CONTROLLER_BUTTON_MAX> CONTROLLER_BUTTON_MAP = [] {
	std::array<Input, SDL_CONTROLLER_BUTTON_MAX> result{};
	result[SDL_CONTROLLER_BUTTON_A] = Input::CONTROLLER_BUTTON_A;
	result[SDL_CONTROLLER_BUTTON_B] = Input::CONTROLLER_BUTTON_B;
	result[SDL_CONTROLLER_BUTTON_X] = Input::CONTROLLER_BUTTON_X;
	result[SDL_CONTROLLER_BUTTON_Y] = Input::CONTROLLER_BUTTON_Y;
	result[SDL_CONTROLLER_BUTTON_BACK] = Input::CONTROLLER_BUTTON_BACK;
	result[SDL_CONTROLLER_BUTTON_GUIDE] = Input::CONTROLLER_BUTTON_GUIDE;
	result[SDL_CONTROLLER_BUTTON_START] = Input::CONTROLLER_BUTTON_START;
	result[SDL_CONTROLLER_BUTTON_LEFTSTICK] = Input::CONTROLLER_BUTTON_LEFT_STICK;
	result[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = Input::CONTROLLER_BUTTON_RIGHT_STICK;
	result[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = Input::CONTROLLER_BUTTON_LEFT_SHOULDER;
	result[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = Input::CONTROLLER_BUTTON_RIGHT_SHOULDER;
	result[SDL_CONTROLLER_BUTTON_DPAD_UP] = Input::CONTROLLER_BUTTON_DPAD_UP;
	result[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = Input::CONTROLLER_BUTTON_DPAD_DOWN;
	result[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = Input::CONTROLLER_BUTTON_DPAD_LEFT;
	result[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = Input::CONTROLLER_BUTTON_DPAD_RIGHT;
	result[SDL_CONTROLLER_BUTTON_MISC1] = Input::CONTROLLER_BUTTON_MISC1;
	result[SDL_CONTROLLER_BUTTON_PADDLE1] = Input::CONTROLLER_BUTTON_PADDLE1;
	result[SDL_CONTROLLER_BUTTON_PADDLE2] = Input::CONTROLLER_BUTTON_PADDLE2;
	result[SDL_CONTROLLER_BUTTON_PADDLE3] = Input::CONTROLLER_BUTTON_PADDLE3;
	result[SDL_CONTROLLER_BUTTON_PADDLE4] = Input::CONTROLLER_BUTTON_PADDLE4;
	result[SDL_CONTROLLER_BUTTON_TOUCHPAD] = Input::CONTROLLER_BUTTON_TOUCHPAD;
	return result;
}();

[[nodiscard]] std::string consumeText(char* text) {
	std::string result{};
	if (text) {
		try {
			result = text;
		} catch (...) {
			SDL_free(text);
			throw;
		}
		SDL_free(text);
	}
	return result;
}

[[nodiscard]] std::optional<Event> translateEvent(const SDL_Event& event) {
	std::optional<Event> result{};
	switch (event.type) {
		case SDL_QUIT: result = Event{ApplicationQuitRequestedEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_APP_TERMINATING: result = Event{ApplicationTerminatingEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_APP_LOWMEMORY: result = Event{ApplicationLowMemoryEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_APP_WILLENTERBACKGROUND: result = Event{ApplicationPausingEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_APP_DIDENTERBACKGROUND: result = Event{ApplicationPausedEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_APP_WILLENTERFOREGROUND: result = Event{ApplicationUnpausingEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_APP_DIDENTERFOREGROUND: result = Event{ApplicationUnpausedEvent{ApplicationEventBase{EventBase{}}}}; break;
		case SDL_WINDOWEVENT: {
			const WindowEventBase windowEventBase{EventBase{}, event.window.windowID};
			switch (event.window.event) {
				case SDL_WINDOWEVENT_SHOWN: result = Event{WindowShownEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_HIDDEN: result = Event{WindowHiddenEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_EXPOSED: result = Event{WindowExposedEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_MOVED: result = Event{WindowMovedEvent{windowEventBase, {event.window.data1, event.window.data2}}}; break;
				case SDL_WINDOWEVENT_RESIZED: result = Event{WindowResizedEvent{windowEventBase, {event.window.data1, event.window.data2}}}; break;
				case SDL_WINDOWEVENT_SIZE_CHANGED: result = Event{WindowSizeChangedEvent{windowEventBase, {event.window.data1, event.window.data2}}}; break;
				case SDL_WINDOWEVENT_MINIMIZED: result = Event{WindowMinimizedEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_MAXIMIZED: result = Event{WindowMaximizedEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_RESTORED: result = Event{WindowRestoredEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_ENTER: result = Event{WindowMouseFocusGainedEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_LEAVE: result = Event{WindowMouseFocusLostEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_FOCUS_GAINED: result = Event{WindowKeyboardFocusGainedEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_FOCUS_LOST: result = Event{WindowKeyboardFocusLostEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_CLOSE: result = Event{WindowCloseRequestedEvent{windowEventBase}}; break;
				case SDL_WINDOWEVENT_DISPLAY_CHANGED: result = Event{WindowDisplayChangedEvent{windowEventBase, event.window.data1}}; break;
				default: break;
			}
			break;
		}
		case SDL_KEYDOWN:
			if (event.key.repeat == 0) {
				result = Event{KeyPressedEvent{
					KeyEventBase{InputEventBase{EventBase{}, event.key.windowID}, KEY_SCANCODE_MAP[event.key.keysym.scancode], event.key.keysym.sym, event.key.keysym.mod}}};
			} else {
				result = Event{KeyPressRepeatedEvent{
					KeyEventBase{InputEventBase{EventBase{}, event.key.windowID}, KEY_SCANCODE_MAP[event.key.keysym.scancode], event.key.keysym.sym, event.key.keysym.mod}}};
			}
			break;
		case SDL_KEYUP:
			result = Event{KeyReleasedEvent{
				KeyEventBase{InputEventBase{EventBase{}, event.key.windowID}, KEY_SCANCODE_MAP[event.key.keysym.scancode], event.key.keysym.sym, event.key.keysym.mod}}};
			break;
		case SDL_TEXTEDITING:
			result = Event{TextInputEditedEvent{TextInputEventBase{InputEventBase{EventBase{}, event.edit.windowID}, event.edit.text}, event.edit.start, event.edit.length}};
			break;
		case SDL_TEXTEDITING_EXT: {
			std::string text = consumeText(event.editExt.text);
			result =
				Event{TextInputEditedEvent{TextInputEventBase{InputEventBase{EventBase{}, event.editExt.windowID}, std::move(text)}, event.editExt.start, event.editExt.length}};
			break;
		}
		case SDL_TEXTINPUT: result = Event{TextInputSubmittedEvent{TextInputEventBase{InputEventBase{EventBase{}, event.text.windowID}, event.text.text}}}; break;
		case SDL_MOUSEMOTION:
			result = Event{MouseMovedEvent{
				MouseEventBase{InputEventBase{EventBase{}, event.motion.windowID}, event.motion.which, {event.motion.x, event.motion.y}, {event.motion.xrel, event.motion.yrel}}}};
			break;
		case SDL_MOUSEBUTTONDOWN:
			result = Event{MouseButtonPressedEvent{
				MouseButtonEventBase{MouseEventBase{InputEventBase{EventBase{}, event.button.windowID}, event.button.which, {event.button.x, event.button.y}, {0, 0}},
					MOUSE_BUTTON_MAP[event.button.button], event.button.clicks}}};
			break;
		case SDL_MOUSEBUTTONUP:
			result = Event{MouseButtonReleasedEvent{
				MouseButtonEventBase{MouseEventBase{InputEventBase{EventBase{}, event.button.windowID}, event.button.which, {event.button.x, event.button.y}, {0, 0}},
					MOUSE_BUTTON_MAP[event.button.button], event.button.clicks}}};
			break;
		case SDL_MOUSEWHEEL: {
			const int direction = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1;
			result = Event{
				MouseWheelScrolledEvent{MouseEventBase{InputEventBase{EventBase{}, event.wheel.windowID}, event.wheel.which, {event.wheel.mouseX, event.wheel.mouseY}, {0, 0}},
					{event.wheel.x * direction, event.wheel.y * direction}, {event.wheel.preciseX, event.wheel.preciseY}}};
			break;
		}
		case SDL_CONTROLLERDEVICEADDED: result = Event{ControllerAddedEvent{ControllerEventBase{InputEventBase{EventBase{}, 0}, event.cdevice.which}}}; break;
		case SDL_CONTROLLERDEVICEREMOVED: result = Event{ControllerRemovedEvent{ControllerEventBase{InputEventBase{EventBase{}, 0}, event.cdevice.which}}}; break;
		case SDL_CONTROLLERDEVICEREMAPPED: result = Event{ControllerRemappedEvent{ControllerEventBase{InputEventBase{EventBase{}, 0}, event.cdevice.which}}}; break;
		case SDL_CONTROLLERAXISMOTION:
			result = Event{ControllerAxisMovedEvent{ControllerAxisEventBase{ControllerEventBase{InputEventBase{EventBase{}, 0}, event.caxis.which},
				static_cast<ControllerAxisEventBase::ControllerAxis>(event.caxis.axis), event.caxis.value}}};
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			result = Event{ControllerButtonPressedEvent{
				ControllerButtonEventBase{ControllerEventBase{InputEventBase{EventBase{}, 0}, event.cbutton.which}, CONTROLLER_BUTTON_MAP[event.cbutton.button]}}};
			break;
		case SDL_CONTROLLERBUTTONUP:
			result = Event{ControllerButtonReleasedEvent{
				ControllerButtonEventBase{ControllerEventBase{InputEventBase{EventBase{}, 0}, event.cbutton.which}, CONTROLLER_BUTTON_MAP[event.cbutton.button]}}};
			break;
		case SDL_FINGERMOTION:
			result = Event{TouchMovedEvent{TouchEventBase{InputEventBase{EventBase{}, event.tfinger.windowID}, event.tfinger.touchId, event.tfinger.fingerId,
				{event.tfinger.x, event.tfinger.y}, {event.tfinger.dx, event.tfinger.dy}, event.tfinger.pressure}}};
			break;
		case SDL_FINGERDOWN:
			result = Event{TouchPressedEvent{TouchEventBase{InputEventBase{EventBase{}, event.tfinger.windowID}, event.tfinger.touchId, event.tfinger.fingerId,
				{event.tfinger.x, event.tfinger.y}, {event.tfinger.dx, event.tfinger.dy}, event.tfinger.pressure}}};
			break;
		case SDL_FINGERUP:
			result = Event{TouchReleasedEvent{TouchEventBase{InputEventBase{EventBase{}, event.tfinger.windowID}, event.tfinger.touchId, event.tfinger.fingerId,
				{event.tfinger.x, event.tfinger.y}, {event.tfinger.dx, event.tfinger.dy}, event.tfinger.pressure}}};
			break;
		case SDL_KEYMAPCHANGED: result = Event{KeymapChangedEvent{KeymapEventBase{EventBase{}}}}; break;
		case SDL_CLIPBOARDUPDATE: result = Event{ClipboardUpdatedEvent{ClipboardEventBase{EventBase{}}}}; break;
		case SDL_DROPFILE: {
			std::string droppedFilepath = consumeText(event.drop.file);
			result = Event{DropFileEvent{DropEventBase{EventBase{}, event.drop.windowID}, std::move(droppedFilepath)}};
			break;
		}
		case SDL_DROPTEXT: {
			std::string droppedText = consumeText(event.drop.file);
			result = Event{DropTextEvent{DropEventBase{EventBase{}, event.drop.windowID}, std::move(droppedText)}};
			break;
		}
		case SDL_DROPBEGIN: result = Event{DropStartedEvent{DropEventBase{EventBase{}, event.drop.windowID}}}; break;
		case SDL_DROPCOMPLETE: result = Event{DropCompletedEvent{DropEventBase{EventBase{}, event.drop.windowID}}}; break;
		default: break;
	}
	return result;
}

} // namespace

EventPump::EventPump()
	: events{} { // NOLINT(readability-redundant-member-init)
	if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0) {
		throw Error{std::format("Failed to initialize SDL events subsystem:\n{}", SDL_GetError())};
	}
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
}

EventPump::~EventPump() {
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

std::span<const Event> EventPump::pollEvents() {
	events.clear();
	for (SDL_Event event{}; SDL_PollEvent(&event) != 0;) {
		if (std::optional<Event> translated = translateEvent(event)) {
			events.push_back(std::move(*translated));
		}
	}
	return events;
}

void EventPump::setRelativeMouseMode(bool relativeMouseMode) {
	if (SDL_SetRelativeMouseMode((relativeMouseMode) ? SDL_TRUE : SDL_FALSE) != 0) {
		throw Error{std::format("Failed to set relative mouse mode:\n{}", SDL_GetError())};
	}
}

void EventPump::setTextInputRectangle(ivec2 offset, ivec2 size) {
	const SDL_Rect rect{
		.x = offset.x,
		.y = offset.y,
		.w = size.x,
		.h = size.y,
	};
	SDL_SetTextInputRect(&rect);
}

void EventPump::startTextInput() {
	SDL_StartTextInput();
}

void EventPump::stopTextInput() {
	SDL_StopTextInput();
}

std::span<const Event> EventPump::getLatestPolledEvents() const noexcept {
	return events;
}

std::string EventPump::getClipboardText() const {
	return consumeText(SDL_GetClipboardText());
}

bool EventPump::hasScreenKeyboardSupport() const noexcept {
	return SDL_HasScreenKeyboardSupport() == SDL_TRUE;
}

} // namespace donut::events
