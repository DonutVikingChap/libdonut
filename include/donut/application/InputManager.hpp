#ifndef DONUT_APPLICATION_INPUT_MANAGER_HPP
#define DONUT_APPLICATION_INPUT_MANAGER_HPP

#include <donut/Resource.hpp>
#include <donut/application/Event.hpp>
#include <donut/application/Input.hpp>

#include <array>
#include <bitset>
#include <cstddef>
#include <glm/glm.hpp>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace donut {
namespace application {

struct InputManagerOptions {
	glm::vec2 mouseOrigin{0.0f, 0.0f};
	float controllerLeftStickDeadzone = 0.05f;
	float controllerRightStickDeadzone = 0.05f;
	float controllerLeftTriggerDeadzone = 0.1f;
	float controllerRightTriggerDeadzone = 0.1f;
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

	void resize(glm::ivec2 newWindowSize);
	void beginFrame();
	void handleEvent(const application::Event& event);

	void bind(Input input, Outputs outputs);
	void addBinding(Input input, Outputs outputs);
	void unbind(Input input);
	void unbindAll() noexcept;

	void press(Input input) noexcept;
	void press(Outputs outputs) noexcept;
	void release(Input input) noexcept;
	void release(Outputs outputs) noexcept;
	void set(Input input, float value = 1.0f) noexcept;
	void set(Outputs outputs, float value = 1.0f) noexcept;
	void move(Input input, float offset = 0.0f) noexcept;
	void move(Outputs outputs, float offset = 0.0f) noexcept;
	void moveTo(Input input, float value, float offset) noexcept;
	void moveTo(Outputs outputs, float value, float offset) noexcept;
	void reset(Input input) noexcept;
	void reset(Outputs outputs) noexcept;
	void resetAll() noexcept;

	void setMouseOrigin(glm::vec2 origin) noexcept;
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
	[[nodiscard]] float getAbsoluteValue(std::size_t output) const noexcept;
	[[nodiscard]] float getRelativeValue(std::size_t output) const noexcept;

	[[nodiscard]] bool isPressed(Input input) const noexcept;
	[[nodiscard]] bool justPressed(Input input) const noexcept;
	[[nodiscard]] bool justReleased(Input input) const noexcept;

	template <typename Action>
	void bind(Input input, Action action) requires(std::is_enum_v<Action>) {
		bind(input, Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))));
	}

	template <typename Action>
	void addBinding(Input input, Action action) requires(std::is_enum_v<Action>) {
		addBinding(input, Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))));
	}

	template <typename Action>
	void press(Action action) noexcept requires(std::is_enum_v<Action>) {
		press(Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))));
	}

	template <typename Action>
	void release(Action action) noexcept requires(std::is_enum_v<Action>) {
		release(Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))));
	}

	template <typename Action>
	void set(Action action, float value = 1.0f) noexcept requires(std::is_enum_v<Action>) {
		set(Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))), value);
	}

	template <typename Action>
	void move(Action action, float offset = 0.0f) noexcept requires(std::is_enum_v<Action>) {
		move(Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))), offset);
	}

	template <typename Action>
	void moveTo(Action action, float value, float offset) noexcept requires(std::is_enum_v<Action>) {
		moveTo(Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))), value, offset);
	}

	template <typename Action>
	void reset(Action action) noexcept requires(std::is_enum_v<Action>) {
		reset(Outputs{}.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action))));
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
	[[nodiscard]] float getAbsoluteValue(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteValue(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	template <typename Action>
	[[nodiscard]] float getRelativeValue(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeValue(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

private:
	struct ControllerDeleter final {
		void operator()(void* handle) const noexcept;
	};

	using Controller = Resource<void*, ControllerDeleter, nullptr>;

	std::unordered_map<Input, Outputs> bindings{};
	glm::vec2 mouseOrigin{0.0f, 0.0f};
	float controllerLeftStickDeadzone;
	float controllerRightStickDeadzone;
	float controllerLeftTriggerDeadzone;
	float controllerRightTriggerDeadzone;
	float mouseCoordinateScale = 1.0f;
	std::optional<glm::vec2> mousePosition = std::nullopt;
	Controller controller{};
	std::optional<glm::vec2> controllerLeftStickPosition = std::nullopt;
	std::optional<glm::vec2> controllerRightStickPosition = std::nullopt;
	std::optional<float> controllerLeftTriggerPosition = std::nullopt;
	std::optional<float> controllerRightTriggerPosition = std::nullopt;
	Outputs currentPersistentOutputs{};
	Outputs previousPersistentOutputs{};
	Outputs transientOutputs{};
	std::array<float, OUTPUT_COUNT> outputAbsoluteValues{};
	std::array<float, OUTPUT_COUNT> outputRelativeValues{};
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
