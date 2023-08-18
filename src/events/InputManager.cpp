#include <donut/events/Error.hpp>
#include <donut/events/Input.hpp>
#include <donut/events/InputManager.hpp>
#include <donut/math.hpp>

#include <SDL.h>        // SDL...
#include <cstddef>      // std::size_t
#include <fmt/format.h> // fmt::format
#include <optional>     // std::optional
#include <vector>       // std::vector

namespace donut::events {

namespace {

constexpr float DIAGONAL_RATIO = 0.41421356237f; // sqrt(2) - 1 or tan(pi / 8)

[[nodiscard]] i32 getIntegerValue(float value) noexcept {
	return static_cast<i32>(floor(value * 32767.5f));
}

[[nodiscard]] float getFloatValue(i32 value) noexcept {
	return (static_cast<float>(value) + 0.5f) / 32767.5f;
}

} // namespace

InputManager::InputManager(const InputManagerOptions& options)
	: options(options) {
	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
		throw Error{fmt::format("Failed to initialize SDL gamecontroller subsystem:\n{}", SDL_GetError())};
	}
}

InputManager::~InputManager() {
	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void InputManager::prepareForEvents() {
	previousPersistentOutputs = currentPersistentOutputs;
	transientOutputPresses = {};
	transientOutputReleases = {};
	outputRelativeStates.fill(0);
	previousPersistentInputs = currentPersistentInputs;
	transientInputPresses = {};
	transientInputReleases = {};
	mouseTransientMotion = false;
	mouseWheelHorizontalTransientMotion = false;
	mouseWheelVerticalTransientMotion = false;
	controllerLeftStickTransientMotion = false;
	controllerRightStickTransientMotion = false;
	controllerLeftTriggerTransientMotion = false;
	controllerRightTriggerTransientMotion = false;
	touchTransientMotion = false;
	touchTransientPressure = false;
}

void InputManager::handleEvent(const Event& event) {
	match(event)(                                                               //
		[&](const WindowKeyboardFocusLostEvent&) -> void { resetAllInputs(); }, //
		[&](const WindowMouseFocusLostEvent&) -> void {
			mousePosition.reset();
			touchPosition.reset();
			touchPressure.reset();
		},                                                                                     //
		[&](const KeyPressedEvent& pressed) -> void { press(pressed.physicalKeyInput); },      //
		[&](const KeyReleasedEvent& released) -> void { release(released.physicalKeyInput); }, //
		[&](const MouseMovedEvent& moved) -> void {
			setMousePosition({static_cast<float>(moved.mousePosition.x), static_cast<float>(moved.mousePosition.y)},
				{static_cast<float>(moved.relativeMouseMotion.x), static_cast<float>(moved.relativeMouseMotion.y)});
		},                                                                                                     //
		[&](const MouseButtonPressedEvent& pressed) -> void { press(pressed.physicalMouseButtonInput); },      //
		[&](const MouseButtonReleasedEvent& released) -> void { release(released.physicalMouseButtonInput); }, //
		[&](const MouseWheelScrolledEvent& scrolled) -> void {
			scrollMouseWheelHorizontally(scrolled.scrollAmount.x);
			scrollMouseWheelVertically(scrolled.scrollAmount.y);
		}, //
		[&](const ControllerAddedEvent& added) -> void {
			if (controller) {
				controller.reset(SDL_GameControllerOpen(added.controllerId));
				resetAllInputs();
			} else {
				controller.reset(SDL_GameControllerOpen(added.controllerId));
			}
		}, //
		[&](const ControllerRemovedEvent& removed) -> void {
			if (controller && removed.controllerId == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				controller.reset();
				resetAllInputs();
			}
		}, //
		[&](const ControllerAxisMovedEvent& moved) -> void {
			if (controller && moved.controllerId == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				const float value = getFloatValue(static_cast<i32>(moved.axisValue));
				switch (moved.axis) {
					case ControllerAxisMovedEvent::ControllerAxis::LEFT_STICK_X:
						setControllerLeftStickPosition({value, (controllerLeftStickPosition) ? controllerLeftStickPosition->y : 0});
						break;
					case ControllerAxisMovedEvent::ControllerAxis::LEFT_STICK_Y:
						setControllerLeftStickPosition({(controllerLeftStickPosition) ? controllerLeftStickPosition->x : 0, value});
						break;
					case ControllerAxisMovedEvent::ControllerAxis::RIGHT_STICK_X:
						setControllerRightStickPosition({value, (controllerRightStickPosition) ? controllerRightStickPosition->y : 0});
						break;
					case ControllerAxisMovedEvent::ControllerAxis::RIGHT_STICK_Y:
						setControllerRightStickPosition({(controllerRightStickPosition) ? controllerRightStickPosition->x : 0, value});
						break;
					case ControllerAxisMovedEvent::ControllerAxis::LEFT_TRIGGER: setControllerLeftTriggerPosition(value); break;
					case ControllerAxisMovedEvent::ControllerAxis::RIGHT_TRIGGER: setControllerRightTriggerPosition(value); break;
					default: break;
				}
			}
		}, //
		[&](const ControllerButtonPressedEvent& pressed) -> void {
			if (controller && pressed.controllerId == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				press(pressed.physicalControllerButtonInput);
			}
		}, //
		[&](const ControllerButtonReleasedEvent& released) -> void {
			if (controller && released.controllerId == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(static_cast<SDL_GameController*>(controller.get())))) {
				release(released.physicalControllerButtonInput);
			}
		}, //
		[&](const TouchMovedEvent& moved) -> void {
			setTouchPosition(moved.normalizedFingerPosition);
			setTouchPressure(moved.normalizedFingerPressure);
		}, //
		[&](const TouchPressedEvent& pressed) -> void {
			setTouchPressure(pressed.normalizedFingerPressure);
			press(Input::TOUCH_FINGER_TAP);
		}, //
		[&](const TouchReleasedEvent& released) -> void {
			setTouchPressure(released.normalizedFingerPressure);
			release(Input::TOUCH_FINGER_TAP);
		}, //
		[&](const auto&) -> void {});
}

