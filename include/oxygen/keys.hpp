#ifndef OXYGEN_KEYS_HPP_INCLUDED
#define OXYGEN_KEYS_HPP_INCLUDED

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <Carbon/Carbon.h>
#else
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#endif

namespace aspect { namespace gui {

// System-dependent virtual key codes
enum key_code
{
#if defined(_WIN32)
	KEY_BACKSPACE = VK_BACK,
	KEY_TAB = VK_TAB,
	KEY_RETURN = VK_RETURN,

	KEY_LEFT_SHIFT = VK_LSHIFT,
	KEY_RIGHT_SHIFT = VK_RSHIFT,
	KEY_LEFT_CONTROL = VK_LCONTROL,
	KEY_RIGHT_CONTROL = VK_RCONTROL,

	KEY_ESCAPE = VK_ESCAPE,
	KEY_SPACE = VK_SPACE,
	KEY_PAGE_UP = VK_PRIOR,
	KEY_PAGE_DOWN = VK_NEXT,
	KEY_END = VK_END,
	KEY_HOME = VK_HOME,
	KEY_LEFT = VK_LEFT,
	KEY_UP = VK_UP,
	KEY_RIGHT = VK_RIGHT,
	KEY_DOWN = VK_DOWN,
	KEY_INSERT = VK_INSERT,
	KEY_DELETE = VK_DELETE,

	KEY_0 = 0x30, KEY_1 = 0x31, KEY_2 = 0x32, KEY_3 = 0x33, KEY_4 = 0x34,
	KEY_5 = 0x35, KEY_6 = 0x36, KEY_7 = 0x37, KEY_8 = 0x38, KEY_9 = 0x39,

	KEY_A = 0x41, KEY_B = 0x42, KEY_C = 0x43, KEY_D = 0x44, KEY_E = 0x45,
	KEY_F = 0x46, KEY_G = 0x47,	KEY_H = 0x48, KEY_I = 0x49, KEY_J = 0x4a,
	KEY_K = 0x4b, KEY_L = 0x4c, KEY_M = 0x4d, KEY_N = 0x4e, KEY_O = 0x4f,
	KEY_P = 0x50, KEY_Q = 0x51, KEY_R = 0x52, KEY_S = 0x53, KEY_T = 0x54,
	KEY_U = 0x55, KEY_V = 0x56, KEY_W = 0x57, KEY_X = 0x58, KEY_Y = 0x59,
	KEY_Z = 0x5a,

	KEY_EQUAL = VK_OEM_PLUS,
	KEY_PLUS = VK_OEM_PLUS,       // +
	KEY_MINUS = VK_OEM_MINUS,     // -

	KEY_LEFT_BRACKET = VK_OEM_4,  // [
	KEY_RIGHT_BRACKET = VK_OEM_6, // ]

	KEY_COMMA = VK_OEM_COMMA,     // ,
	KEY_PERIOD = VK_OEM_PERIOD,   // .

	KEY_SLASH = VK_OEM_2,         // /
	KEY_BACKSLASH = VK_OEM_5,     // \|

	KEY_SEMICOLON = VK_OEM_1,     // ;
	KEY_APOSTROPHE = VK_OEM_7,    // '
	KEY_TILDE = VK_OEM_3,         // ~

	KEY_NUMPAD_0 = VK_NUMPAD0, KEY_NUMPAD_1 = VK_NUMPAD1,
	KEY_NUMPAD_2 = VK_NUMPAD2, KEY_NUMPAD_3 = VK_NUMPAD3,
	KEY_NUMPAD_4 = VK_NUMPAD4, KEY_NUMPAD_5 = VK_NUMPAD5,
	KEY_NUMPAD_6 = VK_NUMPAD6, KEY_NUMPAD_7 = VK_NUMPAD7,
	KEY_NUMPAD_8 = VK_NUMPAD8, KEY_NUMPAD_9 = VK_NUMPAD9,

	KEY_NUMPAD_SEPARATOR = VK_SEPARATOR,
	KEY_NUMPAD_DECIMAL = VK_DECIMAL,
	KEY_NUMPAD_ADD = VK_ADD,
	KEY_NUMPAD_SUBTRACT = VK_SUBTRACT,
	KEY_NUMPAD_MULTIPLY = VK_MULTIPLY,
	KEY_NUMPAD_DIVIDE = VK_DIVIDE,

