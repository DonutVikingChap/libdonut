#ifndef DONUT_EVENTS_INPUT_MANAGER_HPP
#define DONUT_EVENTS_INPUT_MANAGER_HPP

#include <donut/UniqueHandle.hpp>
#include <donut/events/Event.hpp>
#include <donut/events/Input.hpp>
#include <donut/math.hpp>

#include <array>         // std::array
#include <bitset>        // std::bitset
#include <cstddef>       // std::size_t
#include <optional>      // std::optional
#include <type_traits>   // std::is_enum_v, std::underlying_type_t
#include <unordered_map> // std::unordered_map
#include <vector>        // std::vector

namespace donut::events {

/**
 * Configuration options for an InputManager.
 */
struct InputManagerOptions {
	/**
	 * Mouse sensitivity coefficient.
	 *
	 * The influence of mouse movement on its bound output values will be
	 * multiplied by this value before being applied. This should adjust the
	 * original mouse motion, which is measured in screen coordinates, such that
	 * it becomes more in line with the typical 0-1 range of a key press or
	 * joystick axis.
	 */
	float mouseSensitivity = 0.00038397243543875251f;

	/**
	 * Controller left analog stick sensitivity coefficient.
	 *
	 * The influence of controller left analog stick movement on its bound
	 * output values will be multiplied by this value before being applied.
	 */
	float controllerLeftStickSensitivity = 1.0f;

	/**
	 * Controller right analog stick sensitivity coefficient.
	 *
	 * The influence of controller right analog stick movement on its bound
	 * output values will be multiplied by this value before being applied.
	 */
	float controllerRightStickSensitivity = 1.0f;

	/**
	 * Controller left analog stick deadzone fraction.
	 *
	 * When the stick is at a position whose fractional distance from (0, 0) is
	 * less than or equal to this value, the actual position will be ignored and
	 * treated as if it was (0, 0) in order to avoid fluctuations or drifting
	 * when the stick is at rest.
	 */
	float controllerLeftStickDeadzone = 0.2f;

	/**
	 * Controller right analog stick deadzone fraction.
	 *
	 * When the stick is at a position whose fractional distance from (0, 0) is
	 * less than or equal to this value, the actual position will be ignored and
	 * treated as if it was (0, 0) in order to avoid fluctuations or drifting
	 * when the stick is at rest.
	 */
	float controllerRightStickDeadzone = 0.2f;

	/**
	 * Controller left trigger deadzone fraction.
	 *
	 * When the trigger is at a position whose fractional distance from 0 is
	 * less than or equal to this value, the actual position will be ignored and
	 * treated as if it was 0 in order to avoid fluctuations and accidental
	 * inputs when the trigger is at rest.
	 */
	float controllerLeftTriggerDeadzone = 0.2f;

	/**
	 * Controller right trigger deadzone fraction.
	 *
	 * When the trigger is at a position whose fractional distance from 0 is
	 * less than or equal to this value, the actual position will be ignored and
	 * treated as if it was 0 in order to avoid fluctuations and accidental
	 * inputs when the trigger is at rest.
	 */
	float controllerRightTriggerDeadzone = 0.2f;

	/**
	 * Touch finger motion sensitivity coefficient.
	 *
	 * The influence of finger movement on its bound output values will be
	 * multiplied by this value before being applied.
	 */
	float touchMotionSensitivity = 1.0f;

	/**
	 * Touch finger pressure deadzone fraction.
	 *
	 * When the pressure amount is less than or equal to this value, the actual
	 * position will be ignored and treated as if it was 0 in order to avoid
	 * fluctuations and accidental inputs when the finger is at rest.
	 */
	float touchPressureDeadzone = 0.2f;
};

/**
 * Persistent system for mapping physical #Input controls to abstract output
 * numbers and processing input events that control their associated values.
 *
 * By keeping an instance of this class and continuously feeding it the events
 * received from an EventPump, it can serve as the main source of user input for
 * the whole program. After handling the events received in a frame, the input
 * manager can be queried for the current state of any specific physical inputs,
 * or the values of the abstract outputs to which they are bound, as well as the
 * corresponding state of the previous frame. This combination allows the inputs
 * or outputs which were just triggered since the previous frame to be derived
 * as well.
 *
 * The supported control types include keyboard, mouse, touch and game
 * controller devices, and it is possible for the value of a given output to be
 * affected by the input of different device types simultaneously. However,
 * differentiating the source of an input between multiple connected devices of
 * the same type is not possible. Therefore, if any filtering of events by their
 * source is desired, it needs to be done _before_ feeding the events to the
 * input manager.
 */
class InputManager {
public:
	/**
	 * The maximum supported number of separate outputs that the input manager
	 * can keep track of.
	 */
	static constexpr std::size_t OUTPUT_COUNT = 64;

	/**
	 * Set of up to #OUTPUT_COUNT unique abstract output numbers.
	 *
	 * Each bit in the set corresponds to the output number that is equal to its
	 * index.
	 */
	using Outputs = std::bitset<OUTPUT_COUNT>;

	/**
	 * A single configured binding from a physical input to a set of abstract
	 * output numbers.
	 */
	struct Binding {
		Input input;
		Outputs outputs;
	};

	/**
	 * Construct an input manager.
	 *
	 * \param options initial configuration of the input manager, see
	 *        InputManagerOptions.
	 *
	 * \throws events::Error on failure to initialize the required global
	 *         subsystems.
	 */
	InputManager(const InputManagerOptions& options = {});

	/** Destructor. */
	~InputManager();

	/** Copying an input manager is not allowed, since it manages global state. */
	InputManager(const InputManager&) = delete;

	/** Moving an input manager is not allowed, since it manages global state. */
	InputManager(InputManager&&) = delete;

