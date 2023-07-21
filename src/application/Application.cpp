#include <donut/Time.hpp>
#include <donut/application/Application.hpp>
#include <donut/application/Error.hpp>
#include <donut/application/Event.hpp>
#include <donut/application/Input.hpp>

//
#include <SDL.h> // SDL..., Uint8, Uint32, Uint64
//
#ifdef __EMSCRIPTEN__
#include <emscripten.h> // emscripten_...
#endif
//

#include <algorithm>   // std::min
#include <array>       // std::array
#include <cmath>       // std::ceil
#include <cstdio>      // stderr, std::fprintf
#include <exception>   // std::exception
#include <format>      // std::format
#include <glm/glm.hpp> // glm::...
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional
#include <physfs.h>    // PHYSFS_...
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::move

namespace donut {
namespace application {

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

void Application::showSimpleMessageBox(MessageBoxType type, const char* title, const char* message) {
	Uint32 flags = 0;
	switch (type) {
		case MessageBoxType::ERROR_MESSAGE: flags = SDL_MESSAGEBOX_ERROR; break;
		case MessageBoxType::WARNING_MESSAGE: flags = SDL_MESSAGEBOX_WARNING; break;
		case MessageBoxType::INFO_MESSAGE: flags = SDL_MESSAGEBOX_INFORMATION; break;
	}
	if (SDL_ShowSimpleMessageBox(flags, title, message, nullptr) != 0) {
		throw std::runtime_error{std::format("Failed to show simple message box: {}", SDL_GetError())};
	}
}

Application::Application(const char* programFilepath, const ApplicationOptions& options)
	: physFSManager(programFilepath, options.organizationName, options.applicationName, options.dataDirectoryFilepath, options.archiveFilenameExtension)
	, clockFrequency(SDL_GetPerformanceFrequency())
	, clockInterval(1.0f / static_cast<float>(clockFrequency)) {
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
	setFrameRateParameters(options.tickRate, options.minFps, options.maxFps);
}

void Application::run() {
	startClockTime = SDL_GetPerformanceCounter();
	latestFrameClockTime = startClockTime;
	latestFpsMeasurementClockTime = startClockTime;
	processedTickClockTime = startClockTime;
	latestMeasuredFps = 0u;
	fpsCounter = 0u;
	frameInfo.tickInfo.processedTickCount = 0u;
	frameInfo.tickInfo.processedTickTime = {};
	frameInfo.tickInterpolationAlpha = 0.0f;
	frameInfo.elapsedTime = {};
	frameInfo.deltaTime = {};
	running = true;

#ifdef __EMSCRIPTEN__
	constexpr auto run_emscripten_frame = [](void* arg) -> void {
		Application* const application = static_cast<Application*>(arg);
		try {
			application->runFrame();
		} catch (const std::exception& e) {
			std::fprintf(stderr, "Fatal error: %s\n", e.what());
			application->quit();
		} catch (...) {
			std::fprintf(stderr, "Fatal error!\n");
			application->quit();
		}
		if (!application->isRunning()) {
			application->~Application();
			emscripten_cancel_main_loop();
		}
	};
	emscripten_set_main_loop_arg(run_emscripten_frame, this, 0, 1);
#else
	while (isRunning()) {
		try {
			runFrame();
		} catch (...) {
			quit();
			throw;
		}
	}
#endif
}

void Application::quit() noexcept {
	running = false;
}

bool Application::isRunning() const noexcept {
	return running;
}

unsigned Application::getLatestMeasuredFps() const noexcept {
	return latestMeasuredFps;
}

std::string Application::getClipboardText() const {
	return consumeText(SDL_GetClipboardText());
}

bool Application::hasScreenKeyboardSupport() const noexcept {
	return SDL_HasScreenKeyboardSupport() == SDL_TRUE;
}

void Application::setFrameRateParameters(float tickRate, float minFps, float maxFps) {
	tickClockInterval = static_cast<Uint64>(std::ceil(static_cast<float>(clockFrequency) / tickRate));
	frameInfo.tickInfo.tickInterval = Time<float>::Duration{static_cast<float>(tickClockInterval) * clockInterval};
	minFrameClockInterval = (maxFps == 0.0f) ? 0 : static_cast<Uint64>(std::ceil(static_cast<float>(clockFrequency) / maxFps));
	maxTicksPerFrame = (minFps <= 0.0f || tickRate <= minFps) ? 1 : static_cast<Uint64>(tickRate / minFps);
}

void Application::setTextInputRectangle(glm::ivec2 offset, glm::ivec2 size) {
	const SDL_Rect rect{
		.x = offset.x,
		.y = offset.y,
		.w = size.x,
		.h = size.y,
	};
	SDL_SetTextInputRect(&rect);
}

void Application::startTextInput() {
	SDL_StartTextInput();
}

void Application::stopTextInput() {
	SDL_StopTextInput();
}

void Application::runFrame() {
	const Uint64 currentClockTime = SDL_GetPerformanceCounter();
	const Uint64 clockDeltaTime = currentClockTime - latestFrameClockTime;
	if (currentClockTime > latestFrameClockTime && clockDeltaTime >= minFrameClockInterval) {
		latestFrameClockTime = currentClockTime;
		++fpsCounter;
		if (currentClockTime - latestFpsMeasurementClockTime >= clockFrequency) {
			latestFpsMeasurementClockTime = currentClockTime;
			latestMeasuredFps = fpsCounter;
			fpsCounter = 0;
		}

		frameInfo.elapsedTime = Time<float>::Duration{static_cast<float>(currentClockTime - startClockTime) * clockInterval};
		frameInfo.deltaTime = Time<float>::Duration{static_cast<float>(clockDeltaTime) * clockInterval};

		prepareForEvents(frameInfo);
		for (SDL_Event event{}; SDL_PollEvent(&event) != 0;) {
			if (event.type == SDL_QUIT) {
				quit();
				return;
			}
			if (const std::optional<Event> translatedEvent = translateEvent(event)) {
				handleEvent(frameInfo, *translatedEvent);
			}
		}

		update(frameInfo);
		const Uint64 clockTimeSinceLatestTick = currentClockTime - processedTickClockTime;
		for (Uint64 ticksToProcess = std::min(clockTimeSinceLatestTick / tickClockInterval, maxTicksPerFrame); ticksToProcess > 0; --ticksToProcess) {
			tick(frameInfo.tickInfo);
			++frameInfo.tickInfo.processedTickCount;
			frameInfo.tickInfo.processedTickTime += frameInfo.tickInfo.tickInterval;
			processedTickClockTime += tickClockInterval;
		}

		frameInfo.tickInterpolationAlpha = std::min(1.0f, (static_cast<float>(currentClockTime - processedTickClockTime) * clockInterval) / frameInfo.tickInfo.tickInterval);

		display(frameInfo);
	}
}

Application::PhysFSManager::PhysFSManager(const char* programFilepath, const char* organizationName, const char* applicationName, const char* dataDirectoryFilepath,
	const char* archiveFilenameExtension) {
	struct Context {
		[[nodiscard]] Context(const char* programFilepath, const char* organizationName, const char* applicationName, const char* dataDirectoryFilepath,
			const char* archiveFilenameExtension) {
			if (PHYSFS_init(programFilepath) == 0) {
				throw Error{std::format("Failed to initialize PhysicsFS: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
			}

#ifndef __EMSCRIPTEN__
			if (organizationName && applicationName) {
				const char* const prefDir = PHYSFS_getPrefDir(organizationName, applicationName);
				if (!prefDir) {
					throw Error{std::format("Failed to get application preferences directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
				}

				if (PHYSFS_setWriteDir(prefDir) == 0) {
					throw Error{std::format("Failed to set application write directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
				}

				if (PHYSFS_mount(prefDir, nullptr, 0) == 0) {
					throw Error{std::format("Failed to mount application write directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
				}
			}
#endif

			if (dataDirectoryFilepath) {
				if (PHYSFS_mount(dataDirectoryFilepath, nullptr, 1) == 0) {
					throw Error{std::format("Failed to set application data directory: {}", PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()))};
				}
			}

			if (archiveFilenameExtension) {
				PHYSFS_enumerate(
					"/",
					[](void* data, const char*, const char* fname) -> PHYSFS_EnumerateCallbackResult {
						const std::string_view archiveFilenameExtension{static_cast<const char*>(data)};
						const std::string_view filename{fname};
						if (!archiveFilenameExtension.empty() && filename.size() > archiveFilenameExtension.size() &&
							(archiveFilenameExtension.front() == '.' || filename[filename.size() - archiveFilenameExtension.size() - 1] == '.') &&
							filename.ends_with(archiveFilenameExtension)) {
							if (const char* directory = PHYSFS_getRealDir(fname)) {
								const std::string archiveFilepath = std::format("{}{}{}", directory, PHYSFS_getDirSeparator(), fname);
								if (PHYSFS_mount(archiveFilepath.c_str(), nullptr, 0) == 0) {
									std::fprintf(stderr, "Failed to mount archive \"%s\": %s\n", archiveFilepath.c_str(), PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
								}
							} else {
								std::fprintf(stderr, "Failed to get the real directory of archive \"%s\": %s\n", fname, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
							}
						}
						return PHYSFS_ENUM_OK;
					},
					const_cast<char*>(archiveFilenameExtension));
			}
		}

		~Context() {
			PHYSFS_deinit();
		}

		Context(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = delete;
	};
	static const Context context{programFilepath, organizationName, applicationName, dataDirectoryFilepath, archiveFilenameExtension};
}

Application::SDLManager::SDLManager() {
	struct Context {
		[[nodiscard]] Context() {
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
				throw Error{std::format("Failed to initialize SDL: {}", SDL_GetError())};
			}
		}

		~Context() {
			SDL_Quit();
		}

		Context(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = delete;
	};
	static const Context context{};
}

} // namespace application
} // namespace donut