	KEY_F1 = VK_F1, KEY_F2 = VK_F2, KEY_F3 = VK_F3, KEY_F4 = VK_F4,
	KEY_F5 = VK_F5, KEY_F6 = VK_F6, KEY_F7 = VK_F7, KEY_F8 = VK_F8,
	KEY_F9 = VK_F9,	KEY_F10 = VK_F10, KEY_F11 = VK_F11, KEY_F12 = VK_F12,
	KEY_F13 = VK_F13, KEY_F14 = VK_F14, KEY_F15 = VK_F15, KEY_F16 = VK_F16,
	KEY_F17 = VK_F17, KEY_F18 = VK_F18, KEY_F19 = VK_F19, KEY_F20 = VK_F20,
	KEY_F21 = VK_F21, KEY_F22 = VK_F22, KEY_F23 = VK_F23, KEY_F24 = VK_F24,

	KEY_PRINT_SCREEN = VK_SNAPSHOT,
	KEY_PAUSE = VK_PAUSE,
	KEY_CAPS_LOCK = VK_CAPITAL,
	KEY_NUM_LOCK = VK_NUMLOCK,
	KEY_SCROLL_LOCK = VK_SCROLL,
#elif defined(__APPLE__)
	KEY_BACKSPACE = kVK_Delete,
	KEY_TAB = kVK_Tab,
	KEY_RETURN = kVK_Return,
	KEY_LEFT_SHIFT = kVK_Shift,
	KEY_RIGHT_SHIFT = kVK_RightShift,
	KEY_LEFT_CONTROL = kVK_Control,
	KEY_RIGHT_CONTROL = kVK_RightControl,

	KEY_ESCAPE = kVK_Escape,
	KEY_SPACE = kVK_Space,
	KEY_PAGE_UP = kVK_PageUp,
	KEY_PAGE_DOWN = kVK_PageDown,
	KEY_END = kVK_End,
	KEY_HOME = kVK_Home,
	KEY_LEFT = kVK_LeftArrow,
	KEY_UP = kVK_UpArrow,
	KEY_RIGHT = kVK_RightArrow,
	KEY_DOWN = kVK_DownArrow,
	KEY_INSERT = -1, //kVK_Insert,
	KEY_DELETE = -1, //kVK_ForwardDelete,

	KEY_0 = kVK_ANSI_0, KEY_1 = kVK_ANSI_1, KEY_2 = kVK_ANSI_2,
	KEY_3 = kVK_ANSI_3, KEY_4 = kVK_ANSI_4, KEY_5 = kVK_ANSI_5,
	KEY_6 = kVK_ANSI_6, KEY_7 = kVK_ANSI_7, KEY_8 = kVK_ANSI_8,
	KEY_9 = kVK_ANSI_9,

	KEY_A = kVK_ANSI_A, KEY_B = kVK_ANSI_B, KEY_C = kVK_ANSI_C,
	KEY_D = kVK_ANSI_D, KEY_E = kVK_ANSI_E,	KEY_F = kVK_ANSI_F,
	KEY_G = kVK_ANSI_G,	KEY_H = kVK_ANSI_H, KEY_I = kVK_ANSI_I,
	KEY_J = kVK_ANSI_J,	KEY_K = kVK_ANSI_K, KEY_L = kVK_ANSI_L,
	KEY_M = kVK_ANSI_M, KEY_N = kVK_ANSI_N, KEY_O = kVK_ANSI_O,
	KEY_P = kVK_ANSI_P, KEY_Q = kVK_ANSI_Q, KEY_R = kVK_ANSI_R,
	KEY_S = kVK_ANSI_S, KEY_T = kVK_ANSI_T,	KEY_U = kVK_ANSI_U,
	KEY_V = kVK_ANSI_V, KEY_W = kVK_ANSI_W, KEY_X = kVK_ANSI_X,
	KEY_Y = kVK_ANSI_Y, KEY_Z = kVK_ANSI_Z,

	KEY_EQUAL = kVK_ANSI_Equal,                 // =
	KEY_PLUS = kVK_ANSI_Equal,                  // +
	KEY_MINUS = kVK_ANSI_Minus,                 // -

