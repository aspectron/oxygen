#include "oxygen.hpp"
#include "library.hpp"
#include "keys.hpp"

namespace aspect { namespace gui {

using namespace v8;

Handle<Value> get_screen_size(Arguments const&)
{
	HandleScope scope;

	video_mode const curr_mode = get_current_video_mode();

	Handle<Object> result = Object::New();
	set_option(result, "left", 0);
	set_option(result, "top", 0);
	set_option(result, "width", curr_mode.width);
	set_option(result, "height", curr_mode.height);

	return scope.Close(result);
}

DECLARE_LIBRARY_ENTRYPOINTS(oxygen_install, oxygen_uninstall);

Handle<Value> oxygen_install()
{
	window::init();

	v8pp::module oxygen_module;

	window::js_class window_class(*v8_core::event_emitter::js_binding);
	window_class
		.set("destroy", &window::destroy)
		.set("on", &window::on)
		.set("off", &window::off)
		.set("width", v8pp::property(&window::width))
		.set("height", v8pp::property(&window::height))
		.set("get_client_rect", &window::get_client_rect)
		.set("get_window_rect", &window::get_window_rect)
		.set("set_window_rect", &window::set_window_rect)
		.set("show_frame", &window::show_frame)
		.set("set_topmost", &window::set_topmost)
		.set("use_as_splash_screen", &window::use_as_splash_screen)
		.set("load_icon_from_file", &window::load_icon_from_file)
		;
	oxygen_module.set("window", window_class);

	oxygen_module.set("get_screen_size", get_screen_size);

	v8pp::module styles;
	styles.set_const("NONE", GWS_NONE);
	styles.set_const("TITLEBAR", GWS_TITLEBAR);
	styles.set_const("RESIZE", GWS_RESIZE);
	styles.set_const("CLOSE", GWS_CLOSE);
	styles.set_const("FULLSCREEN", GWS_FULLSCREEN);
	styles.set_const("APPLICATION", GWS_APPWINDOW);
	oxygen_module.set("styles", styles);

	v8pp::module keys;
#define KEY(name) keys.set_const(#name, KEY_##name)
	KEY(BACKSPACE);	KEY(TAB); KEY(RETURN);
	KEY(LEFT_SHIFT); KEY(RIGHT_SHIFT);
	KEY(LEFT_CONTROL); KEY(RIGHT_CONTROL);

	KEY(ESCAPE); KEY(SPACE);
	KEY(PAGE_UP); KEY(PAGE_DOWN);
	KEY(END); KEY(HOME);
	KEY(LEFT); KEY(UP); KEY(RIGHT); KEY(DOWN);
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
