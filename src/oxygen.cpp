#include "oxygen/oxygen.hpp"
#include "oxygen/keys.hpp"

#include <node.h>

#include <v8pp/module.hpp>
#include <v8pp/class.hpp>
#include <v8pp/convert.hpp>

namespace aspect { namespace gui {

static void display_enumerate_v8(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();
	v8::EscapableHandleScope scope(isolate);

	std::vector<display> result = display::enumerate();
	v8::Local<v8::Array> arr = v8::Array::New(isolate, static_cast<int>(result.size()));
	for (uint32_t i = 0; i < result.size(); ++i)
	{
		arr->Set(i, v8pp::class_<display>::import_external(isolate, new display(result[i])));
	}
	args.GetReturnValue().Set(scope.Escape(arr));
}

static void display_primary_v8(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	args.GetReturnValue().Set(v8pp::class_<display>::import_external(isolate, new display(display::primary())));
}

static void display_from_window_v8(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	window const* wnd = v8pp::from_v8<window*>(isolate, args[0]);
	if (args.Length() > 0 && !wnd)
	{
		throw std::invalid_argument("required window argument");
	}
	args.GetReturnValue().Set(v8pp::class_<display>::import_external(isolate, new display(display::from_window(wnd))));
}

static void init(v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module)
{
	window::init();

	v8::Isolate* isolate = v8::Isolate::GetCurrent();

	/**
	@module oxygen Oxygen

	Window library for JavaScript.
	**/
	v8pp::module oxygen_module(isolate);

	/**
	@class Display
	Display information. Contains information about display monitor, such as
	color depth, dimensions in pixels, current display mode.
	
	Display `Mode` is an object with following attributes:
	  * `width`       - display width in pixels
	  * `height`      - display height in pixels
	  * `colorDepth`  - display color depth, bits per pixel
	  * `frequency`   - display refresh rate, Hz
	**/
	v8pp::class_<display> display_class(isolate);
	display_class
		/**
		@function enumerate()
		@return {Array}
		Enumerate display monitors. Return array of Display objects.
		**/
		.set("enumerate", display_enumerate_v8)

		/**
		@function primary()
		@return {Display}
		Get primary display instance.
		**/
		.set("primary", display_primary_v8)

		/**
		@function fromWindow(Window window)
		@param window {Window}
		@return {Display}
		Get display instance for a specified window.
		**/
		.set("fromWindow", display_from_window_v8)

		/**
		@property name {String} Display name
		**/
		.set("name", &display::name)
		/**
		@property colorDepth {Number} Display color depth, bits per pixel
		**/
		.set("colorDepth", &display::color_depth)
		/**
		@property rectangle {Rectangle} Display rectangle object `{ left, top, width, height }`
		**/
		.set("rectangle", &display::rect)
		/**
		@property workRectangle {Rectangle} Display work area rectangle object `{ left, top, width, height }`
		**/
		.set("workRectangle", &display::work_rect)

		/**
		@function modes()
		@return {Array}
		Supported display modes. Return array of `Mode` objects
		**/
		.set("modes", &display::modes)

		/**
		@function currentMode()
		@return {Mode}
		Current display mode.
		**/
		.set("currentMode", &display::current_mode)
		;
	oxygen_module.set("Display", display_class);

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
	v8pp::class_<window> window_class(isolate);
	window_class
		.ctor<v8::FunctionCallbackInfo<v8::Value> const&>()
		.inherit<event_emitter>()
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

		/**
		@function setCursor(cursor)
		@param cursor - see #cursors
		Set window cursor
		**/
		.set("setCursor", &window::set_stock_cursor)

		/**
		@function runFileDialog(options)
		@return {Undefined|String|Array}
		Run modal dialog for file name selection.

		If the dialog has been canceled, `runFileDialog()` function returns `undefined` value.
		When `multiselect = true` the function returns an array of file names, otherwise it returns a selected file name string.

		Allowed options are:
		  * `type` Dialog type string, open or save, default is open.
		  * `multiselect` Enable multiple file names selection, valid only for type = 'open' dialogs, default is false.
		  * `title` Custom dialog title string
		  * `filter` Key-value mapping with filter to display file names. Each key is a file name filter, value is the filter description.
		  * `defaultDir` Initial directory name where to display files in the dialog
		  * `defaultName` Default file name, used in the dialog
		  * `defaultExt` Default file extension, used in the dialog

		Examples:
		```
		// Dialog to open multiple files
		var filenames_to_open = window.runFileDialog({
			title: 'My title',
			multiselect: true,
			filter: {
				'*.pdf': 'PDF documents',
				'*.html': 'HTML documents',
				'*.rtf': 'RTF documents',
				'*.doc;*.docx': 'Microsoft Word documents',
				'*.pdf;*.html;*.rtf;*doc;*.docx': 'All supported documents',
				'*.*': 'All files',
			},
		});

		// Dialog to save a file, default name is `filename.ext`
		var filename_to_save = window.runFileDialog({
			type: 'save',
			defaultName: 'filename',
			defaultExt: 'ext',
		});
		```
		**/
#if defined(_WIN32) || defined(__APPLE__)
		.set("runFileDialog", &window::run_file_dialog)
#endif
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
	v8pp::module styles(isolate);
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
	  * `EQUAL`, `PLUS`, `MINUS`
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
	v8pp::module keys(isolate);
#define KEY(name) if (KEY_##name >= 0) keys.set_const(#name, KEY_##name)
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

	KEY(EQUAL); KEY(PLUS); KEY(MINUS);
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

	/**
	@module oxygen
	@property cursors Window cursors. Contains following constants:
	  * `ARROW`       Default cursor pointer
	  * `INPUT`       Cursor in input field
	  * `HAND`        Hand
	  * `CROSS`       Crosshair
	  * `MOVE`        Arrows to 4  directions
	  * `WAIT`        Hourglass
	**/
	v8pp::module cursors(isolate);
#define CURSOR(name) cursors.set_const(#name, window::name)
	CURSOR(ARROW);
	CURSOR(INPUT);
	CURSOR(HAND);
	CURSOR(CROSS);
	CURSOR(MOVE);
	CURSOR(WAIT);
#undef CURSOR
	oxygen_module.set("cursors", cursors);

	node::AtExit([](void*)
	{
		v8pp::class_<window>::destroy_objects(v8::Isolate::GetCurrent());
		window::cleanup();
	}, nullptr);

	exports->SetPrototype(oxygen_module.new_instance());
}

NODE_MODULE(oxygen, init)

}} // ::aspect::gui
