#include <donut/application/Error.hpp>
#include <donut/application/Input.hpp>
#include <donut/application/InputManager.hpp>

#include <SDL.h>        // SDL...
#include <array>        // std::array
#include <cstddef>      // std::size_t
#include <fmt/format.h> // fmt::format
#include <glm/glm.hpp>  // glm::...
#include <optional>     // std::optional
#include <vector>       // std::vector

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

constexpr float DIAGONAL_RATIO = 0.41421356237f; // sqrt(2) - 1

[[nodiscard]] glm::i32 getIntegerValue(float value) noexcept {
	return static_cast<glm::i32>(glm::floor(value * 32767.5f));
}

[[nodiscard]] float getFloatValue(glm::i32 value) noexcept {
	return (static_cast<float>(value) + 0.5f) / 32767.5f;
}

} // namespace

InputManager::InputManager(const InputManagerOptions& options)
	: mouseSensitivity(options.mouseSensitivity)
	, controllerLeftStickSensitivity(options.controllerLeftStickSensitivity)
	, controllerRightStickSensitivity(options.controllerRightStickSensitivity)
	, controllerLeftStickDeadzone(options.controllerLeftStickDeadzone)
	, controllerRightStickDeadzone(options.controllerRightStickDeadzone)
	, controllerLeftTriggerDeadzone(options.controllerLeftTriggerDeadzone)
	, controllerRightTriggerDeadzone(options.controllerRightTriggerDeadzone) {
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
		throw Error{fmt::format("Failed to initialize SDL GameController subsystem: {}", SDL_GetError())};
	}
}

InputManager::~InputManager() {
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void InputManager::prepareForEvents() {
	previousPersistentOutputs = currentPersistentOutputs;
	transientOutputs = {};
	outputRelativeValues.fill(0);
	previousPersistentInputs = currentPersistentInputs;
	transientInputs = {};
	mouseTransientMotion = false;
	mouseWheelHorizontalTransientMotion = false;
	mouseWheelVerticalTransientMotion = false;
	controllerLeftStickTransientMotion = false;
	controllerRightStickTransientMotion = false;
	controllerLeftTriggerTransientMotion = false;
	controllerRightTriggerTransientMotion = false;
}

void InputManager::handleEvent(const Event& event) {
	switch (event.type) {
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_FOCUS_LOST: resetAllInputs(); break;
				case SDL_WINDOWEVENT_LEAVE: mousePosition.reset(); break;
				default: break;
			}
			break;
		case SDL_KEYDOWN:
			if (event.key.repeat == 0) {
				press(KEY_SCANCODE_MAP[event.key.keysym.scancode]);
			}
			break;
		case SDL_KEYUP: release(KEY_SCANCODE_MAP[event.key.keysym.scancode]); break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event.button.button) {
				case SDL_BUTTON_LEFT: press(Input::MOUSE_BUTTON_LEFT); break;
				case SDL_BUTTON_RIGHT: press(Input::MOUSE_BUTTON_RIGHT); break;
				case SDL_BUTTON_MIDDLE: press(Input::MOUSE_BUTTON_MIDDLE); break;
				case SDL_BUTTON_X1: press(Input::MOUSE_BUTTON_BACK); break;
				case SDL_BUTTON_X2: press(Input::MOUSE_BUTTON_FORWARD); break;
				default: break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (event.button.button) {
				case SDL_BUTTON_LEFT: release(Input::MOUSE_BUTTON_LEFT); break;
				case SDL_BUTTON_RIGHT: release(Input::MOUSE_BUTTON_RIGHT); break;
				case SDL_BUTTON_MIDDLE: release(Input::MOUSE_BUTTON_MIDDLE); break;
				case SDL_BUTTON_X1: release(Input::MOUSE_BUTTON_BACK); break;
				case SDL_BUTTON_X2: release(Input::MOUSE_BUTTON_FORWARD); break;
				default: break;
			}
			break;
		case SDL_MOUSEMOTION: setMousePosition({static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)}); break;
		case SDL_MOUSEWHEEL:
			scrollMouseWheelHorizontally(event.wheel.preciseX);
			scrollMouseWheelVertically(-event.wheel.preciseY);
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			if (controller && event.cbutton.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				press(CONTROLLER_BUTTON_MAP[event.cbutton.button]);
			}
			break;
		case SDL_CONTROLLERBUTTONUP:
			if (controller && event.cbutton.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				release(CONTROLLER_BUTTON_MAP[event.cbutton.button]);
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
			if (controller && event.caxis.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				switch (event.caxis.axis) {
					case SDL_CONTROLLER_AXIS_LEFTX:
						setControllerLeftStickPosition({getFloatValue(event.caxis.value), (controllerLeftStickPosition) ? controllerLeftStickPosition->y : 0});
						break;
					case SDL_CONTROLLER_AXIS_LEFTY:
						setControllerLeftStickPosition({(controllerLeftStickPosition) ? controllerLeftStickPosition->x : 0, getFloatValue(event.caxis.value)});
						break;
					case SDL_CONTROLLER_AXIS_RIGHTX:
						setControllerRightStickPosition({getFloatValue(event.caxis.value), (controllerRightStickPosition) ? controllerRightStickPosition->y : 0});
						break;
					case SDL_CONTROLLER_AXIS_RIGHTY:
						setControllerRightStickPosition({(controllerRightStickPosition) ? controllerRightStickPosition->x : 0, getFloatValue(event.caxis.value)});
						break;
					case SDL_CONTROLLER_AXIS_TRIGGERLEFT: setControllerLeftTriggerPosition(getFloatValue(event.caxis.value)); break;
					case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: setControllerRightTriggerPosition(getFloatValue(event.caxis.value)); break;
					default: break;
				}
			}
			break;
		case SDL_CONTROLLERDEVICEADDED:
			if (controller) {
				controller.reset(SDL_GameControllerOpen(event.cdevice.which));
				resetAllInputs();
			} else {
				controller.reset(SDL_GameControllerOpen(event.cdevice.which));
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			if (controller && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				controller.reset();
				resetAllInputs();
			}
			break;
		default: break;
	}
}

void InputManager::bind(Input input, Outputs outputs) {
	bindings.insert_or_assign(input, outputs);
}

void InputManager::addBinding(Input input, Outputs outputs) {
	if (const auto [it, inserted] = bindings.emplace(input, outputs); !inserted) {
		it->second |= outputs;
	}
}

void InputManager::unbind(Input input) {
	bindings.erase(input);
}

void InputManager::unbindAll() noexcept {
	bindings.clear();
}

void InputManager::press(Input input, glm::i32 offset) noexcept {
	const std::size_t inputIndex = getInputIndex(input);
	const bool wasReleased = !currentPersistentInputs.test(inputIndex);
	currentPersistentInputs.set(inputIndex);
	if (const auto it = bindings.find(input); it != bindings.end()) {
		const Outputs outputs = it->second;
		currentPersistentOutputs |= outputs;
		for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
			if (outputs.test(i)) {
				outputRelativeValues[i] += offset;
				outputAbsoluteValues[i] += offset;
				if (wasReleased) {
					++outputPersistentPresses[i];
				}
			}
		}
	}
}

