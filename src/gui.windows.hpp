#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)

#include <boost/atomic.hpp>
#include <boost/scoped_array.hpp>

namespace aspect { namespace gui {

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

	window& on(std::string const& name, v8::Handle<v8::Value> fn);
	window& off(std::string const& name);

	void set_window_rect(uint32_t left, uint32_t top, uint32_t width, uint32_t height);
	v8::Handle<v8::Value> get_window_rect(v8::Arguments const&);
	v8::Handle<v8::Value> get_client_rect(v8::Arguments const&);

	void show_frame(bool show);
	void set_topmost(bool topmost);
	void use_as_splash_screen(std::wstring const& filename);
	void load_icon_from_file(std::wstring const& filename);

	void show_mouse_cursor(bool show);
	void show(bool visible);
	void switch_to_fullscreen(video_mode const& mode);

/*
	void get_size(uint32_t* width, uint32_t* height)
	{
		*width = width_;
		*height = height_;
	}
*/

private:
// handlers in the window thread
	static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

	void create(creation_args args);
	void update_window_size();
	bool process_event(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);

	typedef std::vector<std::wstring> wstrings;
	typedef boost::shared_ptr<wstrings> shared_wstrings;

	void drag_accept_files_enable_impl(bool enable);
	void drag_accept_files(shared_wstrings files);

private:
	// handlers in V8 thread
	void v8_process_message(uint32_t message, uint32_t wparam, uint32_t lparam);
	void v8_process_input_event(boost::shared_ptr<input_event> e);
	void v8_process_event(std::string const& type);
	void v8_process_resize(uint32_t width, uint32_t height);

private:
	boost::atomic<HWND> hwnd_;

	uint32_t width_, height_;
	unsigned style_;
	HCURSOR cursor_;
	bool fullscreen_;

	bool message_handling_enabled_;
	bool drag_accept_files_enabled_;

	boost::scoped_array<uint8_t> splash_bitmap_;

	aspect::event_handler<std::string> event_handlers_;
};

}} // aspect::gui

#endif // OS(WINDOWS)

#endif // __WINDOW_WINXX_HPP__
