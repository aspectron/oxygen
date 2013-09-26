#pragma once

#ifndef __GUI_XF86_HPP__
#define __GUI_XF86_HPP__

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <iostream>
#include <sstream>
#include <vector>

#include <GL/glx.h>
#include <set>
#include <string>

#include "gui.hpp"

namespace aspect { namespace gui {

extern Display		*g_display;
extern int			g_screen;
extern XIM			g_input_method;

Bool check_event(::Display*, XEvent* event, XPointer user_data);

class OXYGEN_API window : public window_base
{
public:
	typedef v8pp::class_<window, v8pp::v8_args_factory> js_class;

	static void init();
	static void cleanup();
	static void process_windows_events();

	explicit window(v8::Arguments const& args);

	~window() { destroy(); }

	void destroy();

	uint32_t width() const { return width_; }
	uint32_t height() const { return height_; }

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
	void switch_to_fullscreen(video_mode const& mode);
	void cleanup();

	void process_event(XEvent WinEvent);
	void process_events();
	void process_events_blocking();

private:
	Window window_;
	Atom atom_close_;
	int previous_video_mode_;
	Cursor hidden_cursor_;
	XIC input_context_;
	XVisualInfo current_visual_;

	bool fullscreen_;
	volatile bool terminating_;

	uint32_t width_, height_;
	aspect::event_handler<std::string> event_handlers_;
};

}} // aspect::gui

#endif // __GUI_XF86_HPP__