	/** Copying an input manager is not allowed, since it manages global state. */
	InputManager& operator=(const InputManager&) = delete;

	/** Moving an input manager is not allowed, since it manages global state. */
	InputManager& operator=(InputManager&&) = delete;

	/**
	 * Update the internal state to prepare for the input events of the current
	 * frame to be handled.
	 *
	 * This effectively shifts any inputs/outputs which are currently considered
	 * to be pressed to the previous frame, and then resets the current input
	 * state.
	 *
	 * \note This function should typically be called once every frame during
	 *       the application::Application::update() callback.
	 *
	 * \sa handleEvent()
	 */
	void prepareForEvents();

	/**
	 * Handle an event from an EventPump, which may cause updates to the
	 * internal input/output state of the current frame.
	 *
	 * \param event event to handle.
	 *
	 * \note This function should typically be called during the
	 *       application::Application::update() callback, after polling events
	 *       from an EventPump.
	 *
	 * \sa prepareForEvents()
	 */
	void handleEvent(const Event& event);

	/**
	 * Bind a physical input to a set of abstract output numbers, overriding the
	 * previous binding if one already existed for the same input.
	 *
	 * \param input physical input to set the binding for.
	 * \param outputs set of outputs which the input should control.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa addBinding()
	 * \sa unbind()
	 * \sa unbindAll()
	 */
	void bind(Input input, Outputs outputs);

	/**
	 * Bind a physical input to a set of abstract output numbers, adding to the
	 * previous binding if one already existed for the same input.
	 *
	 * \param input physical input to add the binding to.
	 * \param outputs additional set of outputs which the input should control.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa bind()
	 * \sa unbind()
	 * \sa unbindAll()
	 */
	void addBinding(Input input, Outputs outputs);

	/**
	 * Remove all outputs from a specific input.
	 *
	 * \param input physical input to remove the binding from.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa bind()
	 * \sa addBinding()
	 * \sa unbindAll()
	 */
	void unbind(Input input);

	/**
	 * Remove all bindings from all inputs.
	 *
	 * \sa bind()
	 * \sa addBinding()
	 * \sa unbind()
	 * \sa set()
	 * \sa resetAllInputs()
	 */
	void unbindAll() noexcept;

	/**
	 * Add to an input and apply an offset to all of its bound outputs.
	 *
	 * \param input physical input to press.
	 * \param offset signed integer offset to apply to all bound outputs. The
	 *        standard range of values is 0 (no change) to 32767 (fully
	 *        pressed).
	 *
	 * \note This function is called automatically by the input manager when an
	 *       input is activated by handleEvent(), and should typically not be
	 *       called manually.
	 *
	 * \sa release()
	 * \sa move()
	 * \sa set()
	 * \sa resetAllInputs()
	 */
	void press(Input input, i32 offset = 32767) noexcept;

	/**
	 * Subtract from an input and apply an offset to all of its bound outputs.
	 *
	 * \param input physical input to release.
	 * \param offset signed integer offset to apply to all bound outputs. The
	 *        standard range of values is -32767 (fully unpressed) to 0 (no
	 *        change).
	 *
	 * \note This function is called automatically by the input manager when an
	 *       input is deactivated by handleEvent(), and should typically not be
	 *       called manually.
	 *
	 * \sa press()
	 * \sa move()
	 * \sa set()
	 * \sa resetAllInputs()
	 */
	void release(Input input, i32 offset = -32767) noexcept;

	/**
	 * Trigger a transient activation of an input that only lasts for the current
	 * frame and apply a relative offset to all of its bound outputs.
	 *
	 * \param input physical input to trigger.
	 * \param offset signed integer offset to apply to all bound outputs. The
	 *        standard range of values is -32767 to 32767.
	 *
	 * \note This function is called automatically by the input manager when an
	 *       input is triggered by handleEvent(), and should typically not be
	 *       called manually.
	 *
	 * \sa press()
	 * \sa release()
	 * \sa set()
	 * \sa resetAllInputs()
	 */
	void move(Input input, i32 offset) noexcept;

	/**
	 * Set the absolute value of all outputs bound to a specific input, without
	 * affecting the relative values for the current frame.
	 *
	 * \param input physical input to set.
	 * \param value signed integer value to set all bound outputs to. The
	 *        standard range of values is 0 (fully unpressed) to 32767 (fully
	 *        pressed).
	 *
	 * \sa press()
	 * \sa release()
	 * \sa move()
	 * \sa unbindAll()
	 * \sa resetAllInputs()
	 */
	void set(Input input, i32 value) noexcept;

	/**
	 * Reset the internal state of all inputs and outputs for both the current
	 * and previous frame.
	 *
	 * \note Does not affect bindings.
	 *
	 * \sa press()
	 * \sa release()
	 * \sa move()
	 * \sa set()
	 * \sa unbindAll()
	 */
	void resetAllInputs() noexcept;

	/**
	 * Set the mouse sensitivity coefficient.
	 *
	 * \param sensitivity new mouse sensitivity.
	 *
	 * \sa InputManagerOptions::mouseSensitivity
	 */
	void setMouseSensitivity(float sensitivity) noexcept;

	/**
	 * Set the controller left analog stick sensitivity coefficient.
	 *
	 * \param sensitivity new analog stick sensitivity.
	 *
	 * \sa InputManagerOptions::controllerLeftStickSensitivity
	 */
	void setControllerLeftStickSensitivity(float sensitivity) noexcept;

	/**
	 * Set the controller right analog stick sensitivity coefficient.
	 *
	 * \param sensitivity new analog stick sensitivity.
	 *
	 * \sa InputManagerOptions::controllerRightStickSensitivity
	 */
	void setControllerRightStickSensitivity(float sensitivity) noexcept;

