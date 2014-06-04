#ifndef OXYGEN_GUI_WINDOWS_HPP_INCLUDED
#define OXYGEN_GUI_WINDOWS_HPP_INCLUDED

#include <boost/atomic.hpp>
#include <boost/scoped_array.hpp>

#include "geometry.hpp"
#include "gui.hpp"

namespace aspect { namespace gui {

class OXYGEN_API window : public window_base
{
public:
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
	void set_stock_cursor(cursor_id id);
	void show_mouse_cursor(bool show);
	void capture_mouse(bool capture);
	void set_mouse_pos(int x, int y);
	void show(bool visible);
	void switch_to_fullscreen(video_mode const& mode);
	void set_focus();
	void toggle_fullscreen();

	v8::Handle<v8::Value> run_file_dialog(v8::Arguments const& args);
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
	bool forced_cursor_, is_cursor_visible_;
	bool fullscreen_;
	WINDOWPLACEMENT prev_placement_;
	boost::atomic<int> capture_count_;

	bool message_handling_enabled_;
	bool drag_accept_files_enabled_;

	boost::scoped_array<uint8_t> splash_bitmap_;
};

}} // aspect::gui

#endif // OXYGEN_GUI_WINDOWS_HPP_INCLUDED
