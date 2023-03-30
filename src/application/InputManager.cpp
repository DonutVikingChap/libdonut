#include <donut/Variant.hpp>
#include <donut/application/Input.hpp>
#include <donut/application/InputManager.hpp>

#include <SDL.h>       // SDL...
#include <cstddef>     // std::size_t
#include <glm/glm.hpp> // glm::...
#include <optional>    // std::optional
#include <vector>      // std::vector

namespace donut {
namespace application {

namespace {

constexpr float DIAGONAL_RATIO = 0.41421356237f; // sqrt(2) - 1 or tan(pi / 8)

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
	, controllerRightTriggerDeadzone(options.controllerRightTriggerDeadzone) {}

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
	match(event)(                                                                              //
		[&](const WindowKeyboardFocusLostEvent&) -> void { resetAllInputs(); },                //
		[&](const WindowMouseFocusLostEvent&) -> void { mousePosition.reset(); },              //
		[&](const KeyPressedEvent& pressed) -> void { press(pressed.physicalKeyInput); },      //
		[&](const KeyReleasedEvent& released) -> void { release(released.physicalKeyInput); }, //
		[&](const MouseMovedEvent& moved) -> void {
			setMousePosition({static_cast<float>(moved.mousePosition.x), static_cast<float>(moved.mousePosition.y)});
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
				const float value = getFloatValue(static_cast<glm::i32>(moved.axisValue));
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
		[&](const auto&) -> void {});
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
	return getFloatValue(glm::max(glm::i32{0}, getAbsoluteValue(outputPositive)));
}

float InputManager::getRelativeVector(std::size_t outputPositive) const noexcept {
	return getFloatValue(glm::max(glm::i32{0}, getRelativeValue(outputPositive)));
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
	move(Input::MOUSE_SCROLL_DOWN, getIntegerValue(-offset));
	move(Input::MOUSE_SCROLL_UP, getIntegerValue(offset));
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
