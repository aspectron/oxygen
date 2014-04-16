#pragma once

#ifndef __GUI_XF86_HPP__
#define __GUI_XF86_HPP__

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "geometry.hpp"
#include "gui.hpp"

namespace aspect { namespace gui {

extern Display* g_display;
extern int g_screen;
extern XIM g_input_method;

class OXYGEN_API window : public window_base
{
public:
	static void init();
	static void cleanup();

	explicit window(creation_args const& args);
	explicit window(v8::Arguments const& args);

	~window() { destroy(); }

	void destroy();

	operator Window&() { return window_; }

	XVisualInfo& current_visual() { return current_visual_; }

	window& on(std::string const& name, v8::Handle<v8::Function> fn)
	{
		window_base::on(name, fn);
		return *this;
	}

	window& off(std::string const& name)
	{
		window_base::off(name);
		return *this;
	}

	void show_mouse_cursor(bool show);
	void capture_mouse(bool capture);
	void set_mouse_pos(int x, int y);
	void show(bool visible);
	void set_focus();
	void toggle_fullscreen() {}

	rectangle<int> rect() const;
	void set_rect(rectangle<int> const& rect);

	void show_frame(bool show) { }
	void set_topmost(bool topmost) { }

	void load_icon_from_file(std::string const&) { }
	void use_as_splash_screen(std::string filename) { }

private:
	void create(creation_args const& args);
	void _init();
	void create_hidden_cursor();
	bool switch_to_fullscreen(video_mode const& mode);
	void _cleanup();

	void process(XEvent& event);
	static void process_events();

private:
	Window window_;
	Atom atom_close_;
	int previous_video_mode_;
	Cursor hidden_cursor_;
	XIC input_context_;
	XVisualInfo current_visual_;

	uint32_t pressed_key_code_;
	uint32_t pressed_char_code_;
	boost::atomic<int> capture_count_;

	friend class input_event; // to access input_context_
};

}} // aspect::gui

#endif // __GUI_XF86_HPP__
