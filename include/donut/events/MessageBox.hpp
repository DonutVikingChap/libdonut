#ifndef DONUT_EVENTS_MESSAGE_BOX_HPP
#define DONUT_EVENTS_MESSAGE_BOX_HPP

namespace donut::events {

/**
 * Utility class for simple message boxes to be delivered to the user through
 * the host environment.
 */
class MessageBox {
public:
	/**
	 * Type of message contained in a message box.
	 */
	enum class Type {
		ERROR_MESSAGE,   ///< Indicates that an error occured.
		WARNING_MESSAGE, ///< Warns the user about a potential error.
		INFO_MESSAGE,    ///< Provides general information.
	};

	/**
	 * Display a simple message box that blocks execution on the current thread
	 * until the user presses OK.
	 *
	 * \param type type of message shown in the box. This may be reflected in
	 *        the styling of the box.
	 * \param title non-owning read-only pointer to a UTF-8 string containing
	 *        the window title of the message box. Must not be nullptr.
	 * \param message non-owning read-only pointer to a UTF-8 string containing
	 *        the main message to show in the box. Must not be nullptr.
	 *
	 * \throws events::Error on failure to show the message box.
	 * \throws std::bad_alloc on allocation failure.
	 */
	static void show(Type type, const char* title, const char* message);

	/** Message box objects are not intended to be constructed directly. */
	MessageBox() = delete;
};

} // namespace donut::events

#endif
