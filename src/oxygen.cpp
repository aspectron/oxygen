#include "oxygen.hpp"
#include "library.hpp"

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

	window::js_class window_class;
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

	return oxygen_module.new_instance();
}

void oxygen_uninstall(Handle<Value> library)
{
	window::js_class::destroy_objects();
	window::cleanup();
}

}} // ::aspect::gui