	KEY_LEFT_BRACKET = kVK_ANSI_LeftBracket,    // [
	KEY_RIGHT_BRACKET = kVK_ANSI_RightBracket,  // ]

	KEY_COMMA = kVK_ANSI_Comma,                 // ,
	KEY_PERIOD = kVK_ANSI_Period,               // .

	KEY_SLASH = kVK_ANSI_Slash,                 // /
	KEY_BACKSLASH = kVK_ANSI_Backslash,         // \|

	KEY_SEMICOLON = kVK_ANSI_Semicolon,         // ;
	KEY_APOSTROPHE = kVK_ANSI_Quote,            // '
	KEY_TILDE = kVK_ANSI_Grave,                 // ~

	KEY_NUMPAD_0 = kVK_ANSI_Keypad0,
	KEY_NUMPAD_1 = kVK_ANSI_Keypad1,
	KEY_NUMPAD_2 = kVK_ANSI_Keypad2,
	KEY_NUMPAD_3 = kVK_ANSI_Keypad3,
	KEY_NUMPAD_4 = kVK_ANSI_Keypad4,
	KEY_NUMPAD_5 = kVK_ANSI_Keypad5,
	KEY_NUMPAD_6 = kVK_ANSI_Keypad6,
	KEY_NUMPAD_7 = kVK_ANSI_Keypad7,
	KEY_NUMPAD_8 = kVK_ANSI_Keypad8,
	KEY_NUMPAD_9 = kVK_ANSI_Keypad9,

	KEY_NUMPAD_SEPARATOR = -1,
	KEY_NUMPAD_DECIMAL = kVK_ANSI_KeypadDecimal,
	KEY_NUMPAD_ADD = kVK_ANSI_KeypadPlus,
	KEY_NUMPAD_SUBTRACT = kVK_ANSI_KeypadMinus,
	KEY_NUMPAD_MULTIPLY = kVK_ANSI_KeypadMultiply,
	KEY_NUMPAD_DIVIDE = kVK_ANSI_KeypadDivide,

	KEY_F1 = kVK_F1, KEY_F2 = kVK_F2, KEY_F3 = kVK_F3, KEY_F4 = kVK_F4,
	KEY_F5 = kVK_F5, KEY_F6 = kVK_F6, KEY_F7 = kVK_F7, KEY_F8 = kVK_F8,
	KEY_F9 = kVK_F9, KEY_F10 = kVK_F10, KEY_F11 = kVK_F11, KEY_F12 = kVK_F12,
	KEY_F13 = kVK_F13, KEY_F14 = kVK_F14, KEY_F15 = kVK_F15, KEY_F16 = kVK_F16,
	KEY_F17 = kVK_F17, KEY_F18 = kVK_F18, KEY_F19 = kVK_F19, KEY_F20 = kVK_F20,
	KEY_F21 = -1, KEY_F22 = -1, KEY_F23 = -1, KEY_F24 = -1,

	KEY_PRINT_SCREEN = -1,
	KEY_PAUSE = -1,
	KEY_CAPS_LOCK = kVK_CapsLock,
	KEY_NUM_LOCK = -1,
	KEY_SCROLL_LOCK = -1,
#else
	KEY_BACKSPACE = XK_BackSpace,
	KEY_TAB = XK_Tab,
	KEY_RETURN = XK_Return,
	KEY_LEFT_SHIFT = XK_Shift_L,
	KEY_RIGHT_SHIFT = XK_Shift_R,
	KEY_LEFT_CONTROL = XK_Control_L,
	KEY_RIGHT_CONTROL = XK_Control_R,

	KEY_ESCAPE = XK_Escape,
	KEY_SPACE = XK_space,
	KEY_PAGE_UP = XK_Prior,
	KEY_PAGE_DOWN = XK_Next,
	KEY_END = XK_End,
	KEY_HOME = XK_Home,
	KEY_LEFT = XK_Left,
	KEY_UP = XK_Up,
	KEY_RIGHT = XK_Right,
	KEY_DOWN = XK_Down,
	KEY_INSERT = XK_Insert,
	KEY_DELETE = XK_Delete,

