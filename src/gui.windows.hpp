#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)

#include <boost/atomic.hpp>
#include <boost/scoped_array.hpp>

#include "geometry.hpp"
#include "gui.hpp"

namespace aspect { namespace gui {

class OXYGEN_API window : public window_base
{
public:
	typedef v8pp::class_<window, v8pp::v8_args_factory> js_class;

	static void init();
	static void cleanup();

	explicit window(creation_args const& args) { init(args); }
	explicit window(v8::Arguments const& args) { init(creation_args(args)); }

	~window() { destroy(); }

	void destroy();

	operator HWND() const { return hwnd_; }

	window& on(std::string const& name, v8::Handle<v8::Function> fn);
	window& off(std::string const& name);

	rectangle<int> rect() const;
	void set_rect(rectangle<int> const& rect);

	void show_frame(bool show);
	void set_topmost(bool topmost);
	void use_as_splash_screen(std::wstring const& filename);
	void load_icon_from_file(std::wstring const& filename);

	void set_cursor(HCURSOR cursor);
	void show_mouse_cursor(bool show);
	void show(bool visible);
	void switch_to_fullscreen(video_mode const& mode);
	void set_focus();
	void toggle_fullscreen();
private:
	void init(creation_args const& args);

// handlers in the window thread
	static void message_loop();
	static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	void create(creation_args args);
	void update_window_size();
	bool process(event& e);

	typedef std::vector<std::wstring> wstrings;
	typedef boost::shared_ptr<wstrings> shared_wstrings;

	void drag_accept_files_enable_impl(bool enable);
	void drag_accept_files(shared_wstrings files);

private:
	// handlers in V8 thread
	void v8_process_message(uint32_t message, uint32_t wparam, uint32_t lparam);

private:
	HWND hwnd_;

	HCURSOR cursor_;
	bool fullscreen_;
	WINDOWPLACEMENT prev_placement_;

	bool message_handling_enabled_;
	bool drag_accept_files_enabled_;

	boost::scoped_array<uint8_t> splash_bitmap_;
};

}} // aspect::gui

#endif // OS(WINDOWS)

#endif // __WINDOW_WINXX_HPP__