void InputManager::release(Input input, glm::i32 offset) noexcept {
	const std::size_t inputIndex = getInputIndex(input);
	const bool wasPressed = currentPersistentInputs.test(inputIndex);
	currentPersistentInputs.set(inputIndex, false);
	if (const auto it = bindings.find(input); it != bindings.end()) {
		const Outputs outputs = it->second;
		for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
			if (outputs.test(i)) {
				outputRelativeValues[i] += offset;
				outputAbsoluteValues[i] += offset;
				if (wasPressed) {
					if (outputPersistentPresses[i] > 1) {
						--outputPersistentPresses[i];
					} else {
						outputPersistentPresses[i] = 0;
						currentPersistentOutputs.set(i, false);
					}
				}
			}
		}
	}
}

void InputManager::move(Input input, glm::i32 offset) noexcept {
	if (offset > 0) {
		const std::size_t inputIndex = getInputIndex(input);
		transientInputs.set(inputIndex);
		if (const auto it = bindings.find(input); it != bindings.end()) {
			const Outputs outputs = it->second;
			transientOutputs |= outputs;
			for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
				if (outputs.test(i)) {
					outputRelativeValues[i] += offset;
				}
			}
		}
	}
}

void InputManager::set(Input input, glm::i32 value) noexcept {
	if (const auto it = bindings.find(input); it != bindings.end()) {
		const Outputs outputs = it->second;
		for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
			if (outputs.test(i)) {
				outputAbsoluteValues[i] = value;
			}
		}
	}
}

void InputManager::resetAllInputs() noexcept {
	mousePosition = {};
	controllerLeftStickPosition = {};
	controllerRightStickPosition = {};
	currentPersistentOutputs = {};
	transientOutputs = {};
	outputAbsoluteValues.fill(0);
	outputRelativeValues.fill(0);
	outputPersistentPresses.fill(0);
	currentPersistentInputs = {};
	transientInputs = {};
}

