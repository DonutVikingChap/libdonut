#ifndef DONUT_APPLICATION_INPUT_HPP
#define DONUT_APPLICATION_INPUT_HPP

#include <cstddef>     // std::size_t
#include <cstdint>     // std::uint8_t
#include <optional>    // std::optional
#include <string_view> // std::string_view

namespace donut {
namespace application {

// clang-format off
//  X(name,                                 id,                                     str                     )
#define DONUT_ENUM_INPUTS(X) \
	X(KEY_A,                                "key_a",                                "A"                     ) /**< Keyboard A key. */ \
	X(KEY_B,                                "key_b",                                "B"                     ) /**< Keyboard B key. */ \
	X(KEY_C,                                "key_c",                                "C"                     ) /**< Keyboard C key. */ \
	X(KEY_D,                                "key_d",                                "D"                     ) /**< Keyboard D key. */ \
	X(KEY_E,                                "key_e",                                "E"                     ) /**< Keyboard E key. */ \
	X(KEY_F,                                "key_f",                                "F"                     ) /**< Keyboard F key. */ \
	X(KEY_G,                                "key_g",                                "G"                     ) /**< Keyboard G key. */ \
	X(KEY_H,                                "key_h",                                "H"                     ) /**< Keyboard H key. */ \
	X(KEY_I,                                "key_i",                                "I"                     ) /**< Keyboard I key. */ \
	X(KEY_J,                                "key_j",                                "J"                     ) /**< Keyboard J key. */ \
	X(KEY_K,                                "key_k",                                "K"                     ) /**< Keyboard K key. */ \
	X(KEY_L,                                "key_l",                                "L"                     ) /**< Keyboard L key. */ \
	X(KEY_M,                                "key_m",                                "M"                     ) /**< Keyboard M key. */ \
	X(KEY_N,                                "key_n",                                "N"                     ) /**< Keyboard N key. */ \
	X(KEY_O,                                "key_o",                                "O"                     ) /**< Keyboard O key. */ \
	X(KEY_P,                                "key_p",                                "P"                     ) /**< Keyboard P key. */ \
	X(KEY_Q,                                "key_q",                                "Q"                     ) /**< Keyboard Q key. */ \
	X(KEY_R,                                "key_r",                                "R"                     ) /**< Keyboard R key. */ \
	X(KEY_S,                                "key_s",                                "S"                     ) /**< Keyboard S key. */ \
	X(KEY_T,                                "key_t",                                "T"                     ) /**< Keyboard T key. */ \
	X(KEY_U,                                "key_u",                                "U"                     ) /**< Keyboard U key. */ \
	X(KEY_V,                                "key_v",                                "V"                     ) /**< Keyboard V key. */ \
	X(KEY_W,                                "key_w",                                "W"                     ) /**< Keyboard W key. */ \
	X(KEY_X,                                "key_x",                                "X"                     ) /**< Keyboard X key. */ \
	X(KEY_Y,                                "key_y",                                "Y"                     ) /**< Keyboard Y key. */ \
	X(KEY_Z,                                "key_z",                                "Z"                     ) /**< Keyboard Z key. */ \
	X(KEY_1,                                "key_1",                                "1"                     ) /**< Keyboard 1 key. */ \
	X(KEY_2,                                "key_2",                                "2"                     ) /**< Keyboard 2 key. */ \
	X(KEY_3,                                "key_3",                                "3"                     ) /**< Keyboard 3 key. */ \
	X(KEY_4,                                "key_4",                                "4"                     ) /**< Keyboard 4 key. */ \
	X(KEY_5,                                "key_5",                                "5"                     ) /**< Keyboard 5 key. */ \
	X(KEY_6,                                "key_6",                                "6"                     ) /**< Keyboard 6 key. */ \
	X(KEY_7,                                "key_7",                                "7"                     ) /**< Keyboard 7 key. */ \
	X(KEY_8,                                "key_8",                                "8"                     ) /**< Keyboard 8 key. */ \
	X(KEY_9,                                "key_9",                                "9"                     ) /**< Keyboard 9 key. */ \
	X(KEY_0,                                "key_0",                                "0"                     ) /**< Keyboard 0 key. */ \
	X(KEY_ESCAPE,                           "key_escape",                           "Esc"                   ) /**< Keyboard escape key. */ \
	X(KEY_LCTRL,                            "key_lctrl",                            "LCtrl"                 ) /**< Keyboard left control key. */ \
	X(KEY_RCTRL,                            "key_rctrl",                            "RCtrl"                 ) /**< Keyboard right control key. */ \
	X(KEY_LSHIFT,                           "key_lshift",                           "LShift"                ) /**< Keyboard left shift key. */ \
	X(KEY_RSHIFT,                           "key_rshift",                           "RShift"                ) /**< Keyboard right shift key. */ \
	X(KEY_LALT,                             "key_lalt",                             "LAlt"                  ) /**< Keyboard left alt key. */ \
	X(KEY_RALT,                             "key_ralt",                             "RAlt"                  ) /**< Keyboard right alt key. */ \
	X(KEY_MENU,                             "key_menu",                             "Menu"                  ) /**< Keyboard menu key. */ \
	X(KEY_LEFT_BRACKET,                     "key_left_bracket",                      "["                    ) /**< Keyboard [ key. */ \
	X(KEY_RIGHT_BRACKET,                    "key_right_bracket",                     "]"                    ) /**< Keyboard ] key. */ \
	X(KEY_SEMICOLON,                        "key_semicolon",                         ";"                    ) /**< Keyboard ; key. */ \
	X(KEY_COMMA,                            "key_comma",                             ","                    ) /**< Keyboard , key. */ \
	X(KEY_PERIOD,                           "key_period",                            "."                    ) /**< Keyboard .\ key. */ \
	X(KEY_APOSTROPHE,                       "key_apostrophe",                        "'"                    ) /**< Keyboard ' key. */ \
	X(KEY_SLASH,                            "key_slash",                             "/"                    ) /**< Keyboard / key. */ \
	X(KEY_BACKSLASH,                        "key_backslash",                         "\\"                   ) /**< Keyboard \\ key. */ \
	X(KEY_GRAVE,                            "key_grave",                             "`"                    ) /**< Keyboard ` key. */ \
	X(KEY_EQUALS,                           "key_equals",                            "="                    ) /**< Keyboard = key. */ \
	X(KEY_MINUS,                            "key_minus",                             "-"                    ) /**< Keyboard \- key. */ \
	X(KEY_SPACE,                            "key_space",                            "Space"                 ) /**< Keyboard space key. */ \
	X(KEY_RETURN,                           "key_return",                           "Return"                ) /**< Keyboard return key. */ \
	X(KEY_BACKSPACE,                        "key_backspace",                        "Backspace"             ) /**< Keyboard backspace key. */ \
	X(KEY_TAB,                              "key_tab",                              "Tab"                   ) /**< Keyboard tab key. */ \
	X(KEY_PAGE_UP,                          "key_page_up",                          "Pgup"                  ) /**< Keyboard page up key. */ \
	X(KEY_PAGE_DOWN,                        "key_page_down",                        "Pgdn"                  ) /**< Keyboard page down key. */ \
	X(KEY_END,                              "key_end",                              "End"                   ) /**< Keyboard end key. */ \
	X(KEY_HOME,                             "key_home",                             "Home"                  ) /**< Keyboard home key. */ \
	X(KEY_INSERT,                           "key_insert",                           "Insert"                ) /**< Keyboard insert key. */ \
	X(KEY_DELETE,                           "key_delete",                           "Delete"                ) /**< Keyboard delete key. */ \
	X(KEY_ARROW_UP,                         "key_arrow_up",                         "Up Arrow"              ) /**< Keyboard up arrow key. */ \
	X(KEY_ARROW_DOWN,                       "key_arrow_down",                       "Down Arrow"            ) /**< Keyboard down arrow key. */ \
	X(KEY_ARROW_LEFT,                       "key_arrow_left",                       "Left Arrow"            ) /**< Keyboard left arrow key. */ \
	X(KEY_ARROW_RIGHT,                      "key_arrow_right",                      "Right Arrow"           ) /**< Keyboard right arrow key. */ \
	X(KEY_NUMPAD_PLUS,                      "key_numpad_plus",                      "Numpad +"              ) /**< Keyboard numpad + key. */ \
	X(KEY_NUMPAD_MINUS,                     "key_numpad_minus",                     "Numpad -"              ) /**< Keyboard numpad - key. */ \
	X(KEY_NUMPAD_MULTIPLY,                  "key_numpad_multiply",                  "Numpad *"              ) /**< Keyboard numpad * key. */ \
	X(KEY_NUMPAD_DIVIDE,                    "key_numpad_divide",                    "Numpad /"              ) /**< Keyboard numpad / key. */ \
	X(KEY_NUMPAD_1,                         "key_numpad_1",                         "Numpad 1"              ) /**< Keyboard numpad 1 key. */ \
	X(KEY_NUMPAD_2,                         "key_numpad_2",                         "Numpad 2"              ) /**< Keyboard numpad 2 key. */ \
	X(KEY_NUMPAD_3,                         "key_numpad_3",                         "Numpad 3"              ) /**< Keyboard numpad 3 key. */ \
	X(KEY_NUMPAD_4,                         "key_numpad_4",                         "Numpad 4"              ) /**< Keyboard numpad 4 key. */ \
	X(KEY_NUMPAD_5,                         "key_numpad_5",                         "Numpad 5"              ) /**< Keyboard numpad 5 key. */ \
	X(KEY_NUMPAD_6,                         "key_numpad_6",                         "Numpad 6"              ) /**< Keyboard numpad 6 key. */ \
	X(KEY_NUMPAD_7,                         "key_numpad_7",                         "Numpad 7"              ) /**< Keyboard numpad 7 key. */ \
	X(KEY_NUMPAD_8,                         "key_numpad_8",                         "Numpad 8"              ) /**< Keyboard numpad 8 key. */ \
	X(KEY_NUMPAD_9,                         "key_numpad_9",                         "Numpad 9"              ) /**< Keyboard numpad 9 key. */ \
	X(KEY_NUMPAD_0,                         "key_numpad_0",                         "Numpad 0"              ) /**< Keyboard numpad 0 key. */ \
	X(KEY_F1,                               "key_f1",                               "F1"                    ) /**< Keyboard F1 key. */ \
	X(KEY_F2,                               "key_f2",                               "F2"                    ) /**< Keyboard F2 key. */ \
	X(KEY_F3,                               "key_f3",                               "F3"                    ) /**< Keyboard F3 key. */ \
	X(KEY_F4,                               "key_f4",                               "F4"                    ) /**< Keyboard F4 key. */ \
	X(KEY_F5,                               "key_f5",                               "F5"                    ) /**< Keyboard F5 key. */ \
	X(KEY_F6,                               "key_f6",                               "F6"                    ) /**< Keyboard F6 key. */ \
	X(KEY_F7,                               "key_f7",                               "F7"                    ) /**< Keyboard F7 key. */ \
	X(KEY_F8,                               "key_f8",                               "F8"                    ) /**< Keyboard F8 key. */ \
	X(KEY_F9,                               "key_f9",                               "F9"                    ) /**< Keyboard F9 key. */ \
	X(KEY_F10,                              "key_f10",                              "F10"                   ) /**< Keyboard F10 key. */ \
	X(KEY_F11,                              "key_f11",                              "F11"                   ) /**< Keyboard F11 key. */ \
	X(KEY_F12,                              "key_f12",                              "F12"                   ) /**< Keyboard F12 key. */ \
	X(KEY_F13,                              "key_f13",                              "F13"                   ) /**< Keyboard F13 key. */ \
	X(KEY_F14,                              "key_f14",                              "F14"                   ) /**< Keyboard F14 key. */ \
	X(KEY_F15,                              "key_f15",                              "F15"                   ) /**< Keyboard F15 key. */ \
	X(KEY_PRINT_SCREEN,                     "key_print_screen",                     "PrtSc"                 ) /**< Keyboard PrtSc key. */ \
	X(KEY_SCROLL_LOCK,                      "key_scroll_lock",                      "Scroll Lock"           ) /**< Keyboard Scroll Lock key. */ \
	X(KEY_PAUSE,                            "key_pause",                            "Pause"                 ) /**< Keyboard Pause key. */ \
	X(MOUSE_BUTTON_LEFT,                    "mouse_button_left",                    "Left Click"            ) /**< Mouse left click. */ \
	X(MOUSE_BUTTON_RIGHT,                   "mouse_button_right",                   "Right Click"           ) /**< Mouse right click. */ \
	X(MOUSE_BUTTON_MIDDLE,                  "mouse_button_middle",                  "Middle Mouse Button"   ) /**< Mouse middle button. */ \
	X(MOUSE_BUTTON_BACK,                    "mouse_button_back",                    "Back Mouse Button"     ) /**< Mouse back button. */ \
	X(MOUSE_BUTTON_FORWARD,                 "mouse_button_forward",                 "Forward Mouse Button"  ) /**< Mouse forward button. */ \
	X(MOUSE_SCROLL_UP,                      "mouse_scroll_up",                      "Scroll Up"             ) /**< Mouse scroll wheel upward movement. */ \
	X(MOUSE_SCROLL_DOWN,                    "mouse_scroll_down",                    "Scroll Down"           ) /**< Mouse scroll wheel downward movement. */ \
	X(MOUSE_SCROLL_LEFT,                    "mouse_scroll_left",                    "Scroll Left"           ) /**< Mouse scroll wheel leftward movement. */ \
	X(MOUSE_SCROLL_RIGHT,                   "mouse_scroll_right",                   "Scroll Right"          ) /**< Mouse scroll wheel rightward movement. */ \
	X(MOUSE_MOTION_UP,                      "mouse_motion_up",                      "Mouse Up"              ) /**< Mouse up movement. */ \
	X(MOUSE_MOTION_DOWN,                    "mouse_motion_down",                    "Mouse Down"            ) /**< Mouse down movement. */ \
	X(MOUSE_MOTION_LEFT,                    "mouse_motion_left",                    "Mouse Left"            ) /**< Mouse left movement. */ \
	X(MOUSE_MOTION_RIGHT,                   "mouse_motion_right",                   "Mouse Right"           ) /**< Mouse right movement. */ \
	X(CONTROLLER_BUTTON_A,                  "controller_button_a",                  "A Button"              ) /**< Controller A button. */ \
	X(CONTROLLER_BUTTON_B,                  "controller_button_b",                  "B Button"              ) /**< Controller B button. */ \
	X(CONTROLLER_BUTTON_X,                  "controller_button_x",                  "X Button"              ) /**< Controller X button. */ \
	X(CONTROLLER_BUTTON_Y,                  "controller_button_y",                  "Y Button"              ) /**< Controller Y button. */ \
	X(CONTROLLER_BUTTON_BACK,               "controller_button_back",               "Back Button"           ) /**< Controller back button. */ \
	X(CONTROLLER_BUTTON_GUIDE,              "controller_button_guide",              "Guide Button"          ) /**< Controller guide button. */ \
	X(CONTROLLER_BUTTON_START,              "controller_button_start",              "Start Button"          ) /**< Controller start button. */ \
	X(CONTROLLER_BUTTON_LEFT_STICK,         "controller_button_left_stick",         "Left Stick Button"     ) /**< Controller left analog stick button. */ \
	X(CONTROLLER_BUTTON_RIGHT_STICK,        "controller_button_right_stick",        "Right Stick Button"    ) /**< Controller right analog stick button. */ \
	X(CONTROLLER_BUTTON_LEFT_SHOULDER,      "controller_button_left_shoulder",      "Left Shoulder Button"  ) /**< Controller left shoulder button. */ \
	X(CONTROLLER_BUTTON_RIGHT_SHOULDER,     "controller_button_right_shoulder",     "Right Shoulder Button" ) /**< Controller right shoulder button. */ \
	X(CONTROLLER_BUTTON_DPAD_UP,            "controller_button_dpad_up",            "D-Pad Up"              ) /**< Controller D-pad up. */ \
	X(CONTROLLER_BUTTON_DPAD_DOWN,          "controller_button_dpad_down",          "D-Pad Down"            ) /**< Controller D-pad down. */ \
	X(CONTROLLER_BUTTON_DPAD_LEFT,          "controller_button_dpad_left",          "D-Pad Left"            ) /**< Controller D-pad left. */ \
	X(CONTROLLER_BUTTON_DPAD_RIGHT,         "controller_button_dpad_right",         "D-Pad Right"           ) /**< Controller D-pad right. */ \
	X(CONTROLLER_BUTTON_MISC1,              "controller_button_misc1",              "Misc1 Button"          ) /**< Controller misc1 button. */ \
	X(CONTROLLER_BUTTON_PADDLE1,            "controller_button_paddle1",            "Paddle 1"              ) /**< Controller paddle 1. */ \
	X(CONTROLLER_BUTTON_PADDLE2,            "controller_button_paddle2",            "Paddle 2"              ) /**< Controller paddle 2. */ \
	X(CONTROLLER_BUTTON_PADDLE3,            "controller_button_paddle3",            "Paddle 3"              ) /**< Controller paddle 3. */ \
	X(CONTROLLER_BUTTON_PADDLE4,            "controller_button_paddle4",            "Paddle 4"              ) /**< Controller paddle 4. */ \
	X(CONTROLLER_BUTTON_TOUCHPAD,           "controller_button_touchpad",           "Touchpad Button"       ) /**< Controller touchpad button. */ \
	X(CONTROLLER_AXIS_LEFT_STICK_UP,        "controller_axis_left_stick_up",        "Left Stick Up"         ) /**< Controller left analog stick upward movement. */ \
	X(CONTROLLER_AXIS_LEFT_STICK_DOWN,      "controller_axis_left_stick_down",      "Left Stick Down"       ) /**< Controller left analog stick downward movement. */ \
	X(CONTROLLER_AXIS_LEFT_STICK_LEFT,      "controller_axis_left_stick_left",      "Left Stick Left"       ) /**< Controller left analog stick leftward movement. */ \
	X(CONTROLLER_AXIS_LEFT_STICK_RIGHT,     "controller_axis_left_stick_right",     "Left Stick Right"      ) /**< Controller left analog stick rightward movement. */ \
	X(CONTROLLER_AXIS_RIGHT_STICK_UP,       "controller_axis_right_stick_up",       "Right Stick Up"        ) /**< Controller right analog stick upward movement. */ \
	X(CONTROLLER_AXIS_RIGHT_STICK_DOWN,     "controller_axis_right_stick_down",     "Right Stick Down"      ) /**< Controller right analog stick downward movement. */ \
	X(CONTROLLER_AXIS_RIGHT_STICK_LEFT,     "controller_axis_right_stick_left",     "Right Stick Left"      ) /**< Controller right analog stick leftward movement. */ \
	X(CONTROLLER_AXIS_RIGHT_STICK_RIGHT,    "controller_axis_right_stick_right",    "Right Stick Right"     ) /**< Controller right analog stick rightward movement. */ \
	X(CONTROLLER_AXIS_LEFT_TRIGGER,         "controller_axis_left_trigger",         "Left Trigger"          ) /**< Controller left trigger movement. */ \
	X(CONTROLLER_AXIS_RIGHT_TRIGGER,        "controller_axis_right_trigger",        "Right Trigger"         ) /**< Controller right trigger movement. */
// clang-format on

/**
 * Unique identifier for a specific control on a physical input device, such as
 * a certain keyboard key, mouse button or joystick axis.
 */
enum class Input : std::uint8_t {
#define X(name, id, str) name,
	DONUT_ENUM_INPUTS(X)
#undef X
};

/**
 * Total number of inputs that exist in the #Input enumeration.
 */
inline constexpr std::size_t INPUT_COUNT /** \cond */ = [] {
	std::size_t result = 0;
#define X(name, id, str) ++result;
	DONUT_ENUM_INPUTS(X)
#undef X
	return result;
}() /** \endcond */;

/**
 * Get the 0-based index of an #Input.
 *
 * \param input valid input value to get the index of.
 *
 * \return an integer between 0 (inclusive) and #INPUT_COUNT
 *         (exclusive) that uniquely identifies the given input.
 */
[[nodiscard]] constexpr std::size_t getInputIndex(Input input) noexcept {
	return static_cast<std::size_t>(static_cast<std::uint8_t>(input));
}

/**
 * Get the identifier string of an #Input.
 *
 * \param input valid input value to get the identifier of.
 *
 * \return a reference to a statically allocated ASCII string that uniquely
 *         identifies the given input, starts with a lowercase letter and only
 *         contains lowercase letters, decimal digits and underscores.
 *
 * \sa findInput()
 */
[[nodiscard]] constexpr std::string_view getInputIdentifier(Input input) noexcept {
	switch (input) {
		/// \cond
#define X(name, id, str) \
	case Input::name: return id;
		DONUT_ENUM_INPUTS(X)
#undef X
		/// \endcond
	}
	return "unknown";
}

/**
 * Get a short human-readable string description of an #Input.
 *
 * \param input valid input value to get the string of.
 *
 * \return a reference to a statically allocated ASCII string that describes the
 *         given input in English.
 */
[[nodiscard]] constexpr std::string_view getInputString(Input input) noexcept {
	switch (input) {
		/// \cond
#define X(name, id, str) \
	case Input::name: return str;
		DONUT_ENUM_INPUTS(X)
#undef X
		/// \endcond
	}
	return "Unknown";
}

/**
 * Find the #Input corresponding to a given identifier.
 *
 * \param identifier the identifier string to search for.
 *
 * \return if found, returns the input value whose identifier matches the given
 *         string. Otherwise, returns an empty optional.
 *
 * \sa getInputIdentifier()
 */
[[nodiscard]] constexpr std::optional<Input> findInput(std::string_view identifier) noexcept {
	/// \cond
#define X(name, id, str) \
	if (identifier == id) { \
		return Input::name; \
	}
	DONUT_ENUM_INPUTS(X)
#undef X
	/// \endcond
	return {};
}

#undef DONUT_ENUM_INPUTS

} // namespace application
} // namespace donut

#endif
