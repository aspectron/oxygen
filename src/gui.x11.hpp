#ifndef OXYGEN_GUI_X11_HPP_INCLUDED
#define OXYGEN_GUI_X11_HPP_INCLUDED

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "geometry.hpp"
#include "gui.hpp"

namespace aspect { namespace gui {

extern Display* g_display;
extern int g_screen;
extern Window g_root;
extern XIM g_input_method;

struct randr_info {
	bool is_available;
	int event_base;
	int error_base;
	int version_major;
	int version_minor;
};

extern randr_info randr;

class OXYGEN_API window : public window_base
{
public:
	static void init();
	static void cleanup();

	explicit window(v8::FunctionCallbackInfo<v8::Value> const& args);

	~window() { destroy(); }

	void destroy();

	operator Window() const { return window_; }

	XVisualInfo& current_visual() { return current_visual_; }

	window& on(std::string const& name, v8::Handle<v8::Function> fn)
	{
		window_base::on(rt_.isolate(), name, fn);
		return *this;
	}

	window& off(std::string const& name)
	{
		window_base::off(rt_.isolate(), name);
		return *this;
	}

	void show_mouse_cursor(bool show);
	void set_stock_cursor(cursor_id id);
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
	bool switch_to_fullscreen(display::mode const& mode);
	void _cleanup();

	void process(XEvent& event);
	static void process_events();

private:
	Window window_;
	Atom atom_close_;
	int previous_video_mode_;
	Cursor hidden_cursor_, current_cursor_;
	XIC input_context_;
	XVisualInfo current_visual_;

	uint32_t pressed_key_code_;
	uint32_t pressed_char_code_;
	boost::atomic<int> capture_count_;

	friend class input_event; // to access input_context_
};

}} // aspect::gui

#endif // OXYGEN_GUI_X11_HPP_INCLUDED
