#ifndef OXYGEN_GUI_MAC_HPP_INCLUDED
#define OXYGEN_GUI_MAC_HPP_INCLUDED

#include "jsx/geometry.hpp"
#include "oxygen/gui.hpp"

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

	explicit window(v8::FunctionCallbackInfo<v8::Value> const& args);
	~window() { destroy(); }

	void destroy();

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

	void run_file_dialog(v8::FunctionCallbackInfo<v8::Value> const& args);

	/// Backing size
	box<int> const& backing_size() const { return backing_size_; }

private:
	void create(creation_args args);

	unsigned style_mask_;
	bool fullscreen_;
	int level_;
	rectangle<float> rect_;

	box<int> backing_size_;

public:
// access from NSWindowDelegate
	NSWindow* object;
	NSObject* delegate;
	NSView* view;

	void handle_input(event e);
	void handle_resize();
	void handle_backing_change();
	void handle_close();
};

}} // aspect::gui

#endif // OXYGEN_GUI_MAC_HPP_INCLUDED
