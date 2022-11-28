#ifndef DONUT_APPLICATION_INPUT_HPP
#define DONUT_APPLICATION_INPUT_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace donut {
namespace application {

// X(name, id, str)
#define DONUT_ENUM_INPUTS(X) \
	X(KEY_A, "key_a", "A") \
	X(KEY_B, "key_b", "B") \
	X(KEY_C, "key_c", "C") \
	X(KEY_D, "key_d", "D") \
	X(KEY_E, "key_e", "E") \
	X(KEY_F, "key_f", "F") \
	X(KEY_G, "key_g", "G") \
	X(KEY_H, "key_h", "H") \
	X(KEY_I, "key_i", "I") \
	X(KEY_J, "key_j", "J") \
	X(KEY_K, "key_k", "K") \
	X(KEY_L, "key_l", "L") \
	X(KEY_M, "key_m", "M") \
	X(KEY_N, "key_n", "N") \
	X(KEY_O, "key_o", "O") \
	X(KEY_P, "key_p", "P") \
	X(KEY_Q, "key_q", "Q") \
	X(KEY_R, "key_r", "R") \
	X(KEY_S, "key_s", "S") \
	X(KEY_T, "key_t", "T") \
	X(KEY_U, "key_u", "U") \
	X(KEY_V, "key_v", "V") \
	X(KEY_W, "key_w", "W") \
	X(KEY_X, "key_x", "X") \
	X(KEY_Y, "key_y", "Y") \
	X(KEY_Z, "key_z", "Z") \
	X(KEY_1, "key_1", "1") \
	X(KEY_2, "key_2", "2") \
	X(KEY_3, "key_3", "3") \
	X(KEY_4, "key_4", "4") \
	X(KEY_5, "key_5", "5") \
	X(KEY_6, "key_6", "6") \
	X(KEY_7, "key_7", "7") \
	X(KEY_8, "key_8", "8") \
	X(KEY_9, "key_9", "9") \
	X(KEY_0, "key_0", "0") \
	X(KEY_ESCAPE, "key_escape", "Esc") \
	X(KEY_LCTRL, "key_lctrl", "LCtrl") \
	X(KEY_RCTRL, "key_rctrl", "RCtrl") \
	X(KEY_LSHIFT, "key_lshift", "LShift") \
	X(KEY_RSHIFT, "key_rshift", "RShift") \
	X(KEY_LALT, "key_lalt", "LAlt") \
	X(KEY_RALT, "key_ralt", "RAlt") \
	X(KEY_MENU, "key_menu", "Menu") \
	X(KEY_LEFT_BRACKET, "key_left_bracket", "[") \
	X(KEY_RIGHT_BRACKET, "key_right_bracket", "]") \
	X(KEY_SEMICOLON, "key_semicolon", ";") \
	X(KEY_COMMA, "key_comma", ",") \
	X(KEY_PERIOD, "key_period", ".") \
	X(KEY_APOSTROPHE, "key_apostrophe", "'") \
	X(KEY_SLASH, "key_slash", "/") \
	X(KEY_BACKSLASH, "key_backslash", "\\") \
	X(KEY_GRAVE, "key_grave", "`") \
	X(KEY_EQUALS, "key_equals", "=") \
	X(KEY_MINUS, "key_minus", "-") \
	X(KEY_SPACE, "key_space", "Space") \
	X(KEY_RETURN, "key_return", "Return") \
	X(KEY_BACKSPACE, "key_backspace", "Backspace") \
	X(KEY_TAB, "key_tab", "Tab") \
	X(KEY_PAGE_UP, "key_page_up", "Pgup") \
	X(KEY_PAGE_DOWN, "key_page_down", "Pgdn") \
	X(KEY_END, "key_end", "End") \
	X(KEY_HOME, "key_home", "Home") \
	X(KEY_INSERT, "key_insert", "Insert") \
	X(KEY_DELETE, "key_delete", "Delete") \
	X(KEY_ARROW_UP, "key_arrow_up", "Up Arrow") \
	X(KEY_ARROW_DOWN, "key_arrow_down", "Down Arrow") \
	X(KEY_ARROW_LEFT, "key_arrow_left", "Left Arrow") \
	X(KEY_ARROW_RIGHT, "key_arrow_right", "Right Arrow") \
	X(KEY_NUMPAD_PLUS, "key_numpad_plus", "Numpad +") \
	X(KEY_NUMPAD_MINUS, "key_numpad_minus", "Numpad -") \
	X(KEY_NUMPAD_MULTIPLY, "key_numpad_multiply", "Numpad *") \
	X(KEY_NUMPAD_DIVIDE, "key_numpad_divide", "Numpad /") \
	X(KEY_NUMPAD_1, "key_numpad_1", "Numpad 1") \
	X(KEY_NUMPAD_2, "key_numpad_2", "Numpad 2") \
	X(KEY_NUMPAD_3, "key_numpad_3", "Numpad 3") \
	X(KEY_NUMPAD_4, "key_numpad_4", "Numpad 4") \
	X(KEY_NUMPAD_5, "key_numpad_5", "Numpad 5") \
	X(KEY_NUMPAD_6, "key_numpad_6", "Numpad 6") \
	X(KEY_NUMPAD_7, "key_numpad_7", "Numpad 7") \
	X(KEY_NUMPAD_8, "key_numpad_8", "Numpad 8") \
	X(KEY_NUMPAD_9, "key_numpad_9", "Numpad 9") \
	X(KEY_NUMPAD_0, "key_numpad_0", "Numpad 0") \
	X(KEY_F1, "key_f1", "F1") \
	X(KEY_F2, "key_f2", "F2") \
	X(KEY_F3, "key_f3", "F3") \
	X(KEY_F4, "key_f4", "F4") \
	X(KEY_F5, "key_f5", "F5") \
	X(KEY_F6, "key_f6", "F6") \
	X(KEY_F7, "key_f7", "F7") \
	X(KEY_F8, "key_f8", "F8") \
	X(KEY_F9, "key_f9", "F9") \
	X(KEY_F10, "key_f10", "F10") \
	X(KEY_F11, "key_f11", "F11") \
	X(KEY_F12, "key_f12", "F12") \
	X(KEY_F13, "key_f13", "F13") \
	X(KEY_F14, "key_f14", "F14") \
	X(KEY_F15, "key_f15", "F15") \
	X(KEY_PRINT_SCREEN, "key_print_screen", "PrtSc") \
	X(KEY_SCROLL_LOCK, "key_scroll_lock", "Scroll Lock") \
	X(KEY_PAUSE, "key_pause", "Pause") \
	X(MOUSE_BUTTON_LEFT, "mouse_button_left", "Left Click") \
	X(MOUSE_BUTTON_RIGHT, "mouse_button_right", "Right Click") \
	X(MOUSE_BUTTON_MIDDLE, "mouse_button_middle", "Middle Mouse Button") \
	X(MOUSE_BUTTON_BACK, "mouse_button_back", "Back Mouse Button") \
	X(MOUSE_BUTTON_FORWARD, "mouse_button_forward", "Forward Mouse Button") \
	X(MOUSE_SCROLL_UP, "mouse_scroll_up", "Scroll Up") \
	X(MOUSE_SCROLL_DOWN, "mouse_scroll_down", "Scroll Down") \
	X(MOUSE_SCROLL_LEFT, "mouse_scroll_left", "Scroll Left") \
	X(MOUSE_SCROLL_RIGHT, "mouse_scroll_right", "Scroll Right") \
	X(MOUSE_MOTION_UP, "mouse_motion_up", "Mouse Up") \
	X(MOUSE_MOTION_DOWN, "mouse_motion_down", "Mouse Down") \
	X(MOUSE_MOTION_LEFT, "mouse_motion_left", "Mouse Left") \
	X(MOUSE_MOTION_RIGHT, "mouse_motion_right", "Mouse Right") \
	X(CONTROLLER_BUTTON_A, "controller_button_a", "A Button") \
	X(CONTROLLER_BUTTON_B, "controller_button_b", "B Button") \
	X(CONTROLLER_BUTTON_X, "controller_button_x", "X Button") \
	X(CONTROLLER_BUTTON_Y, "controller_button_y", "Y Button") \
	X(CONTROLLER_BUTTON_BACK, "controller_button_back", "Back Button") \
	X(CONTROLLER_BUTTON_GUIDE, "controller_button_guide", "Guide Button") \
	X(CONTROLLER_BUTTON_START, "controller_button_start", "Start Button") \
	X(CONTROLLER_BUTTON_LEFT_STICK, "controller_button_left_stick", "Left Stick") \
	X(CONTROLLER_BUTTON_RIGHT_STICK, "controller_button_right_stick", "Right Stick") \
	X(CONTROLLER_BUTTON_LEFT_SHOULDER, "controller_button_left_shoulder", "Left Shoulder") \
	X(CONTROLLER_BUTTON_RIGHT_SHOULDER, "controller_button_right_shoulder", "Right Shoulder") \
	X(CONTROLLER_BUTTON_DPAD_UP, "controller_button_dpad_up", "DPad Up") \
	X(CONTROLLER_BUTTON_DPAD_DOWN, "controller_button_dpad_down", "DPad Down") \
	X(CONTROLLER_BUTTON_DPAD_LEFT, "controller_button_dpad_left", "DPad Left") \
	X(CONTROLLER_BUTTON_DPAD_RIGHT, "controller_button_dpad_right", "DPad Right") \
	X(CONTROLLER_BUTTON_MISC1, "controller_button_misc1", "Misc1 Button") \
	X(CONTROLLER_BUTTON_PADDLE1, "controller_button_paddle1", "Paddle 1") \
	X(CONTROLLER_BUTTON_PADDLE2, "controller_button_paddle2", "Paddle 2") \
	X(CONTROLLER_BUTTON_PADDLE3, "controller_button_paddle3", "Paddle 3") \
	X(CONTROLLER_BUTTON_PADDLE4, "controller_button_paddle4", "Paddle 4") \
	X(CONTROLLER_BUTTON_TOUCHPAD, "controller_button_touchpad", "Touchpad Button") \
	X(CONTROLLER_AXIS_LEFT_STICK_UP, "controller_axis_left_stick_up", "Left Stick Up") \
	X(CONTROLLER_AXIS_LEFT_STICK_DOWN, "controller_axis_left_stick_down", "Left Stick Down") \
	X(CONTROLLER_AXIS_LEFT_STICK_LEFT, "controller_axis_left_stick_left", "Left Stick Left") \
	X(CONTROLLER_AXIS_LEFT_STICK_RIGHT, "controller_axis_left_stick_right", "Left Stick Right") \
	X(CONTROLLER_AXIS_RIGHT_STICK_UP, "controller_axis_right_stick_up", "Right Stick Up") \
	X(CONTROLLER_AXIS_RIGHT_STICK_DOWN, "controller_axis_right_stick_down", "Right Stick Down") \
	X(CONTROLLER_AXIS_RIGHT_STICK_LEFT, "controller_axis_right_stick_left", "Right Stick Left") \
	X(CONTROLLER_AXIS_RIGHT_STICK_RIGHT, "controller_axis_right_stick_right", "Right Stick Right") \
	X(CONTROLLER_AXIS_LEFT_TRIGGER, "controller_axis_left_trigger", "Left Trigger") \
	X(CONTROLLER_AXIS_RIGHT_TRIGGER, "controller_axis_right_trigger", "Right Trigger")

enum class Input : std::uint8_t {
#define X(name, id, str) name,
	DONUT_ENUM_INPUTS(X)
#undef X
};

static constexpr std::size_t INPUT_COUNT = [] {
	std::size_t result = 0;
#define X(name, id, str) ++result;
	DONUT_ENUM_INPUTS(X)
#undef X
	return result;
}();

[[nodiscard]] constexpr std::size_t getInputIndex(Input input) noexcept {
	return static_cast<std::size_t>(static_cast<std::uint8_t>(input));
}

[[nodiscard]] constexpr std::string_view getInputIdentifier(Input input) noexcept {
	switch (input) {
#define X(name, id, str) \
	case Input::name: return id;
		DONUT_ENUM_INPUTS(X)
#undef X
	}
	return "unknown";
}

[[nodiscard]] constexpr std::string_view getInputString(Input input) noexcept {
	switch (input) {
#define X(name, id, str) \
	case Input::name: return str;
		DONUT_ENUM_INPUTS(X)
#undef X
	}
	return "Unknown";
}

[[nodiscard]] constexpr std::optional<Input> findInput(std::string_view identifier) noexcept {
#define X(name, id, str) \
	if (identifier == id) { \
		return Input::name; \
	}
	DONUT_ENUM_INPUTS(X)
#undef X
	return std::nullopt;
}

#undef DONUT_ENUM_INPUTS

} // namespace application
} // namespace donut

#endif