	/**
	 * Set the controller left analog stick deadzone fraction.
	 *
	 * \param deadzone new analog stick deadzone.
	 *
	 * \sa InputManagerOptions::controllerLeftStickDeadzone
	 */
	void setControllerLeftStickDeadzone(float deadzone) noexcept;

	/**
	 * Set the controller right analog stick deadzone fraction.
	 *
	 * \param deadzone new analog stick deadzone.
	 *
	 * \sa InputManagerOptions::controllerRightStickDeadzone
	 */
	void setControllerRightStickDeadzone(float deadzone) noexcept;

	/**
	 * Set the controller left trigger deadzone fraction.
	 *
	 * \param deadzone new trigger deadzone.
	 *
	 * \sa InputManagerOptions::controllerLeftTriggerDeadzone
	 */
	void setControllerLeftTriggerDeadzone(float deadzone) noexcept;

	/**
	 * Set the controller right trigger deadzone fraction.
	 *
	 * \param deadzone new trigger deadzone.
	 *
	 * \sa InputManagerOptions::controllerRightTriggerDeadzone
	 */
	void setControllerRightTriggerDeadzone(float deadzone) noexcept;

	/**
	 * Set the touch finger motion sensitivity coefficient.
	 *
	 * \param sensitivity new touch finger motion sensitivity.
	 *
	 * \sa InputManagerOptions::touchMotionSensitivity
	 */
	void setTouchMotionSensitivity(float sensitivity) noexcept;

	/**
	 * Set the touch finger pressure deadzone fraction.
	 *
	 * \param deadzone new touch finger deadzone.
	 *
	 * \sa InputManagerOptions::touchPressureDeadzone
	 */
	void setTouchPressureDeadzone(float deadzone) noexcept;

	/**
	 * Check if this input manager has any active bindings for any input.
	 *
	 * \return true if there exists some input that is currently mapped to a set
	 *         of outputs, false otherwise.
	 */
	[[nodiscard]] bool hasAnyBindings() const noexcept;

	/**
	 * Get all active bindings of this input manager.
	 *
	 * \return an iterable input range of all of the bindings that currently
	 *         exist between a physical input and a set of abstract outputs.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa hasAnyBindings()
	 * \sa findBinding()
	 */
	[[nodiscard]] std::vector<Binding> getBindings() const;

	/**
	 * Get the set of outputs that a specific input is currently bound to.
	 *
	 * \param input physical input for which to search for a binding.
	 *
	 * \return if found, returns the set of bound outputs. Otherwise, returns
	 *         an empty optional.
	 *
	 * \sa getBindings()
	 */
	[[nodiscard]] std::optional<Outputs> findBinding(Input input) const noexcept;

	/**
	 * Get the latest known mouse position processed by the input manager.
	 *
	 * \return if the mouse has a known position, returns a 2D vector, in screen
	 *         coordinates (typically pixels), that represents it. Otherwise,
	 *         returns an empty optional.
	 *
	 * \note Instead of reading the state of the mouse directly, prefer to use
	 *       the getAbsoluteVector() or getRelativeVector() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a joystick, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa mouseJustMoved()
	 */
	[[nodiscard]] std::optional<vec2> getMousePosition() const noexcept;

	/**
	 * Check if the mouse just moved on the current frame.
	 *
	 * \return true if any mouse motion was processed in the current frame,
	 *         false otherwise.
	 *
	 * \note Instead of reading the state of the mouse directly, prefer to use
	 *       the isPressed(), justPressed() and justReleased() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a button, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa getMousePosition()
	 */
	[[nodiscard]] bool mouseJustMoved() const noexcept;

	/**
	 * Check if the mouse wheel was just scrolled horizontally on the current
	 * frame.
	 *
	 * \return true if any horizontal mouse wheel motion was processed in the
	 *         current frame, false otherwise.
	 *
	 * \note Instead of reading the state of the mouse directly, prefer to use
	 *       the isPressed(), justPressed() and justReleased() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a button, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa mouseWheelJustScrolledVertically()
	 */
	[[nodiscard]] bool mouseWheelJustScrolledHorizontally() const noexcept;

	/**
	 * Check if the mouse wheel was just scrolled vertically on the current
	 * frame.
	 *
	 * \return true if any vertical mouse wheel motion was processed in the
	 *         current frame, false otherwise.
	 *
	 * \note Instead of reading the state of the mouse directly, prefer to use
	 *       the isPressed(), justPressed() and justReleased() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a button, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa mouseWheelJustScrolledHorizontally()
	 */
	[[nodiscard]] bool mouseWheelJustScrolledVertically() const noexcept;

	/**
	 * Check if a controller is currently connected as an active input device.
	 *
	 * \return true if an active controller is connected, false otherwise.
	 */
	[[nodiscard]] bool isControllerConnected() const noexcept;

	/**
	 * Get the latest known position of the left analog stick of the connected
	 * controller, if there is one.
	 *
	 * \return if a controller is connected and its left analog stick has a
	 *         known position, returns a 2D vector with a magnitude between 0
	 *         and 1 that represents it. Otherwise, returns an empty optional.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the getAbsoluteVector() or getRelativeVector() functions with
	 *       an abstract output number whenever possible, since this can allow
	 *       the user to bind a different form of input, such as a mouse, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa controllerLeftStickJustMoved()
	 */
	[[nodiscard]] std::optional<vec2> getControllerLeftStickPosition() const noexcept;

	/**
	 * Get the latest known position of the right analog stick of the connected
	 * controller, if there is one.
	 *
	 * \return if a controller is connected and its right analog stick has a
	 *         known position, returns a 2D vector with a magnitude between 0
	 *         and 1 that represents it. Otherwise, returns an empty optional.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the getAbsoluteVector() or getRelativeVector() functions with
	 *       an abstract output number whenever possible, since this can allow
	 *       the user to bind a different form of input, such as a mouse, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa controllerRightStickJustMoved()
	 */
	[[nodiscard]] std::optional<vec2> getControllerRightStickPosition() const noexcept;

