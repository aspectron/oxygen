#include "oxygen.hpp"
#include "library.hpp"
#include "keys.hpp"

namespace aspect { namespace gui {

using namespace v8;

static box<int> get_screen_size()
{
	video_mode const curr_mode = get_current_video_mode();
	return box<int>(curr_mode.width, curr_mode.height);
}

DECLARE_LIBRARY_ENTRYPOINTS(oxygen_install, oxygen_uninstall);

Handle<Value> oxygen_install()
{
	window::init();

	v8pp::module oxygen_module;

	/**
	@module oxygen Oxygen

	Window library for JavaScript.
	**/

	/**
	@function getScreenSize()
	Return current screen dimensions, object with `width` and `height` attributes.
	**/
	oxygen_module.set("getScreenSize", get_screen_size);

	/**
	@class Window Window class

	Window class derives from EventEmitter

	__Input events__

	Input events are generated from keyboard and mouse. They contain following attributes:
	  * `modifiers`    Boolean values set to `true` if the following keys and mouse buttons are pressed down:
	     * `ctrl`
	     * `alt`
	     * `shift`
	     * `lbutton`
	     * `mbutton`
	     * `rbutton`
	     * `xbutton1`
	     * `xbutton2`
	  * `repeats` Number of repeats for the event

	Additionally `key_event` has attributes:
	  * `vk_code`    Virtual key code, see #keys
	  * `key_code`   Platform-specific key code
	  * `char`       Character for `char` event
	  * `key_sym`    Key symbol (in X Window system only)

	And `mouse_event` has attributes:
	  * `button`     Pressed mouse button (0 -none, 1 - left, 2 -middle, 3 - right, 4, 5, ... - X1, X2, ...)
	  * `x`, `y`     Current mouse coordinates
	  * `dx`, `dy`   Scroll delta for mouse wheel events

	@event close()
	@event resize(new_size)
	@param new_size {Object} An object with `width` and `height` attributes
	@event message(msg, wparam, lparam) - Windows only
	@param msg {Number} Windows message code
	@param wparam {Number} Windows message WPARAM
	@param lparam {Number} Windows message LPARAM
	@event drag_accept_files(files) - Windows only
	@param files {Array} file names dragged are being to the window
	@event keydown(key_event)
	@event keyup(key_event)
	@event char(key_event)
	@event mousemove(mouse_event)
	@event mousewheel(mouse_event)
	@event mousedown(mouse_event)
	@event mouseup(mouse_event)
	@event mouseclick(mouse_event)
	**/

	/**
	@function Window(options) Constructor
	@param options {Object}

	Create a new window instance using `options` object.
	Allowed creation options are:

	  * `width` Window client area width, default value is screen width.
	  * `height` Window client area height, default value is screen height.
	  * `left`  Window client area left corner screen position.
	  * `top` Window client area top corner screen position.
	  * `bpp` Window color depth (bits per pixel), default value is current video mode color depth.
	  * `style` Set specific window style. See #styles
	  * `caption` Window caption string.
	  * `icon` Window icon file name (currently implemented in Windows only).
	  * `splash` Splash image file name (currently implemented in Windows only).

	  To set default window dimensions in Windows use zero for `width` and `height`.
	  If `left` and `top` is unspecified, the window is centered on the screen.
	**/
	window::js_class window_class(*v8_core::event_emitter::js_binding);
	window_class
		/**
		@function destroy()
		Close and destroy window instance
		**/
		.set("destroy", &window::destroy)

		/**
		@function on(event, handler)
		@param event {String}
		@param handler {Function}
		Set `handler` function for `event`. See allowed events above.
		**/
		.set("on", &window::on)

		/**
		@function off(event)
		@param event {String}
		Remove handler function for `event`. See allowed events above.
		**/
		.set("off", &window::off)

		/**
		@property width {Number} Client area width
		**/
		.set("width", v8pp::property(&window::width))

		/**
		@property height {Number} Client area height
		**/
		.set("height", v8pp::property(&window::height))

		/**
		@function getRect()
		@return {Rectangle}
		Return window rectangle.
		Rectangle is an object with `left`, `top`, `width`, `height` attributes.
		**/
		.set("getRect", &window::rect)

		/**
		@function setRect(rect)
		@param rect {Rectangle}
		Set window rectangle.
		Rectangle is an object with `left`, `top`, `width`, `height` attributes
		or an array with `[left, top, width, height]` elements.
		**/
		.set("setRect", &window::set_rect)

		/**
		@function setIcon(filename)
		@param filename {String}
		Load and set window icon from a file with specified name.
		Currently implemented in Windows only.
		**/
		.set("setIcon", &window::load_icon_from_file)

		.set("show_frame", &window::show_frame)
		.set("set_topmost", &window::set_topmost)

		.set("use_as_splash_screen", &window::use_as_splash_screen)
		.set("toggle_fullscreen", &window::toggle_fullscreen)
		;
	oxygen_module.set("Window", window_class);

	/**
	@module oxygen
	@property styles Window styles. Contains following constants:
	  * `NONE`         No style.
	  * `TITLEBAR`     Window has a title.
	  * `RESIZE`       Window is allowed to resize.
	  * `CLOSE`        Window is allowed to close.
	  * `FULLSCREEN`   Show window in full screen.
	  * `APPLICATION`  Application main window.
	**/
	v8pp::module styles;
	styles.set_const("NONE", GWS_NONE);
	styles.set_const("TITLEBAR", GWS_TITLEBAR);
	styles.set_const("RESIZE", GWS_RESIZE);
	styles.set_const("CLOSE", GWS_CLOSE);
	styles.set_const("FULLSCREEN", GWS_FULLSCREEN);
	styles.set_const("APPLICATION", GWS_APPWINDOW);
	oxygen_module.set("styles", styles);

	/**
	@module oxygen
	@property keys Key constants. Contains following values:
	  * `0` - `9`
	  * `A` - `Z`
	  * `F1` - `F24`
	  * `BACKSPACE`
	  * `TAB`
	  * `RETURN`
	  * `ESCAPE`
	  * `SPACE`
	  * `INSERT`, `DELETE`
	  * `HOME`, `END`
	  * `PAGE_UP`, `PAGE_DOWN`
	  * `LEFT`, `RIGHT`, `UP` `DOWN`
	  * `PLUS`, `MINUS`
	  * `LEFT_BRACKET`, `RIGHT_BRACKET`
	  * `COMMA`, `PERIOD`, `SLASH`, `BACKSLASH`
	  * `SEMICOLON`, `APOSTROPHE`, `TILDE`
	  * `NUMPAD_0` - `NUMPAD_9`
	  * `NUMPAD_SEPARATOR`, `NUMPAD_DECIMAL`,
	  * `NUMPAD_ADD`, `NUMPAD_SUBTRACT`, `NUMPAD_MULTIPLY`, `NUMPAD_DIVIDE`
	  * `PRINT_SCREEN`, `PAUSE`
	  * `CAPS_LOCK`, `NUM_LOCK`, `SCROLL_LOCK`
	  * `LEFT_SHIFT`, `RIGHT_SHIFT`
	  * `LEFT_CONTROL`, `RIGHT_CONTROL`
	**/
	v8pp::module keys;
#define KEY(name) keys.set_const(#name, KEY_##name)
	KEY(BACKSPACE); KEY(TAB); KEY(RETURN);
	KEY(LEFT_SHIFT); KEY(RIGHT_SHIFT);
	KEY(LEFT_CONTROL); KEY(RIGHT_CONTROL);

	KEY(ESCAPE); KEY(SPACE);
	KEY(PAGE_UP); KEY(PAGE_DOWN);
	KEY(HOME); KEY(END);
	KEY(LEFT); KEY(RIGHT); KEY(UP); KEY(DOWN);
	KEY(INSERT); KEY(DELETE);

	KEY(0); KEY(1); KEY(2); KEY(3); KEY(4);
	KEY(5); KEY(6); KEY(7); KEY(8); KEY(9);

	KEY(A); KEY(B); KEY(C); KEY(D); KEY(E); KEY(F); KEY(G);
	KEY(H); KEY(I); KEY(J); KEY(K); KEY(L); KEY(M); KEY(N);
	KEY(O); KEY(P); KEY(Q); KEY(R); KEY(S); KEY(T);	KEY(U);
	KEY(V); KEY(W); KEY(X); KEY(Y); KEY(Z);

	KEY(PLUS); KEY(MINUS);
	KEY(LEFT_BRACKET); KEY(RIGHT_BRACKET);

	KEY(COMMA); KEY(PERIOD);
	KEY(SLASH); KEY(BACKSLASH);

	KEY(SEMICOLON); KEY(APOSTROPHE); KEY(TILDE);

	KEY(NUMPAD_0); KEY(NUMPAD_1); KEY(NUMPAD_2);
	KEY(NUMPAD_3); KEY(NUMPAD_4); KEY(NUMPAD_5);
	KEY(NUMPAD_6); KEY(NUMPAD_7); KEY(NUMPAD_8);
	KEY(NUMPAD_9);

	KEY(NUMPAD_SEPARATOR); KEY(NUMPAD_DECIMAL);
	KEY(NUMPAD_ADD); KEY(NUMPAD_SUBTRACT);
	KEY(NUMPAD_MULTIPLY); KEY(NUMPAD_DIVIDE);

	KEY(F1); KEY(F2); KEY(F3); KEY(F4); KEY(F5); KEY(F6);
	KEY(F7); KEY(F8); KEY(F9); KEY(F10); KEY(F11); KEY(F12);
	KEY(F13); KEY(F14); KEY(F15); KEY(F16); KEY(F17); KEY(F18);
	KEY(F19); KEY(F20); KEY(F21); KEY(F22); KEY(F23); KEY(F24);

	KEY(PRINT_SCREEN);
	KEY(PAUSE);
	KEY(CAPS_LOCK);
	KEY(NUM_LOCK);
	KEY(SCROLL_LOCK);
#undef KEY
	oxygen_module.set("keys", keys);

	return oxygen_module.new_instance();
}

void oxygen_uninstall(Handle<Value> library)
{
	window::js_class::destroy_objects();
	window::cleanup();
}

}} // ::aspect::gui
