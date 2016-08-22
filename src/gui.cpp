#include "oxygen/oxygen.hpp"

#include <cassert>

#include "nitrogen/geometry.hpp"

#include <v8pp/class.hpp>
#include <v8pp/object.hpp>

namespace aspect {  namespace gui {

creation_args::creation_args(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	v8::HandleScope scope(isolate);

	v8::Local<v8::Object> options = args[0]->ToObject();
	if (options.IsEmpty() || options->IsUndefined())
	{
		throw std::runtime_error("Window constructor requires configuration object as an argument");
	}

	display disp = display::primary();
	v8pp::get_option(isolate, options, "display", disp);
	display::mode const& curr_mode = disp.current_mode();

	v8pp::get_option(isolate, options, "width", width = curr_mode.width);
	v8pp::get_option(isolate, options, "height", height = curr_mode.height);
	v8pp::get_option(isolate, options, "left", left = std::max(int(curr_mode.width - width) / 2, 0));
	v8pp::get_option(isolate, options, "top", top = std::max(int(curr_mode.height - height) / 2, 0));
	v8pp::get_option(isolate, options, "bpp", bpp = curr_mode.bpp);
	v8pp::get_option(isolate, options, "style", style = GWS_TITLEBAR | GWS_RESIZE | GWS_CLOSE | GWS_APPWINDOW);
	v8pp::get_option(isolate, options, "caption", caption);
	v8pp::get_option(isolate, options, "splash", splash);
	v8pp::get_option(isolate, options, "icon", icon);
}

static char const* const types[] =
{
	"unknown", "keydown", "keyup", "char",
	"mousemove", "mousewheel", "mousedown", "mouseup", "mouseclick",
};
static size_t const type_count = sizeof types / sizeof(*types);

std::string input_event::type_to_str(event_type type)
{
	assert(type < type_count && "unknown type string");
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

v8::Handle<v8::Value> input_event::to_v8(v8::Isolate* isolate) const
{
	v8::EscapableHandleScope scope(isolate);

	v8::Local<v8::Object> object = v8::Object::New(isolate);
	v8pp::set_option(isolate, object, "type", type_str());

	if (type() != UNKNOWN)
	{
		v8::Local<v8::Object> modifiers = v8::Object::New(isolate);
		v8pp::set_option(isolate, object, "modifiers", modifiers);

		v8pp::set_option(isolate, modifiers, "ctrl",     ctrl());
		v8pp::set_option(isolate, modifiers, "alt",      alt());
		v8pp::set_option(isolate, modifiers, "shift",    shift());
		v8pp::set_option(isolate, modifiers, "lbutton",  lbutton());
		v8pp::set_option(isolate, modifiers, "mbutton",  mbutton());
		v8pp::set_option(isolate, modifiers, "rbutton",  rbutton());
		v8pp::set_option(isolate, modifiers, "xbutton1", xbutton1());
		v8pp::set_option(isolate, modifiers, "xbutton2", xbutton2());

		v8pp::set_option(isolate, object, "repeats",  repeats());

		if (is_key())
		{
			v8pp::set_option(isolate, object, "vk_code",  vk_code());
			v8pp::set_option(isolate, object, "scan_code", scan_code());
			v8pp::set_option(isolate, object, "key_code", key_code());

			uint32_t const ch = character();
#if defined(_WIN32)
			v8pp::set_option(isolate, object, "char", std::wstring(ch? 1 : 0, static_cast<wchar_t>(ch)));
#else
			std::string str;
			if (ch) utils::to_utf8(&ch, &ch + 1, std::back_inserter(str));
			v8pp::set_option(isolate, object, "char", str);
#if !defined(__APPLE__)
			char const* const keysym = XKeysymToString(vk_code());
			set_option(isolate, object, "key_sym", keysym? keysym : "");
#endif
#endif
		}
		else if (is_mouse())
		{
			if (button()) v8pp::set_option(isolate, object, "button", button());
			v8pp::set_option(isolate, object, "x",  x());
			v8pp::set_option(isolate, object, "y",  y());
			v8pp::set_option(isolate, object, "dx", dx());
			v8pp::set_option(isolate, object, "dy", dy());
		}
	}
	return scope.Escape(object);
}

input_event input_event::from_v8(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
	input_event result;
	result.type_and_state_ = UNKNOWN;

	v8::HandleScope scope(isolate);
	v8::Local<v8::Object> object = value.As<v8::Object>();
	if (object.IsEmpty() || object->IsUndefined())
	{
		return result;
	}

	std::string type_str;
	v8pp::get_option(isolate, object, "type", type_str);
	result.type_and_state_ = type_from_str(type_str);

	if (result.type() == UNKNOWN)
	{
		return result;
	}

	v8::Local<v8::Object> modifiers;
	if (v8pp::get_option(isolate, object, "modifiers", modifiers))
	{
		bool b;

		v8pp::get_option(isolate, modifiers, "ctrl", b = false);
		result.type_and_state_ |= b? CTRL_DOWN : 0;
		v8pp::get_option(isolate, modifiers, "alt", b = false);
		result.type_and_state_ |= b? ALT_DOWN : 0;
		v8pp::get_option(isolate, modifiers, "shift", b = false);
		result.type_and_state_ |= b? SHIFT_DOWN : 0;

		v8pp::get_option(isolate, modifiers, "lbutton", b = false);
		result.type_and_state_ |= b? LBUTTON_DOWN : 0;
		v8pp::get_option(isolate, modifiers, "mbutton", b = false);
		result.type_and_state_ |= b? MBUTTON_DOWN : 0;
		v8pp::get_option(isolate, modifiers, "rbutton", b = false);
		result.type_and_state_ |= b? RBUTTON_DOWN : 0;
		v8pp::get_option(isolate, modifiers, "xbutton1", b = false);
		result.type_and_state_ |= b? XBUTTON1_DOWN : 0;
		v8pp::get_option(isolate, modifiers, "xbutton2", b = false);
		result.type_and_state_ |= b? XBUTTON2_DOWN : 0;
	}

	v8pp::get_option(isolate, object, "repeats", result.repeats_ = 0);

	if (result.is_key())
	{
		v8pp::get_option(isolate, object, "vk_code",  result.data_.key.vk_code = 0);
		v8pp::get_option(isolate, object, "scan_code", result.data_.key.scan_code = 0);
		v8pp::get_option(isolate, object, "key_code",  result.data_.key.key_code = 0);
		result.data_.key.char_code = 0;
#if defined(_WIN32)
		std::wstring str;
		if (v8pp::get_option(isolate, object, "char", str) && !str.empty())
		{
			result.data_.key.char_code = str[0];
		}
#else
		std::string str;
		if (v8pp::get_option(isolate, object, "char", str) && !str.empty())
		{
			utils::from_utf8(str.begin(), str.end(), &result.data_.key.char_code);
		}
#endif
	}
	else if (result.is_mouse())
	{
		uint32_t button = 0;
		v8pp::get_option(isolate, object, "button", button);
		result.type_and_state_ |= button? (button << BUTTON_SHIFT) & BUTTON_MASK : 0;
		v8pp::get_option(isolate, object, "x",  result.data_.mouse.x = 0);
		v8pp::get_option(isolate, object, "y",  result.data_.mouse.y = 0);
		v8pp::get_option(isolate, object, "dx", result.data_.mouse.dx = 0);
		v8pp::get_option(isolate, object, "dy", result.data_.mouse.dy = 0);
	}

	return result;
}

void window_base::on_resize(box<int> const& new_size)
{
	std::for_each(event_sinks_.begin(), event_sinks_.end(),
		[&new_size](event_sink* sink) { sink->on_resize(new_size); });

	if (has("resize"))
	{
		call_in_node([this, new_size](){ on_resize_v8(new_size); });
	}
}

void window_base::on_screen_change()
{
	std::for_each(event_sinks_.begin(), event_sinks_.end(),
		[](event_sink* sink) { sink->on_screen_change(); });
}

void window_base::on_input(input_event const& inp_e)
{
	if (inp_e.type() != input_event::UNKNOWN)
	{
		std::for_each(event_sinks_.begin(), event_sinks_.end(),
			[&inp_e](event_sink* sink) { sink->on_input(inp_e); });

		if (has(inp_e.type_str()))
		{
			call_in_node([this, inp_e](){ on_input_v8(inp_e); });
		}
	}
}

void window_base::on_event(std::string const& type)
{
	if (has(type))
	{
		call_in_node([this, type](){ on_event_v8(type); });
	}
}

void window_base::on_resize_v8(box<int> new_size)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);

	v8::Handle<v8::Value> args[1] = { v8pp::to_v8(isolate, new_size) };
	emit("resize", v8pp::to_v8<event_emitter*>(isolate, this), 1, args);
}

void window_base::on_input_v8(input_event inp_e)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);

	v8::Handle<v8::Value> args[1] = { inp_e.to_v8(isolate) };
	emit(inp_e.type_str(), v8pp::to_v8<event_emitter*>(isolate, this), 1, args);
}

void window_base::on_event_v8(std::string type)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	v8::HandleScope scope(isolate);

	emit(type, v8pp::to_v8<event_emitter*>(isolate, this), 0, nullptr);
}

}} // aspect::gui