	/**
	 * Get the latest known position of the left trigger of the connected
	 * controller, if there is one.
	 *
	 * \return if a controller is connected and its left trigger has a known
	 *         position, returns a floating-point value between 0 and 1 that
	 *         represents it. Otherwise, returns an empty optional.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the getAbsoluteVector() or getRelativeVector() functions with
	 *       an abstract output number whenever possible, since this can allow
	 *       the user to bind a different form of input, such as a key, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa controllerLeftTriggerJustMoved()
	 */
	[[nodiscard]] std::optional<float> getControllerLeftTriggerPosition() const noexcept;

	/**
	 * Get the latest known position of the right trigger of the connected
	 * controller, if there is one.
	 *
	 * \return if a controller is connected and its right trigger has a known
	 *         position, returns a floating-point value between 0 and 1 that
	 *         represents it. Otherwise, returns an empty optional.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the getAbsoluteVector() or getRelativeVector() functions with
	 *       an abstract output number whenever possible, since this can allow
	 *       the user to bind a different form of input, such as a key, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa controllerRightTriggerJustMoved()
	 */
	[[nodiscard]] std::optional<float> getControllerRightTriggerPosition() const noexcept;

	/**
	 * Check if the controller left analog stick just moved on the current
	 * frame.
	 *
	 * \return true if any left analog stick motion was processed in the current
	 *         frame, false otherwise.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the isPressed(), justPressed() and justReleased() functions
	 *       with an abstract output number whenever possible, since this can
	 *       allow the user to bind a different form of input, such as a key, to
	 *       the control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa getControllerLeftStickPosition()
	 */
	[[nodiscard]] bool controllerLeftStickJustMoved() const noexcept;

	/**
	 * Check if the controller right analog stick just moved on the current
	 * frame.
	 *
	 * \return true if any right analog stick motion was processed in the
	 *         current frame, false otherwise.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the isPressed(), justPressed() and justReleased() functions
	 *       with an abstract output number whenever possible, since this can
	 *       allow the user to bind a different form of input, such as a key, to
	 *       the control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa getControllerRightStickPosition()
	 */
	[[nodiscard]] bool controllerRightStickJustMoved() const noexcept;

	/**
	 * Check if the controller left trigger just moved on the current frame.
	 *
	 * \return true if any left trigger motion was processed in the current
	 *         frame, false otherwise.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the isPressed(), justPressed() and justReleased() functions
	 *       with an abstract output number whenever possible, since this can
	 *       allow the user to bind a different form of input, such as a key, to
	 *       the control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa getControllerLeftTriggerPosition()
	 */
	[[nodiscard]] bool controllerLeftTriggerJustMoved() const noexcept;

	/**
	 * Check if the controller right trigger just moved on the current frame.
	 *
	 * \return true if any right trigger motion was processed in the current
	 *         frame, false otherwise.
	 *
	 * \note Instead of reading the state of the controller directly, prefer to
	 *       use the isPressed(), justPressed() and justReleased() functions
	 *       with an abstract output number whenever possible, since this can
	 *       allow the user to bind a different form of input, such as a key, to
	 *       the control instead, according to their preferences.
	 *
	 * \sa isControllerConnected()
	 * \sa getControllerRightTriggerPosition()
	 */
	[[nodiscard]] bool controllerRightTriggerJustMoved() const noexcept;

	/**
	 * Get the latest known touch finger position processed by the input
	 * manager.
	 *
	 * \return if the finger has a known position, returns a 2D vector, in
	 *         normalized coordinates [0, 1], that represents it. Otherwise,
	 *         returns an empty optional.
	 *
	 * \note Instead of reading the state of the finger directly, prefer to use
	 *       the getAbsoluteVector() or getRelativeVector() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a joystick, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa getTouchPressure()
	 * \sa touchJustMoved()
	 * \sa touchJustChangedPressure()
	 */
	[[nodiscard]] std::optional<vec2> getTouchPosition() const noexcept;

	/**
	 * Get the latest known touch finger pressure processed by the input
	 * manager.
	 *
	 * \return if the finger has a known pressure, returns a float in the range
	 *         [0, 1], that represents it. Otherwise, returns an empty optional.
	 *
	 * \note Instead of reading the state of the finger directly, prefer to use
	 *       the getAbsoluteVector() or getRelativeVector() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a trigger, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa getTouchPosition()
	 * \sa touchJustMoved()
	 * \sa touchJustChangedPressure()
	 */
	[[nodiscard]] std::optional<float> getTouchPressure() const noexcept;

	/**
	 * Check if the touch finger just moved on the current frame.
	 *
	 * \return true if any finger motion was processed in the current frame,
	 *         false otherwise.
	 *
	 * \note Instead of reading the state of the finger directly, prefer to use
	 *       the isPressed(), justPressed() and justReleased() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a button, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa getTouchPosition()
	 * \sa getTouchPressure()
	 * \sa touchJustChangedPressure()
	 */
	[[nodiscard]] bool touchJustMoved() const noexcept;

	/**
	 * Check if the touch finger just changed pressure on the current frame.
	 *
	 * \return true if any finger pressure change was processed in the current
	 *         frame, false otherwise.
	 *
	 * \note Instead of reading the state of the finger directly, prefer to use
	 *       the isPressed(), justPressed() and justReleased() functions with an
	 *       abstract output number whenever possible, since this can allow the
	 *       user to bind a different form of input, such as a button, to the
	 *       control instead, according to their preferences.
	 *
	 * \sa getTouchPosition()
	 * \sa getTouchPressure()
	 * \sa touchJustMoved()
	 */
	[[nodiscard]] bool touchJustChangedPressure() const noexcept;

