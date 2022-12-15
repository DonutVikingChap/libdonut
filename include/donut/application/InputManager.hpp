#ifndef DONUT_APPLICATION_INPUT_MANAGER_HPP
#define DONUT_APPLICATION_INPUT_MANAGER_HPP

#include <donut/Resource.hpp>
#include <donut/application/Event.hpp>
#include <donut/application/Input.hpp>

#include <array>   // std::array
#include <bitset>  // std::bitset
#include <cstddef> // std::size_t
#include <glm/fwd.hpp>
#include <glm/glm.hpp>   // glm::...
#include <optional>      // std::optional
#include <type_traits>   // std::is_enum_v, std::underlying_type_t
#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace donut {
namespace application {

struct InputManagerOptions {
	float mouseSensitivity = 0.005f;
	float controllerLeftStickSensitivity = 1.0f;
	float controllerRightStickSensitivity = 1.0f;
	float controllerLeftStickDeadzone = 0.2f;
	float controllerRightStickDeadzone = 0.2f;
	float controllerLeftTriggerDeadzone = 0.2f;
	float controllerRightTriggerDeadzone = 0.2f;
};

class InputManager {
public:
	static constexpr std::size_t OUTPUT_COUNT = 64;

	using Outputs = std::bitset<OUTPUT_COUNT>;

	struct Binding final {
		Input input;
		Outputs outputs;
	};

	InputManager(const InputManagerOptions& options = {});
	~InputManager();

	InputManager(const InputManager&) = delete;
	InputManager(InputManager&&) = delete;
	InputManager& operator=(const InputManager&) = delete;
	InputManager& operator=(InputManager&&) = delete;

	void prepareForEvents();
	void handleEvent(const application::Event& event);

	void bind(Input input, Outputs outputs);
	void addBinding(Input input, Outputs outputs);
	void unbind(Input input);
	void unbindAll() noexcept;

	void press(Input input, glm::i32 offset = 32767) noexcept;
	void release(Input input, glm::i32 offset = -32767) noexcept;
	void move(Input input, glm::i32 offset) noexcept;
	void resetAllInputs() noexcept;

	void setMouseSensitivity(float sensitivity) noexcept;
	void setControllerLeftStickSensitivity(float sensitivity) noexcept;
	void setControllerRightStickSensitivity(float sensitivity) noexcept;
	void setControllerLeftStickDeadzone(float deadzone) noexcept;
	void setControllerRightStickDeadzone(float deadzone) noexcept;
	void setControllerLeftTriggerDeadzone(float deadzone) noexcept;
	void setControllerRightTriggerDeadzone(float deadzone) noexcept;

	void setMousePosition(glm::vec2 position) noexcept;
	void scrollMouseWheelHorizontally(float offset) noexcept;
	void scrollMouseWheelVertically(float offset) noexcept;

	void setControllerLeftStickPosition(glm::vec2 position) noexcept;
	void setControllerRightStickPosition(glm::vec2 position) noexcept;
	void setControllerLeftTriggerPosition(float position) noexcept;
	void setControllerRightTriggerPosition(float position) noexcept;

	[[nodiscard]] bool hasAnyBindings() const noexcept;
	[[nodiscard]] std::vector<Binding> getBindings() const;
	[[nodiscard]] const Outputs* findBinding(Input input) const noexcept;

	[[nodiscard]] std::optional<glm::vec2> getMousePosition() const noexcept;
	[[nodiscard]] bool mouseJustMoved() const noexcept;
	[[nodiscard]] bool mouseWheelJustScrolledHorizontally() const noexcept;
	[[nodiscard]] bool mouseWheelJustScrolledVertically() const noexcept;

	[[nodiscard]] bool isControllerConnected() const noexcept;
	[[nodiscard]] std::optional<glm::vec2> getControllerLeftStickPosition() const noexcept;
	[[nodiscard]] std::optional<glm::vec2> getControllerRightStickPosition() const noexcept;
	[[nodiscard]] std::optional<float> getControllerLeftTriggerPosition() const noexcept;
	[[nodiscard]] std::optional<float> getControllerRightTriggerPosition() const noexcept;
	[[nodiscard]] bool controllerLeftStickJustMoved() const noexcept;
	[[nodiscard]] bool controllerRightStickJustMoved() const noexcept;
	[[nodiscard]] bool controllerLeftTriggerJustMoved() const noexcept;
	[[nodiscard]] bool controllerRightTriggerJustMoved() const noexcept;

	[[nodiscard]] Outputs getCurrentOutputs() const noexcept;
	[[nodiscard]] Outputs getPreviousOutputs() const noexcept;
	[[nodiscard]] Outputs getJustPressedOutputs() const noexcept;
	[[nodiscard]] Outputs getJustReleasedOutputs() const noexcept;

