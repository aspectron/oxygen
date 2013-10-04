#ifndef __GUI_HPP__
#define __GUI_HPP__

namespace aspect { namespace gui {

enum window_style
{
	GWS_NONE        = 0x00000000,
	GWS_TITLEBAR    = 0x00000001,
	GWS_RESIZE      = 0x00000002,
	GWS_CLOSE       = 0x00000004,
	GWS_FULLSCREEN  = 0x00000008,
	GWS_APPWINDOW   = 0x00000020,
};

struct creation_args
{
	int32_t left, top;
	uint32_t width, height;
	uint32_t bpp, style;
#if OS(WINDOWS)
	std::wstring caption;
	std::wstring splash;
#else
	std::string caption;
	std::string splash;
#endif

	explicit creation_args(v8::Arguments const& args);
};

struct graphics_settings
{
	explicit graphics_settings(unsigned int depth = 0, unsigned int stencil = 0, unsigned int antialiasing = 0)
		: depth_bits(depth)
		, stencil_bits(stencil)
		, antialiasing_level(antialiasing)
	{
	}

	unsigned int depth_bits;
	unsigned int stencil_bits;
	unsigned int antialiasing_level;
};

struct OXYGEN_API input_event
{
	std::string type;
	math::vec2 cursor;
	uint32_t vk_code;
	uint32_t scancode;
	uint32_t charcode;
	char char_[2];

	bool mod_ctrl;
	bool mod_alt;
	bool mod_rshift;
	bool mod_lshift;
	bool mod_shift;

	explicit input_event(std::string const& type)
		: type(type)
		, cursor(0, 0)
		, vk_code(0)
		, scancode(0)
		, charcode(0)
		, char_()
		, mod_ctrl(false)
		, mod_alt(false)
		, mod_rshift(false)
		, mod_lshift(false)
		, mod_shift(false)
	{
#if OS(WINDOWS)
		mod_ctrl = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
		mod_alt =  HIWORD(GetAsyncKeyState(VK_MENU)) != 0;
		mod_lshift = HIWORD(GetAsyncKeyState(VK_SHIFT)) != 0;
		mod_rshift = HIWORD(GetAsyncKeyState(VK_LSHIFT)) != 0;
		mod_shift = mod_lshift || mod_rshift;
#else
		// todo-linux
#endif
	}
};

class OXYGEN_API oxygen_thread
{
public:

	/// Callback function to schedule in Berkelium
	typedef boost::function<void ()> callback;

	/// Schedule function call in the windows thread
	static bool schedule(callback cb);

	// Check that caller in the windows thread
	static bool is_window_thread()
	{
		return boost::this_thread::get_id() == global_->thread_.get_id();
	}

	static void start();
	static void stop();

	~oxygen_thread();
private:
	oxygen_thread();

	boost::thread thread_;

	class main_loop;
	boost::scoped_ptr<main_loop> main_loop_;

	boost::scoped_ptr<async_queue> task_queue_;

	static boost::scoped_ptr<oxygen_thread> global_;

};

class OXYGEN_API window_base
{
	friend class event_sink;
public:

#if OS(WINDOWS)
	bool process_event_by_sink(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);
#endif

private:
	typedef std::list<event_sink*> event_sinks;
	event_sinks event_sinks_;
};

class OXYGEN_API event_sink
{
public:
	explicit event_sink(window_base& window)
		: window_(window)
	{
		window_.event_sinks_.push_back(this);
	}

	virtual ~event_sink()
	{
		window_.event_sinks_.remove(this);
	}

#if OS(WINDOWS)
	virtual bool process_events(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) = 0;
#endif

private:
	window_base& window_;
};

}} // aspect::gui

#endif // __GUI_HPP__