void InputManager::setMouseSensitivity(float sensitivity) noexcept {
	mouseSensitivity = sensitivity;
}

void InputManager::setControllerLeftStickSensitivity(float sensitivity) noexcept {
	controllerLeftStickSensitivity = sensitivity;
}

void InputManager::setControllerRightStickSensitivity(float sensitivity) noexcept {
	controllerRightStickSensitivity = sensitivity;
}

void InputManager::setControllerLeftStickDeadzone(float deadzone) noexcept {
	controllerLeftStickDeadzone = deadzone;
}

void InputManager::setControllerRightStickDeadzone(float deadzone) noexcept {
	controllerRightStickDeadzone = deadzone;
}

void InputManager::setControllerLeftTriggerDeadzone(float deadzone) noexcept {
	controllerLeftTriggerDeadzone = deadzone;
}

void InputManager::setControllerRightTriggerDeadzone(float deadzone) noexcept {
	controllerRightTriggerDeadzone = deadzone;
}

bool InputManager::hasAnyBindings() const noexcept {
	return !bindings.empty();
}

std::vector<InputManager::Binding> InputManager::getBindings() const {
	std::vector<Binding> result{};
	result.reserve(bindings.size());
	for (const auto& [input, outputs] : bindings) {
		result.push_back(Binding{.input = input, .outputs = outputs});
	}
	return result;
}

std::optional<InputManager::Outputs> InputManager::findBinding(Input input) const noexcept {
	if (const auto it = bindings.find(input); it != bindings.end()) {
		return it->second;
	}
	return {};
}

std::optional<glm::vec2> InputManager::getMousePosition() const noexcept {
	return mousePosition;
}

bool InputManager::mouseJustMoved() const noexcept {
	return mouseTransientMotion;
}

bool InputManager::mouseWheelJustScrolledHorizontally() const noexcept {
	return mouseWheelHorizontalTransientMotion;
}

bool InputManager::mouseWheelJustScrolledVertically() const noexcept {
	return mouseWheelVerticalTransientMotion;
}

bool InputManager::isControllerConnected() const noexcept {
	return static_cast<bool>(controller);
}

std::optional<glm::vec2> InputManager::getControllerLeftStickPosition() const noexcept {
	return controllerLeftStickPosition;
}

std::optional<glm::vec2> InputManager::getControllerRightStickPosition() const noexcept {
	return controllerRightStickPosition;
}

std::optional<float> InputManager::getControllerLeftTriggerPosition() const noexcept {
	return controllerLeftTriggerPosition;
}

std::optional<float> InputManager::getControllerRightTriggerPosition() const noexcept {
	return controllerRightTriggerPosition;
}

bool InputManager::controllerLeftStickJustMoved() const noexcept {
	return controllerLeftStickTransientMotion;
}

bool InputManager::controllerRightStickJustMoved() const noexcept {
	return controllerRightStickTransientMotion;
}

bool InputManager::controllerLeftTriggerJustMoved() const noexcept {
	return controllerLeftTriggerTransientMotion;
}

bool InputManager::controllerRightTriggerJustMoved() const noexcept {
	return controllerRightTriggerTransientMotion;
}

InputManager::Outputs InputManager::getCurrentOutputs() const noexcept {
	return currentPersistentOutputs;
}

InputManager::Outputs InputManager::getPreviousOutputs() const noexcept {
	return previousPersistentOutputs;
}

InputManager::Outputs InputManager::getJustPressedOutputs() const noexcept {
	return transientOutputs | (currentPersistentOutputs & ~previousPersistentOutputs);
}

InputManager::Outputs InputManager::getJustReleasedOutputs() const noexcept {
	return transientOutputs | (previousPersistentOutputs & ~currentPersistentOutputs);
}

bool InputManager::isPressed(std::size_t output) const noexcept {
	return getCurrentOutputs().test(output);
}

bool InputManager::justPressed(std::size_t output) const noexcept {
	return getJustPressedOutputs().test(output);
}

bool InputManager::justReleased(std::size_t output) const noexcept {
	return getJustReleasedOutputs().test(output);
}

glm::i32 InputManager::getAbsoluteValue(std::size_t output) const noexcept {
	return outputAbsoluteValues[output];
}

glm::i32 InputManager::getRelativeValue(std::size_t output) const noexcept {
	return outputRelativeValues[output];
}