	[[nodiscard]] bool isPressed(std::size_t output) const noexcept;
	[[nodiscard]] bool justPressed(std::size_t output) const noexcept;
	[[nodiscard]] bool justReleased(std::size_t output) const noexcept;
	[[nodiscard]] glm::i32 getAbsoluteValue(std::size_t output) const noexcept;
	[[nodiscard]] glm::i32 getRelativeValue(std::size_t output) const noexcept;
	[[nodiscard]] float getAbsoluteVector(std::size_t outputPositive) const noexcept;
	[[nodiscard]] float getRelativeVector(std::size_t outputPositive) const noexcept;
	[[nodiscard]] float getAbsoluteVector(std::size_t outputNegative, std::size_t outputPositive) const noexcept;
	[[nodiscard]] float getRelativeVector(std::size_t outputNegative, std::size_t outputPositive) const noexcept;
	[[nodiscard]] glm::vec2 getAbsoluteVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept;
	[[nodiscard]] glm::vec2 getRelativeVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept;

	[[nodiscard]] bool isPressed(Input input) const noexcept;
	[[nodiscard]] bool justPressed(Input input) const noexcept;
	[[nodiscard]] bool justReleased(Input input) const noexcept;

	template <typename... Actions>
	void bind(Input input, Actions... actions) requires(std::is_enum_v<Actions>&&...) {
		Outputs outputs{};
		(outputs.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Actions>>(actions))), ...);
		bind(input, outputs);
	}

	template <typename... Actions>
	void addBinding(Input input, Actions... actions) requires(std::is_enum_v<Actions>&&...) {
		Outputs outputs{};
		(outputs.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Actions>>(actions))), ...);
		addBinding(input, outputs);
	}

	template <typename Action>
	[[nodiscard]] bool isPressed(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return isPressed(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	template <typename Action>
	[[nodiscard]] bool justPressed(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return justPressed(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	template <typename Action>
	[[nodiscard]] bool justReleased(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return justReleased(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	template <typename Action>
	[[nodiscard]] glm::i32 getAbsoluteValue(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteValue(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	template <typename Action>
	[[nodiscard]] glm::i32 getRelativeValue(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeValue(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	template <typename Action>
	[[nodiscard]] float getAbsoluteVector(Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	template <typename Action>
	[[nodiscard]] float getRelativeVector(Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	template <typename Action>
	[[nodiscard]] float getAbsoluteVector(Action actionNegative, Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegative)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	template <typename Action>
	[[nodiscard]] float getRelativeVector(Action actionNegative, Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegative)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	template <typename Action>
	[[nodiscard]] glm::vec2 getAbsoluteVector(Action actionNegativeX, Action actionPositiveX, Action actionNegativeY, Action actionPositiveY) const noexcept
		requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveY)));
	}

	template <typename Action>
	[[nodiscard]] glm::vec2 getRelativeVector(Action actionNegativeX, Action actionPositiveX, Action actionNegativeY, Action actionPositiveY) const noexcept
		requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveY)));
	}

private:
	struct ControllerDeleter final {
		void operator()(void* handle) const noexcept;
	};

	using Controller = Resource<void*, ControllerDeleter, nullptr>;

	std::unordered_map<Input, Outputs> bindings{};
	float mouseSensitivity;
	float controllerLeftStickSensitivity;
	float controllerRightStickSensitivity;
	float controllerLeftStickDeadzone;
	float controllerRightStickDeadzone;
	float controllerLeftTriggerDeadzone;
	float controllerRightTriggerDeadzone;
	std::optional<glm::vec2> mousePosition{};
	Controller controller{};
	std::optional<glm::vec2> controllerLeftStickPosition{};
	std::optional<glm::vec2> controllerRightStickPosition{};
	std::optional<float> controllerLeftTriggerPosition{};
	std::optional<float> controllerRightTriggerPosition{};
	Outputs currentPersistentOutputs{};
	Outputs previousPersistentOutputs{};
	Outputs transientOutputs{};
	std::array<glm::i32, OUTPUT_COUNT> outputAbsoluteValues{};
	std::array<glm::i32, OUTPUT_COUNT> outputRelativeValues{};
	std::array<std::uint8_t, OUTPUT_COUNT> outputPersistentPresses{};
	std::bitset<INPUT_COUNT> currentPersistentInputs{};
	std::bitset<INPUT_COUNT> previousPersistentInputs{};
	std::bitset<INPUT_COUNT> transientInputs{};
	bool mouseTransientMotion = false;
	bool mouseWheelHorizontalTransientMotion = false;
	bool mouseWheelVerticalTransientMotion = false;
	bool controllerLeftStickTransientMotion = false;
	bool controllerRightStickTransientMotion = false;
	bool controllerLeftTriggerTransientMotion = false;
	bool controllerRightTriggerTransientMotion = false;
};

} // namespace application
} // namespace donut

#endif