	/**
	 * Get all output numbers for which at least one bound input is currently in
	 * a pressed state.
	 *
	 * \return the set of all currently active outputs.
	 *
	 * \sa getPreviousOutputs()
	 * \sa getJustPressedOutputs()
	 * \sa getJustReleasedOutputs()
	 * \sa isPressed()
	 */
	[[nodiscard]] Outputs getCurrentOutputs() const noexcept;

	/**
	 * Get all output numbers for which at least one bound input was in pressed
	 * state on the previous frame.
	 *
	 * \return the set of all active outputs from the previous frame.
	 *
	 * \sa getCurrentOutputs()
	 * \sa getJustPressedOutputs()
	 * \sa getJustReleasedOutputs()
	 */
	[[nodiscard]] Outputs getPreviousOutputs() const noexcept;

	/**
	 * Get all output numbers for which a press was triggered on the current
	 * frame.
	 *
	 * \return the set of outputs that are currently active, but were not active
	 *         on the previous frame, combined with all transient outputs from
	 *         the current frame.
	 *
	 * \sa getCurrentOutputs()
	 * \sa getPreviousOutputs()
	 * \sa getJustReleasedOutputs()
	 * \sa justPressed()
	 */
	[[nodiscard]] Outputs getJustPressedOutputs() const noexcept;

	/**
	 * Get all output numbers for which a release was triggered on the current
	 * frame.
	 *
	 * \return the set of outputs that are currently not active, but were active
	 *         on the previous frame, combined with all transient outputs from
	 *         the current frame.
	 *
	 * \sa getCurrentOutputs()
	 * \sa getPreviousOutputs()
	 * \sa getJustPressedOutputs()
	 * \sa justReleased()
	 */
	[[nodiscard]] Outputs getJustReleasedOutputs() const noexcept;

	/**
	 * Check if a specific output has any bound inputs which are currently in a
	 * pressed state.
	 *
	 * \param output valid output number between 0 (inclusive) and #OUTPUT_COUNT
	 *        (exclusive) to check the associated state of.
	 *
	 * \return true if the output is active, false otherwise.
	 *
	 * \remark Functionally equivalent to `getCurrentOutputs().test(output)`.
	 *
	 * \sa getCurrentOutputs()
	 * \sa justPressed()
	 * \sa justReleased()
	 * \sa isPressed(Input) const
	 */
	[[nodiscard]] bool isPressed(std::size_t output) const noexcept;

	/**
	 * Check if a specific output had a press triggered on the current frame.
	 *
	 * \param output valid output number between 0 (inclusive) and #OUTPUT_COUNT
	 *        (exclusive) to check the associated state of.
	 *
	 * \return true if a press of the output was triggered on the current frame,
	 *         false otherwise.
	 *
	 * \remark Functionally equivalent to
	 *         `getJustPressedOutputs().test(output)`.
	 *
	 * \sa getJustPressedOutputs()
	 * \sa isPressed()
	 * \sa justReleased()
	 * \sa justPressed(Input) const
	 */
	[[nodiscard]] bool justPressed(std::size_t output) const noexcept;

	/**
	 * Check if a specific output had a release triggered on the current frame.
	 *
	 * \param output valid output number between 0 (inclusive) and #OUTPUT_COUNT
	 *        (exclusive) to check the associated state of.
	 *
	 * \return true if a release of the output was triggered on the current
	 *         frame, false otherwise.
	 *
	 * \remark Functionally equivalent to
	 *         `getJustReleasedOutputs().test(output)`.
	 *
	 * \sa getJustReleasedOutputs()
	 * \sa isPressed()
	 * \sa justPressed()
	 * \sa justReleased(Input) const
	 */
	[[nodiscard]] bool justReleased(std::size_t output) const noexcept;

	/**
	 * Get the current raw total absolute value of a specific output, which
	 * consists of the accumulated contributions from all of its bound inputs.
	 *
	 * \param output valid output number between 0 (inclusive) and #OUTPUT_COUNT
	 *        (exclusive) to check the associated value of.
	 *
	 * \return the accumulated absolute signed integer value of the given
	 *         output, usually between 0 (fully unpressed) and 32767 (fully
	 *         pressed) when only a single input is controlling it, though it
	 *         could be any value.
	 *
	 * \note For most situations, using getAbsoluteVector() instead is
	 *       preferable instead since it provides a more intuitive range of
	 *       floating-point values compared to the raw signed integer value of
	 *       this function.
	 *
	 * \sa getRelativeValue()
	 * \sa getAbsoluteVector()
	 * \sa getRelativeVector()
	 */
	[[nodiscard]] i32 getAbsoluteValue(std::size_t output) const noexcept;

	/**
	 * Get the current raw total relative value of a specific output, which
	 * consists of the accumulated contributions from all of its bound inputs.
	 *
	 * \param output valid output number between 0 (inclusive) and #OUTPUT_COUNT
	 *        (exclusive) to check the associated value of.
	 *
	 * \return the accumulated relative signed integer offset of the given
	 *         output since the previous frame, usually between -32767 and 32767
	 *         when only a single input is controlling it, though it could be
	 *         any value.
	 *
	 * \note For most situations, using getRelativeVector() instead is
	 *       preferable instead since it provides a more intuitive range of
	 *       floating-point values compared to the raw signed integer value of
	 *       this function.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getAbsoluteVector()
	 * \sa getRelativeVector()
	 */
	[[nodiscard]] i32 getRelativeValue(std::size_t output) const noexcept;