void InputManager::bind(Input input, Outputs outputs) {
	bindings[input] = outputs;
}

void InputManager::addBinding(Input input, Outputs outputs) {
	bindings[input] |= outputs;
}

void InputManager::unbind(Input input) {
	bindings.erase(input);
}

void InputManager::unbindAll() noexcept {
	bindings.clear();
}

void InputManager::press(Input input, i32 offset) noexcept {
	const std::size_t inputIndex = getInputIndex(input);
	const bool wasReleased = !currentPersistentInputs.test(inputIndex);
	currentPersistentInputs.set(inputIndex);
	transientInputPresses.set(inputIndex);
	if (const auto it = bindings.find(input); it != bindings.end()) {
		const Outputs outputs = it->second;
		currentPersistentOutputs |= outputs;
		transientOutputPresses |= outputs;
		for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
			if (outputs.test(i)) {
				outputRelativeStates[i] += offset;
				if (wasReleased) {
					outputAbsoluteStates[i] += offset;
					++outputPersistentPresses[i];
				}
			}
		}
	}
}

void InputManager::release(Input input, i32 offset) noexcept {
	const std::size_t inputIndex = getInputIndex(input);
	const bool wasPressed = currentPersistentInputs.test(inputIndex);
	currentPersistentInputs.set(inputIndex, false);
	transientInputReleases.set(inputIndex);
	if (const auto it = bindings.find(input); it != bindings.end()) {
		const Outputs outputs = it->second;
		transientOutputReleases |= outputs;
		for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
			if (outputs.test(i)) {
				outputRelativeStates[i] += offset;
				if (wasPressed) {
					outputAbsoluteStates[i] += offset;
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

void InputManager::move(Input input, i32 offset) noexcept {
	if (offset > 0) {
		const std::size_t inputIndex = getInputIndex(input);
		transientInputPresses.set(inputIndex);
		transientInputReleases.set(inputIndex);
		if (const auto it = bindings.find(input); it != bindings.end()) {
			const Outputs outputs = it->second;
			transientOutputPresses |= outputs;
			transientOutputReleases |= outputs;
			for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
				if (outputs.test(i)) {
					outputRelativeStates[i] += offset;
				}
			}
		}
	}
}

void InputManager::set(Input input, i32 value) noexcept {
	if (const auto it = bindings.find(input); it != bindings.end()) {
		const Outputs outputs = it->second;
		for (std::size_t i = 0; i < OUTPUT_COUNT; ++i) {
			if (outputs.test(i)) {
				outputAbsoluteStates[i] = value;
			}
		}
	}
}

void InputManager::resetAllInputs() noexcept {
	mousePosition = {};
	controllerLeftStickPosition = {};
	controllerRightStickPosition = {};
	touchPosition = {};
	touchPressure = {};
	currentPersistentOutputs = {};
	transientOutputReleases = {};
	transientOutputPresses = {};
	outputAbsoluteStates.fill(0);
	outputRelativeStates.fill(0);
	outputPersistentPresses.fill(0);
	currentPersistentInputs = {};
	transientInputReleases = {};
	transientInputPresses = {};
}

void InputManager::setMouseSensitivity(float sensitivity) noexcept {
	options.mouseSensitivity = sensitivity;
}

void InputManager::setControllerLeftStickSensitivity(float sensitivity) noexcept {
	options.controllerLeftStickSensitivity = sensitivity;
}

void InputManager::setControllerRightStickSensitivity(float sensitivity) noexcept {
	options.controllerRightStickSensitivity = sensitivity;
}

void InputManager::setControllerLeftStickDeadzone(float deadzone) noexcept {
	options.controllerLeftStickDeadzone = deadzone;
}

void InputManager::setControllerRightStickDeadzone(float deadzone) noexcept {
	options.controllerRightStickDeadzone = deadzone;
}

void InputManager::setControllerLeftTriggerDeadzone(float deadzone) noexcept {
	options.controllerLeftTriggerDeadzone = deadzone;
}

void InputManager::setControllerRightTriggerDeadzone(float deadzone) noexcept {
	options.controllerRightTriggerDeadzone = deadzone;
}

void InputManager::setTouchMotionSensitivity(float sensitivity) noexcept {
	options.touchMotionSensitivity = sensitivity;
}

void InputManager::setTouchPressureDeadzone(float deadzone) noexcept {
	options.touchPressureDeadzone = deadzone;
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

std::optional<vec2> InputManager::getMousePosition() const noexcept {
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

std::optional<vec2> InputManager::getControllerLeftStickPosition() const noexcept {
	return controllerLeftStickPosition;
}

std::optional<vec2> InputManager::getControllerRightStickPosition() const noexcept {
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

std::optional<vec2> InputManager::getTouchPosition() const noexcept {
	return touchPosition;
}

std::optional<float> InputManager::getTouchPressure() const noexcept {
	return touchPressure;
}

bool InputManager::touchJustMoved() const noexcept {
	return touchTransientMotion;
}

bool InputManager::touchJustChangedPressure() const noexcept {
	return touchTransientPressure;
}

InputManager::Outputs InputManager::getCurrentOutputs() const noexcept {
	return currentPersistentOutputs;
}

InputManager::Outputs InputManager::getPreviousOutputs() const noexcept {
	return previousPersistentOutputs;
}

InputManager::Outputs InputManager::getJustPressedOutputs() const noexcept {
	return transientOutputPresses | (currentPersistentOutputs & ~previousPersistentOutputs);
}

InputManager::Outputs InputManager::getJustReleasedOutputs() const noexcept {
	return transientOutputReleases | (previousPersistentOutputs & ~currentPersistentOutputs);
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

i32 InputManager::getAbsoluteState(std::size_t output) const noexcept {
	return outputAbsoluteStates[output];
}

i32 InputManager::getRelativeState(std::size_t output) const noexcept {
	return outputRelativeStates[output];
}

float InputManager::getAbsoluteValue(std::size_t outputPositive) const noexcept {
	return getFloatValue(max(i32{0}, getAbsoluteState(outputPositive)));
}

float InputManager::getRelativeValue(std::size_t outputPositive) const noexcept {
	return getFloatValue(max(i32{0}, getRelativeState(outputPositive)));
}

float InputManager::getAbsoluteValue(std::size_t outputNegative, std::size_t outputPositive) const noexcept {
	return getAbsoluteValue(outputPositive) - getAbsoluteValue(outputNegative);
}

float InputManager::getRelativeValue(std::size_t outputNegative, std::size_t outputPositive) const noexcept {
	return getRelativeValue(outputPositive) - getRelativeValue(outputNegative);
}

vec2 InputManager::getAbsoluteValue(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept {
	return {
		getAbsoluteValue(outputNegativeX, outputPositiveX),
		getAbsoluteValue(outputNegativeY, outputPositiveY),
	};
}

vec2 InputManager::getRelativeValue(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept {
	return {
		getRelativeValue(outputNegativeX, outputPositiveX),
		getRelativeValue(outputNegativeY, outputPositiveY),
	};
}

vec3 InputManager::getAbsoluteValue(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY, std::size_t outputNegativeZ,
	std::size_t outputPositiveZ) const noexcept {
	return {
		getAbsoluteValue(outputNegativeX, outputPositiveX),
		getAbsoluteValue(outputNegativeY, outputPositiveY),
		getAbsoluteValue(outputNegativeZ, outputPositiveZ),
	};
}

vec3 InputManager::getRelativeValue(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY, std::size_t outputNegativeZ,
	std::size_t outputPositiveZ) const noexcept {
	return {
		getRelativeValue(outputNegativeX, outputPositiveX),
		getRelativeValue(outputNegativeY, outputPositiveY),
		getRelativeValue(outputNegativeZ, outputPositiveZ),
	};
}

bool InputManager::isPressed(Input input) const noexcept {
	return currentPersistentInputs.test(getInputIndex(input));
}

bool InputManager::justPressed(Input input) const noexcept {
	return (transientInputPresses | (currentPersistentInputs & ~previousPersistentInputs)).test(getInputIndex(input));
}

bool InputManager::justReleased(Input input) const noexcept {
	return (transientInputReleases | (previousPersistentInputs & ~currentPersistentInputs)).test(getInputIndex(input));
}

void InputManager::ControllerDeleter::operator()(void* handle) const noexcept {
	SDL_GameControllerClose(static_cast<SDL_GameController*>(handle));
}

void InputManager::setMousePosition(vec2 position, vec2 relativeMotion) noexcept {
	mousePosition = position;
	mouseTransientMotion = true;
	const vec2 offset = relativeMotion * options.mouseSensitivity;
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
	move(Input::MOUSE_SCROLL_DOWN, getIntegerValue(-offset));
	move(Input::MOUSE_SCROLL_UP, getIntegerValue(offset));
}

void InputManager::setControllerLeftStickPosition(vec2 position) noexcept {
	const float sensitivity = options.controllerLeftStickSensitivity;
	const float deadzone = options.controllerLeftStickDeadzone;
	const vec2 oldPosition = controllerLeftStickPosition.value_or(vec2{0.0f, 0.0f});
	const float oldLength = length(oldPosition);
	const float newLength = length(position);
	const vec2 oldAdjustedPosition = (oldLength > deadzone) ? oldPosition * (sensitivity * ((oldLength - deadzone) / (oldLength * (1.0f - deadzone)))) : vec2{0.0f, 0.0f};
	const vec2 newAdjustedPosition = (newLength > deadzone) ? position * (sensitivity * ((newLength - deadzone) / (newLength * (1.0f - deadzone)))) : vec2{0.0f, 0.0f};
	const i32vec2 oldIntegerPosition{getIntegerValue(oldAdjustedPosition.x), getIntegerValue(oldAdjustedPosition.y)};
	const i32vec2 newIntegerPosition{getIntegerValue(newAdjustedPosition.x), getIntegerValue(newAdjustedPosition.y)};
	const i32vec2 offset = newIntegerPosition - oldIntegerPosition;
	const i32vec2 clampedOffsetNegative = max(min(i32vec2{0, 0}, oldIntegerPosition), min(-offset, -offset - oldIntegerPosition));
	const i32vec2 clampedOffsetPositive = max(min(i32vec2{0, 0}, -oldIntegerPosition), min(offset, offset + oldIntegerPosition));
	controllerLeftStickPosition = position;
	controllerLeftStickTransientMotion = true;
	if (newLength > deadzone) {
		if (abs(static_cast<float>(newIntegerPosition.x) / static_cast<float>(newIntegerPosition.y)) > DIAGONAL_RATIO) {
			if (newIntegerPosition.x < 0) {
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
		if (abs(static_cast<float>(newIntegerPosition.y) / static_cast<float>(newIntegerPosition.x)) > DIAGONAL_RATIO) {
			if (newIntegerPosition.y < 0) {
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

void InputManager::setControllerRightStickPosition(vec2 position) noexcept {
	const float sensitivity = options.controllerRightStickSensitivity;
	const float deadzone = options.controllerRightStickDeadzone;
	const vec2 oldPosition = controllerRightStickPosition.value_or(vec2{0.0f, 0.0f});
	const float oldLength = length(oldPosition);
	const float newLength = length(position);
	const vec2 oldAdjustedPosition = (oldLength > deadzone) ? oldPosition * (sensitivity * ((oldLength - deadzone) / (oldLength * (1.0f - deadzone)))) : vec2{0.0f, 0.0f};
	const vec2 newAdjustedPosition = (newLength > deadzone) ? position * (sensitivity * ((newLength - deadzone) / (newLength * (1.0f - deadzone)))) : vec2{0.0f, 0.0f};
	const i32vec2 oldIntegerPosition{getIntegerValue(oldAdjustedPosition.x), getIntegerValue(oldAdjustedPosition.y)};
	const i32vec2 newIntegerPosition{getIntegerValue(newAdjustedPosition.x), getIntegerValue(newAdjustedPosition.y)};
	const i32vec2 offset = newIntegerPosition - oldIntegerPosition;
	const i32vec2 clampedOffsetNegative = max(min(i32vec2{0, 0}, oldIntegerPosition), min(-offset, -offset - oldIntegerPosition));
	const i32vec2 clampedOffsetPositive = max(min(i32vec2{0, 0}, -oldIntegerPosition), min(offset, offset + oldIntegerPosition));
	controllerRightStickPosition = position;
	controllerRightStickTransientMotion = true;
	if (newLength > deadzone) {
		if (abs(static_cast<float>(newIntegerPosition.x) / static_cast<float>(newIntegerPosition.y)) > DIAGONAL_RATIO) {
			if (newIntegerPosition.x < 0) {
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
		if (abs(static_cast<float>(newIntegerPosition.y) / static_cast<float>(newIntegerPosition.x)) > DIAGONAL_RATIO) {
			if (newIntegerPosition.y < 0) {
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
	const float deadzone = options.controllerLeftTriggerDeadzone;
	const float oldPosition = controllerLeftTriggerPosition.value_or(0.0f);
	const i32 oldAdjustedPosition = getIntegerValue((oldPosition - deadzone) / (1.0f - deadzone));
	const i32 adjustedPosition = getIntegerValue((position - deadzone) / (1.0f - deadzone));
	const i32 offset = adjustedPosition - oldAdjustedPosition;
	controllerLeftTriggerPosition = position;
	controllerLeftTriggerTransientMotion = true;
	if (position > deadzone) {
		press(Input::CONTROLLER_AXIS_LEFT_TRIGGER, offset);
	} else {
		release(Input::CONTROLLER_AXIS_LEFT_TRIGGER, offset);
	}
}

void InputManager::setControllerRightTriggerPosition(float position) noexcept {
	const float deadzone = options.controllerRightTriggerDeadzone;
	const float oldPosition = controllerRightTriggerPosition.value_or(0.0f);
	const i32 oldAdjustedPosition = getIntegerValue((oldPosition - deadzone) / (1.0f - deadzone));
	const i32 adjustedPosition = getIntegerValue((position - deadzone) / (1.0f - deadzone));
	const i32 offset = adjustedPosition - oldAdjustedPosition;
	controllerRightTriggerPosition = position;
	controllerRightTriggerTransientMotion = true;
	if (position > deadzone) {
		press(Input::CONTROLLER_AXIS_RIGHT_TRIGGER, offset);
	} else {
		release(Input::CONTROLLER_AXIS_RIGHT_TRIGGER, offset);
	}
}

void InputManager::setTouchPosition(vec2 position) noexcept {
	const float sensitivity = options.touchMotionSensitivity;
	const vec2 offset = (touchPosition) ? (position - *touchPosition) * sensitivity : vec2{0.0f, 0.0f};
	touchPosition = position;
	touchTransientMotion = true;
	move(Input::TOUCH_FINGER_MOTION_UP, getIntegerValue(-offset.y));
	move(Input::TOUCH_FINGER_MOTION_DOWN, getIntegerValue(offset.y));
	move(Input::TOUCH_FINGER_MOTION_LEFT, getIntegerValue(-offset.x));
	move(Input::TOUCH_FINGER_MOTION_RIGHT, getIntegerValue(offset.x));
}

void InputManager::setTouchPressure(float pressure) noexcept {
	const float deadzone = options.touchPressureDeadzone;
	const float oldPressure = touchPressure.value_or(0.0f);
	const i32 oldAdjustedPressure = getIntegerValue((oldPressure - deadzone) / (1.0f - deadzone));
	const i32 adjustedPressure = getIntegerValue((pressure - deadzone) / (1.0f - deadzone));
	const i32 offset = adjustedPressure - oldAdjustedPressure;
	touchPressure = pressure;
	touchTransientPressure = true;
	if (pressure > deadzone) {
		press(Input::TOUCH_FINGER_PRESSURE, offset);
	} else {
		release(Input::TOUCH_FINGER_PRESSURE, offset);
	}
}

} // namespace donut::events
