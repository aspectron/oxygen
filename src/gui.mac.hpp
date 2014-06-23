#ifndef OXYGEN_GUI_MAC_HPP_INCLUDED
#define OXYGEN_GUI_MAC_HPP_INCLUDED

#include "geometry.hpp"
#include "gui.hpp"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
struct NSWindow;
struct NSView;
struct NSObject;
struct NSCursor;
#endif

namespace aspect { namespace gui {

class OXYGEN_API window : public window_base
{
public:
	static void init();
	static void cleanup();

	explicit window(creation_args const& args) { create(args); }
	explicit window(v8::Arguments const& args) { create(creation_args(args)); }

	~window() { destroy(); }

	void destroy();

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

	void show_mouse_cursor(bool show) {}
	void set_cursor(NSCursor* cursor);
	void set_stock_cursor(cursor_id id);
	void capture_mouse(bool capture) {}
	void set_mouse_pos(int x, int y) {}
	void show(bool visible);
	void set_focus();
	void toggle_fullscreen();

	rectangle<float> rect() const;
	void set_rect(rectangle<float> const& rect);

	void show_frame(bool show);
	void set_topmost(bool topmost);

	void load_icon_from_file(std::string const&);
	void use_as_splash_screen(std::string const& filename);

private:
	void create(creation_args args);

	unsigned style_mask_;
	bool fullscreen_;
	int level_;
	rectangle<float> rect_;
public:
// access from NSWindowDelegate
	NSWindow* object;
	NSObject* delegate;
	NSView* view;

	void handle_input(event& e);
	void handle_resize();
	void handle_close();
};

}} // aspect::gui

#endif // OXYGEN_GUI_MAC_HPP_INCLUDED
