#include "oxygen.hpp"
#include "library.hpp"

namespace aspect { namespace gui {

using namespace v8;

Handle<Value> get_screen_size(Arguments const&)
{
	HandleScope scope;

	video_mode const curr_mode = get_current_video_mode();

	Handle<Object> o = Object::New();
	o->Set(v8pp::to_v8("width"), v8pp::to_v8(curr_mode.width));
	o->Set(v8pp::to_v8("height"), v8pp::to_v8(curr_mode.height));

	return scope.Close(o);
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
	window::cleanup();
	window::js_class::destroy_objects();
}

template<typename T>
T get_option(Handle<Object> options, char const* name, T def_value)
{
	Handle<Value> value = options->Get(String::New(name));
	return value.IsEmpty() || value == Undefined()? def_value : v8pp::from_v8<T>(value);
}

creation_args::creation_args(v8::Arguments const& args)
{
	HandleScope scope;

	Handle<Object> options = args[0]->ToObject();
	if (options.IsEmpty())
	{
		throw std::runtime_error("Window constructor requires configuration object as an argument");
	}

	left = max(get_option(options, "left", 0), 0);
	top = max(get_option(options, "top", 0), 0);
	width = min(get_option(options, "width", 640), 1024*10);
	height = min(get_option(options, "height", 480), 1024*10);
	bpp = get_option(options, "bpp", 32);
	style = get_option(options, "style", GWS_TITLEBAR | GWS_RESIZE | GWS_CLOSE | GWS_APPWINDOW);
	caption = get_option(options, "caption", caption);
	splash = get_option(options, "splash", splash);
}

}} // ::aspect::gui