	/**
	 * Get the current scaled absolute value of a specific output in a single
	 * direction, which consists of the accumulated contributions from all of
	 * its bound inputs.
	 *
	 * \param outputPositive valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the resulting value.
	 *
	 * \return the accumulated scaled absolute floating-point value of the given
	 *         output as a 1D vector (scalar), usually between 0 (fully
	 *         unpressed) and 1 (fully presssed) when only a single input is
	 *         controlling it, though it could be any non-negative value.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getRelativeVector()
	 */
	[[nodiscard]] float getAbsoluteVector(std::size_t outputPositive) const noexcept;

	/**
	 * Get the current scaled relative value of a specific output in a single
	 * direction, which consists of the accumulated contributions from all of
	 * its bound inputs.
	 *
	 * \param outputPositive valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the resulting value.
	 *
	 * \return the accumulated scaled relative floating-point offset of the
	 *         given output since the previous frame as a 1D vector (scalar),
	 *         usually between -1 and 1 when only a single input is controlling
	 *         it, though it could be any non-negative value.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getAbsoluteVector()
	 */
	[[nodiscard]] float getRelativeVector(std::size_t outputPositive) const noexcept;

	/**
	 * Get the current combined scaled absolute value of two specific outputs in
	 * opposite directions, which consist of the accumulated contributions from
	 * all of their bound inputs.
	 *
	 * \param outputNegative valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the resulting value.
	 * \param outputPositive valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the resulting value.
	 *
	 * \return the accumulated scaled absolute floating-point value of the given
	 *         outputs as a 1D vector (scalar), usually between -1 and 1 when
	 *         only a single input is controlling each direction, though it
	 *         could be any value.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getRelativeVector()
	 */
	[[nodiscard]] float getAbsoluteVector(std::size_t outputNegative, std::size_t outputPositive) const noexcept;

	/**
	 * Get the current combined scaled relative value of two specific outputs in
	 * opposite directions, which consist of the accumulated contributions from
	 * all of their bound inputs.
	 *
	 * \param outputNegative valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the resulting value.
	 * \param outputPositive valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the resulting value.
	 *
	 * \return the accumulated scaled relative floating-point offset of the
	 *         given outputs since the previous frame as a 1D vector (scalar),
	 *         usually between -1 and 1 when only a single input is controlling
	 *         each direction, though it could be any value.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getAbsoluteVector()
	 */
	[[nodiscard]] float getRelativeVector(std::size_t outputNegative, std::size_t outputPositive) const noexcept;

	/**
	 * Get the current combined scaled absolute value of four specific outputs
	 * in orthogonal directions, which consist of the accumulated contributions
	 * from all of their bound inputs.
	 *
	 * \param outputNegativeX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the x component of the resulting vector.
	 * \param outputPositiveX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the x component of the resulting vector.
	 * \param outputNegativeY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the y component of the resulting vector.
	 * \param outputPositiveY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the y component of the resulting vector.
	 *
	 * \return the accumulated scaled absolute floating-point value of the given
	 *         outputs as a 2D vector, where each component is usually between
	 *         -1 and 1 when only a single input is controlling each direction,
	 *         though it could be any value.
	 *
	 * \remark This function is useful for controlling 2D movement based on four
	 *         directional inputs such as the arrow keys, a D-pad or a joystick.
	 *         When used for this purpose, it might be necessary to clamp the
	 *         length of the vector to a length of 1 before using it, to make
	 *         sure that the user cannot achieve a higher speed than intended by
	 *         binding multiple inputs to one direction and pressing them at the
	 *         same time such that they increase the accumulated value above 1.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getRelativeVector()
	 */
	[[nodiscard]] vec2 getAbsoluteVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept;

	/**
	 * Get the current combined scaled relative value of four specific outputs
	 * in orthogonal directions, which consist of the accumulated contributions
	 * from all of their bound inputs.
	 *
	 * \param outputNegativeX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the x component of the resulting vector.
	 * \param outputPositiveX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the x component of the resulting vector.
	 * \param outputNegativeY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the y component of the resulting vector.
	 * \param outputPositiveY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the y component of the resulting vector.
	 *
	 * \return the accumulated scaled relative floating-point offset of the
	 *         given outputs since the previous frame as a 2D vector, where each
	 *         component is usually between -1 and 1 when only a single input is
	 *         controlling each direction, though it could be any value.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getAbsoluteVector()
	 */
	[[nodiscard]] vec2 getRelativeVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY) const noexcept;