float InputManager::getAbsoluteVector(std::size_t outputPositive) const noexcept {
	return getFloatValue(glm::max(0, getAbsoluteValue(outputPositive)));
}

float InputManager::getRelativeVector(std::size_t outputPositive) const noexcept {
	return getFloatValue(glm::max(0, getRelativeValue(outputPositive)));
}

float InputManager::getAbsoluteVector(std::size_t outputNegative, std::size_t outputPositive) const noexcept {
	return getAbsoluteVector(outputPositive) - getAbsoluteVector(outputNegative);
}

float InputManager::getRelativeVector(std::size_t outputNegative, std::size_t outputPositive) const noexcept {
	return getRelativeVector(outputPositive) - getRelativeVector(outputNegative);
}

glm::vec2 InputManager::getAbsoluteVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept {
	return {
		getAbsoluteVector(outputNegativeX, outputPositiveX),
		getAbsoluteVector(outputNegativeY, outputPositiveY),
	};
}

glm::vec2 InputManager::getRelativeVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept {
	return {
		getRelativeVector(outputNegativeX, outputPositiveX),
		getRelativeVector(outputNegativeY, outputPositiveY),
	};
}

bool InputManager::isPressed(Input input) const noexcept {
	return currentPersistentInputs.test(getInputIndex(input));
}

bool InputManager::justPressed(Input input) const noexcept {
	return (transientInputs | (currentPersistentInputs & ~previousPersistentInputs)).test(getInputIndex(input));
}

bool InputManager::justReleased(Input input) const noexcept {
	return (transientInputs | (previousPersistentInputs & ~currentPersistentInputs)).test(getInputIndex(input));
}

void InputManager::ControllerDeleter::operator()(void* handle) const noexcept {
	SDL_GameControllerClose(static_cast<SDL_GameController*>(handle));
}

void InputManager::setMousePosition(glm::vec2 position) noexcept {
	const glm::vec2 offset = (mousePosition) ? (position - *mousePosition) * mouseSensitivity : glm::vec2{0.0f, 0.0f};
	mousePosition = position;
	mouseTransientMotion = true;
	move(Input::MOUSE_MOTION_UP, getIntegerValue(-offset.y));
	move(Input::MOUSE_MOTION_DOWN, getIntegerValue(offset.y));
	move(Input::MOUSE_MOTION_LEFT, getIntegerValue(-offset.x));
	move(Input::MOUSE_MOTION_RIGHT, getIntegerValue(offset.x));
}

void InputManager::scrollMouseWheelHorizontally(float offset) noexcept {
	mouseWheelHorizontalTransientMotion = true;
	move(Input::MOUSE_SCROLL_LEFT, getIntegerValue(-offset));
	move(Input::MOUSE_SCROLL_RIGHT, getIntegerValue(offset));
}

void InputManager::scrollMouseWheelVertically(float offset) noexcept {
	mouseWheelVerticalTransientMotion = true;
	move(Input::MOUSE_SCROLL_UP, getIntegerValue(-offset));
	move(Input::MOUSE_SCROLL_DOWN, getIntegerValue(offset));
}

