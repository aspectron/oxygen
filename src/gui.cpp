#include "oxygen.hpp"

#include <boost/algorithm/cxx11/any_of.hpp>

namespace aspect {  namespace gui {

using namespace v8;

creation_args::creation_args(Arguments const& args)
{
	HandleScope scope;

	Handle<Object> options = args[0]->ToObject();
	if (options.IsEmpty() || options->IsUndefined())
	{
		throw std::runtime_error("Window constructor requires configuration object as an argument");
	}

	video_mode const& curr_mode = get_current_video_mode();

	get_option(options, "width", width = curr_mode.width);
	get_option(options, "height", height = curr_mode.height);
	get_option(options, "left", left = max(int(curr_mode.width - width) / 2, 0));
	get_option(options, "top", top = max(int(curr_mode.height - height) / 2, 0));
	get_option(options, "bpp", bpp = curr_mode.bpp);
	get_option(options, "style", style = GWS_TITLEBAR | GWS_RESIZE | GWS_CLOSE | GWS_APPWINDOW);
	get_option(options, "caption", caption);
	get_option(options, "splash", splash);
}

static char const* const types[] =
{
	"unknown", "keydown", "keyup", "char",
	"mousemove", "mousewheel", "mousedown", "mouseup", "mouseclick",
};
static size_t const type_count = sizeof types / sizeof(*types);

std::string input_event::type_to_str(event_type type)
{
	_aspect_assert(type < type_count && "unknown type string");
	return type < type_count? types[type] : types[UNKNOWN];
}

input_event::event_type input_event::type_from_str(std::string const& str)
{
	for (size_t i = 0; i < type_count; ++i)
	{
		if (types[i] == str)
		{
			return static_cast<event_type>(i);
		}
	}
	return UNKNOWN;
}

Handle<Value> input_event::to_v8() const
{
	HandleScope scope;

	Handle<Object> object = Object::New();
	set_option(object, "type", type_str());

	if (type() != UNKNOWN)
	{
		Handle<Object> modifiers = Object::New();
		set_option(object, "modifiers", modifiers);

		set_option(modifiers, "ctrl",     ctrl());
		set_option(modifiers, "alt",      alt());
		set_option(modifiers, "shift",    shift());
		set_option(modifiers, "lbutton",  lbutton());
		set_option(modifiers, "mbutton",  mbutton());
		set_option(modifiers, "rbutton",  rbutton());
		set_option(modifiers, "xbutton1", xbutton1());
		set_option(modifiers, "xbutton2", xbutton2());

		set_option(object, "repeats",  repeats());

		if (is_key())
		{
			set_option(object, "vk_code",  vk_code());
			set_option(object, "scan_code", scan_code());
			set_option(object, "key_code", key_code());

			uint32_t const ch = character();
#if OS(WINDOWS)
			set_option(object, "char", std::wstring(ch? 1 : 0, ch));
#else
			std::string str;
			if (ch) utils::to_utf8(&ch, &ch + 1, std::back_inserter(str));
			set_option(object, "char", str);
			char const* const keysym = XKeysymToString(vk_code());
			set_option(object, "key_sym", keysym? keysym : "");
#endif
		}
		else if (is_mouse())
		{
			if (button()) set_option(object, "button", button());
			set_option(object, "x",  x());
			set_option(object, "y",  y());
			set_option(object, "dx", dx());
			set_option(object, "dy", dy());
		}
	}
	return scope.Close(object);
}

input_event input_event::from_v8(v8::Handle<v8::Value> value)
{
	input_event result;
	result.type_and_state_ = UNKNOWN;

	HandleScope scope;
	Handle<Object> object = value.As<Object>();
	if (object.IsEmpty() || object == Undefined())
	{
		return result;
	}

	std::string type_str;
	get_option(object, "type", type_str);
	result.type_and_state_ = type_from_str(type_str);

	if (result.type() == UNKNOWN)
	{
		return result;
	}

	Handle<Object> modifiers;
	if (get_option(object, "modifiers", modifiers))
	{
		bool b;

		get_option(modifiers, "ctrl", b = false);
		result.type_and_state_ |= b? CTRL_DOWN : 0;
		get_option(modifiers, "alt", b = false);
		result.type_and_state_ |= b? ALT_DOWN : 0;
		get_option(modifiers, "shift", b = false);
		result.type_and_state_ |= b? SHIFT_DOWN : 0;

		get_option(modifiers, "lbutton", b = false);
		result.type_and_state_ |= b? LBUTTON_DOWN : 0;
		get_option(modifiers, "mbutton", b = false);
		result.type_and_state_ |= b? MBUTTON_DOWN : 0;
		get_option(modifiers, "rbutton", b = false);
		result.type_and_state_ |= b? RBUTTON_DOWN : 0;
		get_option(modifiers, "xbutton1", b = false);
		result.type_and_state_ |= b? XBUTTON1_DOWN : 0;
		get_option(modifiers, "xbutton2", b = false);
		result.type_and_state_ |= b? XBUTTON2_DOWN : 0;
	}

	get_option(object, "repeats", result.repeats_ = 0);

	if (result.is_key())
	{
		get_option(object, "vk_code",  result.data_.key.vk_code = 0);
		get_option(object, "scan_code", result.data_.key.scan_code = 0);
		get_option(object, "key_code",  result.data_.key.key_code = 0);
		result.data_.key.char_code = 0;
#if OS(WINDOWS)
		std::wstring str;
		if (get_option(object, "char", str) && !str.empty())
		{
			result.data_.key.char_code = str[0];
		}
#else
		std::string str;
		if (get_option(object, "char", str) && !str.empty())
		{
			utils::from_utf8(str.begin(), str.end(), &result.data_.key.char_code);
		}
#endif
	}
	else if (result.is_mouse())
	{
		uint32_t button = 0;
		get_option(object, "button", button);
		result.type_and_state_ |= button? (button << BUTTON_SHIFT) & BUTTON_MASK : 0;
		get_option(object, "x",  result.data_.mouse.x = 0);
		get_option(object, "y",  result.data_.mouse.y = 0);
		get_option(object, "dx", result.data_.mouse.dx = 0);
		get_option(object, "dy", result.data_.mouse.dy = 0);
	}

	return result;
}

bool window_base::preprocess_by_sink(event& e)
{
	return boost::algorithm::any_of(event_sinks_.begin(), event_sinks_.end(),
		[&e](event_sink* sink) { return sink->preprocess(e); });
}

bool window_base::postprocess_by_sink(event& e)
{
	return boost::algorithm::any_of(event_sinks_.begin(), event_sinks_.end(),
		[&e](event_sink* sink) { return sink->postprocess(e); });
}

void window_base::on_resize(uint32_t width, uint32_t height)
{
	if (has("resize"))
	{
		runtime::main_loop().schedule(boost::bind(&window_base::on_resize_v8, this, width, height));
	}
}

void window_base::on_input(input_event const& e)
{
	if (e.type() != input_event::UNKNOWN && has(e.type_str()))
	{
		runtime::main_loop().schedule(boost::bind(&window::on_input_v8, this, e));
	}
}

void window_base::on_event(std::string const& type)
{
	if (has(type))
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
	emit("resize", 1, args);
}

void window_base::on_input_v8(input_event e)
{
	v8::HandleScope scope;

	v8::Handle<v8::Value> args[1] = { e.to_v8() };
	emit(e.type_str(), 1, args);
}

void window_base::on_event_v8(std::string type)
{
	emit(type, 0, nullptr);
}

}} // aspect::gui