	/**
	 * Get the current combined scaled absolute value of six specific outputs
	 * in orthogonal directions, which consist of the accumulated contributions
	 * from all of their bound inputs.
	 *
	 * \param outputNegativeX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the x component of the resulting vector.
	 * \param outputPositiveX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the x component of the resulting vector.
	 * \param outputNegativeY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the y component of the resulting vector.
	 * \param outputPositiveY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the y component of the resulting vector.
	 * \param outputNegativeZ valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the z component of the resulting vector.
	 * \param outputPositiveZ valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the z component of the resulting vector.
	 *
	 * \return the accumulated scaled absolute floating-point value of the given
	 *         outputs as a 3D vector, where each component is usually between
	 *         -1 and 1 when only a single input is controlling each direction,
	 *         though it could be any value.
	 *
	 * \remark This function is useful for controlling 3D translation based on
	 *         six directional inputs such as the arrow keys combined with two
	 *         extra keys for vertical motion. When used for this purpose, it
	 *         might be necessary to clamp the length of the vector to a length
	 *         of 1 before using it, to make sure that the user cannot achieve a
	 *         higher speed than intended by binding multiple inputs to one
	 *         direction and pressing them at the same time such that they
	 *         increase the accumulated value above 1.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getRelativeVector()
	 */
	[[nodiscard]] vec3 getAbsoluteVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY,
		std::size_t outputNegativeZ, std::size_t outputPositiveZ) const noexcept;

	/**
	 * Get the current combined scaled relative value of six specific outputs
	 * in orthogonal directions, which consist of the accumulated contributions
	 * from all of their bound inputs.
	 *
	 * \param outputNegativeX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the x component of the resulting vector.
	 * \param outputPositiveX valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the x component of the resulting vector.
	 * \param outputNegativeY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the y component of the resulting vector.
	 * \param outputPositiveY valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the y component of the resulting vector.
	 * \param outputNegativeZ valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the negative contribution to
	 *        the z component of the resulting vector.
	 * \param outputPositiveZ valid output number between 0 (inclusive) and
	 *        #OUTPUT_COUNT (exclusive) to use for the positive contribution to
	 *        the z component of the resulting vector.
	 *
	 * \return the accumulated scaled relative floating-point offset of the
	 *         given outputs since the previous frame as a 3D vector, where each
	 *         component is usually between -1 and 1 when only a single input is
	 *         controlling each direction, though it could be any value.
	 *
	 * \sa getAbsoluteValue()
	 * \sa getRelativeValue()
	 * \sa getAbsoluteVector()
	 */
	[[nodiscard]] vec3 getRelativeVector(std::size_t outputNegativeX, std::size_t outputPositiveX, std::size_t outputNegativeY, std::size_t outputPositiveY,
		std::size_t outputNegativeZ, std::size_t outputPositiveZ) const noexcept;

	/**
	 * Check if a specific input is currently in a pressed state.
	 *
	 * \param input valid input value to check the associated state of.
	 *
	 * \return true if the input is pressed, false otherwise.
	 *
	 * \note Instead of checking the state of a physical input, prefer to use
	 *       the version of this function that takes an abstract output number
	 *       whenever possible, since this can allow the user to bind a
	 *       different input to the control instead, according to their
	 *       preferences.
	 *
	 * \sa isPressed(std::size_t) const
	 * \sa justPressed()
	 * \sa justReleased()
	 */
	[[nodiscard]] bool isPressed(Input input) const noexcept;

	/**
	 * Check if a specific input had a press triggered on the current frame.
	 *
	 * \param input valid input value to check the associated state of.
	 *
	 * \return true if a press of the input was triggered on the current frame,
	 *         false otherwise.
	 *
	 * \note Instead of checking the state of a physical input, prefer to use
	 *       the version of this function that takes an abstract output number
	 *       whenever possible, since this can allow the user to bind a
	 *       different input to the control instead, according to their
	 *       preferences.
	 *
	 * \sa justPressed(std::size_t) const
	 * \sa getJustPressedOutputs()
	 * \sa isPressed()
	 * \sa justReleased()
	 */
	[[nodiscard]] bool justPressed(Input input) const noexcept;

	/**
	 * Check if a specific input had a release triggered on the current frame.
	 *
	 * \param input valid input value to check the associated state of.
	 *
	 * \return true if a release of the input was triggered on the current
	 *         frame, false otherwise.
	 *
	 * \note Instead of checking the state of a physical input, prefer to use
	 *       the version of this function that takes an abstract output number
	 *       whenever possible, since this can allow the user to bind a
	 *       different input to the control instead, according to their
	 *       preferences.
	 *
	 * \sa justReleased(std::size_t) const
	 * \sa getJustReleasedOutputs()
	 * \sa isPressed()
	 * \sa justPressed()
	 */
	[[nodiscard]] bool justReleased(Input input) const noexcept;

	/**
	 * Like bind(Input, Outputs), but accepts a pack of "actions" of any enum
	 * type, which are interpreted as corresponding to the output numbers equal
	 * to their underlying values.
	 *
	 * \sa bind(Input, Outputs)
	 */
	template <typename... Actions>
	void bind(Input input, Actions... actions) requires(std::is_enum_v<Actions> && ...) {
		Outputs outputs{};
		(outputs.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Actions>>(actions))), ...);
		bind(input, outputs);
	}

	/**
	 * Like addBinding(Input, Outputs), but accepts a pack of "actions" of any
	 * enum type, which are interpreted as corresponding to the output numbers
	 * equal to their underlying values.
	 *
	 * \sa addBinding(Input, Outputs)
	 */
	template <typename... Actions>
	void addBinding(Input input, Actions... actions) requires(std::is_enum_v<Actions> && ...) {
		Outputs outputs{};
		(outputs.set(static_cast<std::size_t>(static_cast<std::underlying_type_t<Actions>>(actions))), ...);
		addBinding(input, outputs);
	}

	/**
	 * Like isPressed(std::size_t) const, but accepts an "action" of any enum
	 * type, which is interpreted as corresponding to the output number equal to
	 * its underlying value.
	 *
	 * \sa isPressed(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] bool isPressed(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return isPressed(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	/**
	 * Like justPressed(std::size_t) const, but accepts an "action" of any enum
	 * type, which is interpreted as corresponding to the output number equal to
	 * its underlying value.
	 *
	 * \sa justPressed(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] bool justPressed(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return justPressed(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	/**
	 * Like justReleased(std::size_t) const, but accepts an "action" of any enum
	 * type, which is interpreted as corresponding to the output number equal to
	 * its underlying value.
	 *
	 * \sa justReleased(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] bool justReleased(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return justReleased(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	/**
	 * Like getAbsoluteValue(std::size_t) const, but accepts an "action" of any
	 * enum type, which is interpreted as corresponding to the output number
	 * equal to its underlying value.
	 *
	 * \sa getAbsoluteValue(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] i32 getAbsoluteValue(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteValue(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	/**
	 * Like getRelativeValue(std::size_t) const, but accepts an "action" of any
	 * enum type, which is interpreted as corresponding to the output number
	 * equal to its underlying value.
	 *
	 * \sa getRelativeValue(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] i32 getRelativeValue(Action action) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeValue(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(action)));
	}

	/**
	 * Like getAbsoluteVector(std::size_t) const, but accepts an "action" of any
	 * enum type, which is interpreted as corresponding to the output number
	 * equal to its underlying value.
	 *
	 * \sa getAbsoluteVector(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] float getAbsoluteVector(Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	/**
	 * Like getRelativeVector(std::size_t) const, but accepts an "action" of any
	 * enum type, which is interpreted as corresponding to the output number
	 * equal to its underlying value.
	 *
	 * \sa getRelativeVector(std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] float getRelativeVector(Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	/**
	 * Like getAbsoluteVector(std::size_t, std::size_t) const, but accepts
	 * "actions" of any enum type, which are interpreted as corresponding to
	 * output numbers equal to their underlying values.
	 *
	 * \sa getAbsoluteVector(std::size_t, std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] float getAbsoluteVector(Action actionNegative, Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegative)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	/**
	 * Like getRelativeVector(std::size_t, std::size_t) const, but accepts
	 * "actions" of any enum type, which are interpreted as corresponding to
	 * output numbers equal to their underlying values.
	 *
	 * \sa getRelativeVector(std::size_t, std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] float getRelativeVector(Action actionNegative, Action actionPositive) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegative)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositive)));
	}

	/**
	 * Like getAbsoluteVector(std::size_t, std::size_t, std::size_t,
	 * std::size_t) const, but accepts "actions" of any enum type, which are
	 * interpreted as corresponding to the output numbers equal to their
	 * underlying values.
	 *
	 * \sa getAbsoluteVector(std::size_t, std::size_t, std::size_t,
	 *     std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] vec2 getAbsoluteVector(Action actionNegativeX, Action actionPositiveX, Action actionNegativeY, Action actionPositiveY) const noexcept
		requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveY)));
	}

	/**
	 * Like getRelativeVector(std::size_t, std::size_t, std::size_t,
	 * std::size_t) const, but accepts "actions" of any enum type, which are
	 * interpreted as corresponding to the output numbers equal to their
	 * underlying values.
	 *
	 * \sa getRelativeVector(std::size_t, std::size_t, std::size_t,
	 *     std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] vec2 getRelativeVector(Action actionNegativeX, Action actionPositiveX, Action actionNegativeY, Action actionPositiveY) const noexcept
		requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveY)));
	}

	/**
	 * Like getAbsoluteVector(std::size_t, std::size_t, std::size_t,
	 * std::size_t, std::size_t, std::size_t) const, but accepts "actions" of
	 * any enum type, which are interpreted as corresponding to the output
	 * numbers equal to their underlying values.
	 *
	 * \sa getAbsoluteVector(std::size_t, std::size_t, std::size_t,
	 *     std::size_t, std::size_t, std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] vec3 getAbsoluteVector(Action actionNegativeX, Action actionPositiveX, Action actionNegativeY, Action actionPositiveY, Action actionNegativeZ,
		Action actionPositiveZ) const noexcept requires(std::is_enum_v<Action>) {
		return getAbsoluteVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeZ)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveZ)));
	}

	/**
	 * Like getRelativeVector(std::size_t, std::size_t, std::size_t,
	 * std::size_t, std::size_t, std::size_t) const, but accepts "actions" of
	 * any enum type, which are interpreted as corresponding to the output
	 * numbers equal to their underlying values.
	 *
	 * \sa getRelativeVector(std::size_t, std::size_t, std::size_t,
	 *     std::size_t, std::size_t, std::size_t) const
	 */
	template <typename Action>
	[[nodiscard]] vec3 getRelativeVector(Action actionNegativeX, Action actionPositiveX, Action actionNegativeY, Action actionPositiveY, Action actionNegativeZ,
		Action actionPositiveZ) const noexcept requires(std::is_enum_v<Action>) {
		return getRelativeVector(static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveX)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveY)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionNegativeZ)),
			static_cast<std::size_t>(static_cast<std::underlying_type_t<Action>>(actionPositiveZ)));
	}

