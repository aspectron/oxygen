#pragma once

#ifndef __GUI_XF86_HPP__
#define __GUI_XF86_HPP__

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "gui.hpp"

namespace aspect { namespace gui {

extern Display* g_display;
extern int g_screen;
extern XIM g_input_method;

class OXYGEN_API window : public window_base
{
public:
	typedef v8pp::class_<window, v8pp::v8_args_factory> js_class;

	static void init();
	static void cleanup();

	explicit window(v8::Arguments const& args);

	~window() { destroy(); }

	void destroy();

	operator Window&() { return window_; }

	XVisualInfo& current_visual() { return current_visual_; }

	window& on(std::string const& name, v8::Handle<v8::Value> fn)
	{
		event_handlers_.on(name, fn);
		return *this;
	}

	window& off(std::string const& name)
	{
		event_handlers_.off(name);
		return *this;
	}

	void show_mouse_cursor(bool show);
	void show(bool visible);

	void show_frame(bool show) { }
	void set_topmost(bool topmost) { }

	void set_window_rect(uint32_t left, uint32_t top, uint32_t width, uint32_t height) { }
	v8::Handle<v8::Value> get_window_rect(v8::Arguments const&) { }
	v8::Handle<v8::Value> get_client_rect(v8::Arguments const&) { }

	void load_icon_from_file(std::string const&) { }
	void use_as_splash_screen(std::string filename) { }

private:
	void create(creation_args const& args);
	void _init();
	void create_hidden_cursor();
	bool switch_to_fullscreen(video_mode const& mode);
	void _cleanup();

	void process_event(XEvent const& event);
	static void process_events();

private:
	Window window_;
	Atom atom_close_;
	int previous_video_mode_;
	Cursor hidden_cursor_;
	XIC input_context_;
	XVisualInfo current_visual_;
};

}} // aspect::gui

#endif // __GUI_XF86_HPP__