	KEY_0 = XK_0, KEY_1 = XK_1, KEY_2 = XK_2, KEY_3 = XK_3, KEY_4 = XK_4,
	KEY_5 = XK_5, KEY_6 = XK_6, KEY_7 = XK_7, KEY_8 = XK_8, KEY_9 = XK_9,

	KEY_A = XK_a, KEY_B = XK_b, KEY_C = XK_c, KEY_D = XK_d, KEY_E = XK_e,
	KEY_F = XK_f, KEY_G = XK_g,	KEY_H = XK_h, KEY_I = XK_i, KEY_J = XK_j,
	KEY_K = XK_k, KEY_L = XK_l, KEY_M = XK_m, KEY_N = XK_n, KEY_O = XK_o,
	KEY_P = XK_p, KEY_Q = XK_q, KEY_R = XK_r, KEY_S = XK_s, KEY_T = XK_t,
	KEY_U = XK_u, KEY_V = XK_v, KEY_W = XK_w, KEY_X = XK_x, KEY_Y = XK_y,
	KEY_Z = XK_z,

	KEY_EQUAL = XK_equal,                 // =
	KEY_PLUS = XK_equal,                  // +
	KEY_MINUS = XK_minus,                 // -

	KEY_LEFT_BRACKET = XK_bracketleft,    // [
	KEY_RIGHT_BRACKET = XK_bracketright,  // ]

	KEY_COMMA = XK_comma,                 // ,
	KEY_PERIOD = XK_period,               // .

	KEY_SLASH = XK_slash,                 // /
	KEY_BACKSLASH = XK_backslash,         // \|

	KEY_SEMICOLON = XK_semicolon,         // ;
	KEY_APOSTROPHE = XK_apostrophe,       // '
	KEY_TILDE = XK_grave,                 // ~

	KEY_NUMPAD_0 = XK_KP_Insert,
	KEY_NUMPAD_1 = XK_KP_End,
	KEY_NUMPAD_2 = XK_KP_Down,
	KEY_NUMPAD_3 = XK_KP_Next,
	KEY_NUMPAD_4 = XK_KP_Left,
	KEY_NUMPAD_5 = XK_KP_Begin,
	KEY_NUMPAD_6 = XK_KP_Right,
	KEY_NUMPAD_7 = XK_KP_Home,
	KEY_NUMPAD_8 = XK_KP_Up,
	KEY_NUMPAD_9 = XK_KP_Prior,

	KEY_NUMPAD_SEPARATOR = XK_KP_Separator,
	KEY_NUMPAD_DECIMAL = XK_KP_Decimal,
	KEY_NUMPAD_ADD = XK_KP_Add,
	KEY_NUMPAD_SUBTRACT = XK_KP_Subtract,
	KEY_NUMPAD_MULTIPLY = XK_KP_Multiply,
	KEY_NUMPAD_DIVIDE = XK_KP_Divide,

	KEY_F1 = XK_F1, KEY_F2 = XK_F2, KEY_F3 = XK_F3, KEY_F4 = XK_F4,
	KEY_F5 = XK_F5, KEY_F6 = XK_F6, KEY_F7 = XK_F7, KEY_F8 = XK_F8,
	KEY_F9 = XK_F9,	KEY_F10 = XK_F10, KEY_F11 = XK_F11, KEY_F12 = XK_F12,
	KEY_F13 = XK_F13, KEY_F14 = XK_F14, KEY_F15 = XK_F15, KEY_F16 = XK_F16,
	KEY_F17 = XK_F17, KEY_F18 = XK_F18, KEY_F19 = XK_F19, KEY_F20 = XK_F20,
	KEY_F21 = XK_F21, KEY_F22 = XK_F22, KEY_F23 = XK_F23, KEY_F24 = XK_F24,

	KEY_PRINT_SCREEN = XK_Print,
	KEY_PAUSE = XK_Pause,
	KEY_CAPS_LOCK = XK_Caps_Lock,
	KEY_NUM_LOCK = XK_Num_Lock,
	KEY_SCROLL_LOCK = XK_Scroll_Lock,
#endif
};

}} // aspect::gui

#endif // OXYGEN_KEYS_HPP_INCLUDED

