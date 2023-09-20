#ifndef DONUT_EVENTS_EVENT_PUMP_HPP
#define DONUT_EVENTS_EVENT_PUMP_HPP

#include <donut/math.hpp>

#include <span>   // std::span
#include <string> // std::string
#include <vector> // std::vector

namespace donut::events {

struct Event; // Forward declaration, to avoid including Event.hpp.

/**
 * Persistent system for polling Event data and other user input from the host
 * environment on demand.
 *
 * The latest events are stored in a buffer that can be accessed until the next
 * time events are polled. The main intended usage pattern for this is to call
 * pollEvents() once at the start of each Application frame and then access the
 * event buffer immediately or throughout the rest of the frame as necessary.
 */
class EventPump {
public:
	/**
	 * Construct an event pump.
	 *
	 * \throws events::Error on failure to initialize the required global
	 *         subsystems.
	 */
	EventPump();

	/** Destructor. */
	~EventPump();

	/** Copying an event pump is not allowed, since it manages global state. */
	EventPump(const EventPump&) = delete;

	/** Moving an event pump is not allowed, since it manages global state. */
	EventPump(EventPump&&) = delete;

	/** Copying an event pump is not allowed, since it manages global state. */
	EventPump& operator=(const EventPump&) = delete;

	/** Moving an event pump is not allowed, since it manages global state. */
	EventPump& operator=(EventPump&&) = delete;

	/**
	 * Poll events from the environment and update the internal event buffer.
	 *
	 * \return a non-owning read-only view over the polled events, stored in the
	 *         internal event buffer, which is valid until the next call to
	 *         pollEvents() or until the event pump is destroyed, whichever
	 *         happens first.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 *
	 * \sa getLatestPolledEvents()
	 */
	std::span<const Event> pollEvents();

	/**
	 * Enable or disable relative mouse mode.
	 *
	 * When relative mouse mode is enabled, it hides the mouse cursor, locks it
	 * to the window and causes all further mouse motion to be provided as
	 * relative motion events until it is disabled again.
	 *
	 * \param relativeMouseMode true to enable relative mouse mode, false to
	 *        disable.
	 *
	 * \throws events::Error on failure to set the relative mouse mode.
	 */
	void setRelativeMouseMode(bool relativeMouseMode);

	/**
	 * Set the input rectangle for text input.
	 *
	 * \param offset offset of the input rectangle, in screen coordinates.
	 * \param size size of the input rectangle, in screen coordinates.
	 *
	 * \sa startTextInput()
	 * \sa stopTextInput()
	 */
	void setTextInputRectangle(ivec2 offset, ivec2 size);

	/**
	 * Start accepting text input events in the current text input rectangle.
	 *
	 * \sa setTextInputRectangle()
	 * \sa stopTextInput()
	 */
	void startTextInput();

	/**
	 * Stop accepting text input events.
	 *
	 * \sa setTextInputRectangle()
	 * \sa startTextInput()
	 */
	void stopTextInput();

	/**
	 * Get the latest events in the internal event buffer that were polled using
	 * pollEvents().
	 *
	 * \return a non-owning read-only view over the polled events, stored in the
	 *         internal event buffer, which is valid until the next call to
	 *         pollEvents() or until the event pump is destroyed, whichever
	 *         happens first.
	 *
	 * \sa pollEvents()
	 */
	[[nodiscard]] std::span<const Event> getLatestPolledEvents() const noexcept;

	/**
	 * Get the current text contained in the clipboard.
	 *
	 * \return the text in the clipboard.
	 *
	 * \throws std::bad_alloc on allocation failure.
	 */
	[[nodiscard]] std::string getClipboardText() const;

	/**
	 * Check if the application supports a screen keyboard.
	 *
	 * \return true if a screen keyboard is supported, false otherwise.
	 *
	 * \sa graphics::Window::isScreenKeyboardShown()
	 */
	[[nodiscard]] bool hasScreenKeyboardSupport() const noexcept;

private:
	std::vector<Event> events;
};

} // namespace donut::events

#endif