private:
	struct ControllerDeleter {
		void operator()(void* handle) const noexcept;
	};

	using Controller = UniqueHandle<void*, ControllerDeleter>;

	void setMousePosition(vec2 position, vec2 relativeMotion) noexcept;
	void scrollMouseWheelHorizontally(float offset) noexcept;
	void scrollMouseWheelVertically(float offset) noexcept;

	void setControllerLeftStickPosition(vec2 position) noexcept;
	void setControllerRightStickPosition(vec2 position) noexcept;
	void setControllerLeftTriggerPosition(float position) noexcept;
	void setControllerRightTriggerPosition(float position) noexcept;

	void setTouchPosition(vec2 position) noexcept;
	void setTouchPressure(float pressure) noexcept;

	InputManagerOptions options;
	std::unordered_map<Input, Outputs> bindings{};
	std::optional<vec2> mousePosition{};
	Controller controller{};
	std::optional<vec2> controllerLeftStickPosition{};
	std::optional<vec2> controllerRightStickPosition{};
	std::optional<float> controllerLeftTriggerPosition{};
	std::optional<float> controllerRightTriggerPosition{};
	std::optional<vec2> touchPosition{};
	std::optional<float> touchPressure{};
	Outputs currentPersistentOutputs{};
	Outputs previousPersistentOutputs{};
	Outputs transientOutputs{};
	std::array<i32, OUTPUT_COUNT> outputAbsoluteValues{};
	std::array<i32, OUTPUT_COUNT> outputRelativeValues{};
	std::array<u8, OUTPUT_COUNT> outputPersistentPresses{};
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
	bool touchTransientMotion = false;
	bool touchTransientPressure = false;
};

} // namespace donut::events

#endif