void InputManager::setControllerLeftStickPosition(glm::vec2 position) noexcept {
	const glm::vec2 oldPosition = controllerLeftStickPosition.value_or(glm::vec2{0.0f, 0.0f});
	const float oldLength = glm::length(oldPosition);
	const float length = glm::length(position);
	const glm::vec2 oldAdjustedPosition =
		(oldLength > controllerLeftStickDeadzone)
			? oldPosition * (controllerLeftStickSensitivity * ((oldLength - controllerLeftStickDeadzone) / (oldLength * (1.0f - controllerLeftStickDeadzone))))
			: glm::vec2{0.0f, 0.0f};
	const glm::vec2 adjustedPosition =
		(length > controllerLeftStickDeadzone)
			? position * (controllerLeftStickSensitivity * ((length - controllerLeftStickDeadzone) / (length * (1.0f - controllerLeftStickDeadzone))))
			: glm::vec2{0.0f, 0.0f};
	const glm::i32vec2 oldIntegerPosition{getIntegerValue(oldAdjustedPosition.x), getIntegerValue(oldAdjustedPosition.y)};
	const glm::i32vec2 integerPosition{getIntegerValue(adjustedPosition.x), getIntegerValue(adjustedPosition.y)};
	const glm::i32vec2 offset = integerPosition - oldIntegerPosition;
	const glm::i32vec2 clampedOffsetNegative = glm::max(glm::min(glm::i32vec2{0, 0}, oldIntegerPosition), glm::min(-offset, -offset - oldIntegerPosition));
	const glm::i32vec2 clampedOffsetPositive = glm::max(glm::min(glm::i32vec2{0, 0}, -oldIntegerPosition), glm::min(offset, offset + oldIntegerPosition));
	controllerLeftStickPosition = position;
	controllerLeftStickTransientMotion = true;
	if (length > controllerLeftStickDeadzone) {
		if (glm::abs(static_cast<float>(integerPosition.x) / static_cast<float>(integerPosition.y)) > DIAGONAL_RATIO) {
			if (integerPosition.x < 0) {
				press(Input::CONTROLLER_AXIS_LEFT_STICK_LEFT, clampedOffsetNegative.x);
				release(Input::CONTROLLER_AXIS_LEFT_STICK_RIGHT, clampedOffsetPositive.x);
			} else {
				release(Input::CONTROLLER_AXIS_LEFT_STICK_RIGHT, clampedOffsetNegative.x);
				press(Input::CONTROLLER_AXIS_LEFT_STICK_RIGHT, clampedOffsetPositive.x);
			}
		} else {
			release(Input::CONTROLLER_AXIS_LEFT_STICK_LEFT, clampedOffsetNegative.x);
			release(Input::CONTROLLER_AXIS_LEFT_STICK_RIGHT, clampedOffsetPositive.x);
		}
		if (glm::abs(static_cast<float>(integerPosition.y) / static_cast<float>(integerPosition.x)) > DIAGONAL_RATIO) {
			if (integerPosition.y < 0) {
				press(Input::CONTROLLER_AXIS_LEFT_STICK_UP, clampedOffsetNegative.y);
				release(Input::CONTROLLER_AXIS_LEFT_STICK_DOWN, clampedOffsetPositive.y);
			} else {
				release(Input::CONTROLLER_AXIS_LEFT_STICK_UP, clampedOffsetNegative.y);
				press(Input::CONTROLLER_AXIS_LEFT_STICK_DOWN, clampedOffsetPositive.y);
			}
		} else {
			release(Input::CONTROLLER_AXIS_LEFT_STICK_UP, clampedOffsetNegative.y);
			release(Input::CONTROLLER_AXIS_LEFT_STICK_DOWN, clampedOffsetPositive.y);
		}
	} else {
		release(Input::CONTROLLER_AXIS_LEFT_STICK_UP, clampedOffsetNegative.y);
		release(Input::CONTROLLER_AXIS_LEFT_STICK_DOWN, clampedOffsetPositive.y);
		release(Input::CONTROLLER_AXIS_LEFT_STICK_LEFT, clampedOffsetNegative.x);
		release(Input::CONTROLLER_AXIS_LEFT_STICK_RIGHT, clampedOffsetPositive.x);
	}
}

