#include "oxygen.hpp"

namespace aspect {  namespace gui {


creation_args::creation_args(v8::Arguments const& args)
{
	using namespace v8;

	HandleScope scope;

	Handle<Object> options = args[0]->ToObject();
	if (options.IsEmpty())
	{
		throw std::runtime_error("Window constructor requires configuration object as an argument");
	}

	if (get_option(options, "width", width = 640))
	{
		width = min(width, 1024*10u);
	}
	if (get_option(options, "height", height = 480))
	{
		height = min(height, 1024*10u);
	}

	if (!get_option(options, "left", left))
	{
		left = max(int(get_current_video_mode().width - width) / 2, 0);
	}
	if (!get_option(options, "top", top))
	{
		top = max(int(get_current_video_mode().height - height) / 2, 0);
	}

	get_option(options, "bpp", bpp = 32);
	get_option(options, "style", style = GWS_TITLEBAR | GWS_RESIZE | GWS_CLOSE | GWS_APPWINDOW);
	get_option(options, "caption", caption);
	get_option(options, "splash", splash);
}


void window_base::on_resize(uint32_t width, uint32_t height)
{
	if (event_handlers_.has("resize"))
	{
		runtime::main_loop().schedule(boost::bind(&window_base::on_resize_v8, this, width, height));
	}
}

void window_base::on_input(input_event const& e)
{
	if (event_handlers_.has(e.type))
	{
		runtime::main_loop().schedule(boost::bind(&window::on_input_v8, this, e));
	}
}

void window_base::on_event(std::string const& type)
{
	if (event_handlers_.has(type))
	{
		runtime::main_loop().schedule(boost::bind(&window::on_event_v8, this, type));
	}
}

void window_base::on_resize_v8(uint32_t width, uint32_t height)
{
	v8::HandleScope scope;

	v8::Handle<v8::Object> o = v8::Object::New();
	set_option(o, "width", width);
	set_option(o, "height", height);

	v8::Handle<v8::Value> args[1] = { o };
	event_handlers_.call("resize", v8pp::to_v8(this)->ToObject(), 1, args);
}

void window_base::on_input_v8(input_event e)
{
	v8::HandleScope scope;

	v8::Handle<v8::Object> o = v8::Object::New();
	set_option(o, "type", e.type);

	v8::Handle<v8::Object> modifiers = v8::Object::New();
	set_option(o, "modifiers", modifiers);

	set_option(modifiers, "ctrl",   e.mod_ctrl);
	set_option(modifiers, "alt",    e.mod_alt);
	set_option(modifiers, "shift",  e.mod_shift);
	set_option(modifiers, "lshift", e.mod_lshift);
	set_option(modifiers, "rshift", e.mod_rshift);

	set_option(o, "vk_code",  e.vk_code);
	set_option(o, "scancode", e.scancode);
	set_option(o, "charcode", e.charcode);
	set_option(o, "char",     e.char_);

	v8::Handle<v8::Value> args[1] = { o };
	event_handlers_.call(e.type, v8pp::to_v8(this)->ToObject(), 1, args);
}

void window_base::on_event_v8(std::string type)
{
	event_handlers_.call(type, v8pp::to_v8(this)->ToObject(), 0, nullptr);
}

#if OS(WINDOWS)
bool window_base::process_event_by_sink(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result)
{
	for (event_sinks::iterator it = event_sinks_.begin(), end = event_sinks_.end(); it != end; ++it)
	{
		if ( (*it)->process_events(message, wparam, lparam, result))
		{
			return true;
		}
	}
	return false;
}
#endif

}} // aspect::gui