void InputManager::setControllerRightStickPosition(glm::vec2 position) noexcept {
	const glm::vec2 oldPosition = controllerRightStickPosition.value_or(glm::vec2{0.0f, 0.0f});
	const float oldLength = glm::length(oldPosition);
	const float length = glm::length(position);
	const glm::vec2 oldAdjustedPosition =
		(oldLength > controllerRightStickDeadzone)
			? oldPosition * (controllerRightStickSensitivity * ((oldLength - controllerRightStickDeadzone) / (oldLength * (1.0f - controllerRightStickDeadzone))))
			: glm::vec2{0.0f, 0.0f};
	const glm::vec2 adjustedPosition =
		(length > controllerRightStickDeadzone)
			? position * (controllerRightStickSensitivity * ((length - controllerRightStickDeadzone) / (length * (1.0f - controllerRightStickDeadzone))))
			: glm::vec2{0.0f, 0.0f};
	const glm::i32vec2 oldIntegerPosition{getIntegerValue(oldAdjustedPosition.x), getIntegerValue(oldAdjustedPosition.y)};
	const glm::i32vec2 integerPosition{getIntegerValue(adjustedPosition.x), getIntegerValue(adjustedPosition.y)};
	const glm::i32vec2 offset = integerPosition - oldIntegerPosition;
	const glm::i32vec2 clampedOffsetNegative = glm::max(glm::min(glm::i32vec2{0, 0}, oldIntegerPosition), glm::min(-offset, -offset - oldIntegerPosition));
	const glm::i32vec2 clampedOffsetPositive = glm::max(glm::min(glm::i32vec2{0, 0}, -oldIntegerPosition), glm::min(offset, offset + oldIntegerPosition));
	controllerRightStickPosition = position;
	controllerRightStickTransientMotion = true;
	if (length > controllerRightStickDeadzone) {
		if (glm::abs(static_cast<float>(integerPosition.x) / static_cast<float>(integerPosition.y)) > DIAGONAL_RATIO) {
			if (integerPosition.x < 0) {
				press(Input::CONTROLLER_AXIS_RIGHT_STICK_LEFT, clampedOffsetNegative.x);
				release(Input::CONTROLLER_AXIS_RIGHT_STICK_RIGHT, clampedOffsetPositive.x);
			} else {
				release(Input::CONTROLLER_AXIS_RIGHT_STICK_LEFT, clampedOffsetNegative.x);
				press(Input::CONTROLLER_AXIS_RIGHT_STICK_RIGHT, clampedOffsetPositive.x);
			}
		} else {
			release(Input::CONTROLLER_AXIS_RIGHT_STICK_LEFT, clampedOffsetNegative.x);
			release(Input::CONTROLLER_AXIS_RIGHT_STICK_RIGHT, clampedOffsetPositive.x);
		}
		if (glm::abs(static_cast<float>(integerPosition.y) / static_cast<float>(integerPosition.x)) > DIAGONAL_RATIO) {
			if (integerPosition.y < 0) {
				press(Input::CONTROLLER_AXIS_RIGHT_STICK_UP, clampedOffsetNegative.y);
				release(Input::CONTROLLER_AXIS_RIGHT_STICK_DOWN, clampedOffsetPositive.y);
			} else {
				release(Input::CONTROLLER_AXIS_RIGHT_STICK_UP, clampedOffsetNegative.y);
				press(Input::CONTROLLER_AXIS_RIGHT_STICK_DOWN, clampedOffsetPositive.y);
			}
		} else {
			release(Input::CONTROLLER_AXIS_RIGHT_STICK_UP, clampedOffsetNegative.y);
			release(Input::CONTROLLER_AXIS_RIGHT_STICK_DOWN, clampedOffsetPositive.y);
		}
	} else {
		release(Input::CONTROLLER_AXIS_RIGHT_STICK_UP, clampedOffsetNegative.y);
		release(Input::CONTROLLER_AXIS_RIGHT_STICK_DOWN, clampedOffsetPositive.y);
		release(Input::CONTROLLER_AXIS_RIGHT_STICK_LEFT, clampedOffsetNegative.x);
		release(Input::CONTROLLER_AXIS_RIGHT_STICK_RIGHT, clampedOffsetPositive.x);
	}
}

void InputManager::setControllerLeftTriggerPosition(float position) noexcept {
	const float oldPosition = controllerLeftTriggerPosition.value_or(0.0f);
	const glm::i32 oldAdjustedPosition = getIntegerValue((oldPosition - controllerLeftTriggerDeadzone) / (1.0f - controllerLeftTriggerDeadzone));
	const glm::i32 adjustedPosition = getIntegerValue((position - controllerLeftTriggerDeadzone) / (1.0f - controllerLeftTriggerDeadzone));
	const glm::i32 offset = adjustedPosition - oldAdjustedPosition;
	controllerLeftTriggerPosition = position;
	controllerLeftTriggerTransientMotion = true;
	if (position > controllerLeftTriggerDeadzone) {
		press(Input::CONTROLLER_AXIS_LEFT_TRIGGER, offset);
	} else {
		release(Input::CONTROLLER_AXIS_LEFT_TRIGGER, offset);
	}
}

void InputManager::setControllerRightTriggerPosition(float position) noexcept {
	const float oldPosition = controllerRightTriggerPosition.value_or(0.0f);
	const glm::i32 oldAdjustedPosition = getIntegerValue((oldPosition - controllerRightTriggerDeadzone) / (1.0f - controllerRightTriggerDeadzone));
	const glm::i32 adjustedPosition = getIntegerValue((position - controllerRightTriggerDeadzone) / (1.0f - controllerRightTriggerDeadzone));
	const glm::i32 offset = adjustedPosition - oldAdjustedPosition;
	controllerRightTriggerPosition = position;
	controllerRightTriggerTransientMotion = true;
	if (position > controllerRightTriggerDeadzone) {
		press(Input::CONTROLLER_AXIS_RIGHT_TRIGGER, offset);
	} else {
		release(Input::CONTROLLER_AXIS_RIGHT_TRIGGER, offset);
	}
}

} // namespace application
} // namespace donut
