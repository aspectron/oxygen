#ifndef __GUI_HPP__
#define __GUI_HPP__

namespace aspect { namespace gui {

class window_base;
class window;

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
	uint32_t left, top, width, height, bpp, style;
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

class OXYGEN_API event_sink
{
public:
	event_sink()
		: window_(nullptr)
	{
	}

	virtual ~event_sink() {}

#if OS(WINDOWS)
	virtual bool process_events(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) = 0;
#endif

	void assoc(window_base* w);
	void unregister();

private:
	window_base* window_;
};

class OXYGEN_API input_event
{
public:

	input_event(std::string const& type)
		: type_(type)
	{
		init();
	}

	void init()
	{
#if OS(WINDOWS)
		vk_code_ = 0; scancode_ = charcode_ = 0;
		char_[0] = char_[1] = 0;
		mod_ctrl_ = HIWORD(GetAsyncKeyState(VK_CONTROL)) ? true : false;
		mod_alt_ =  HIWORD(GetAsyncKeyState(VK_MENU)) ? true : false;
		mod_lshift_ = HIWORD(GetAsyncKeyState(VK_SHIFT)) ? true : false;
		mod_rshift_ = HIWORD(GetAsyncKeyState(VK_LSHIFT)) ? true : false;
		mod_shift_ = mod_lshift_ || mod_rshift_;
#else
		// todo-linux
#endif
	}

	std::string type_;
	math::vec2 cursor_;
	uint32_t vk_code_;
	uint32_t scancode_;
	uint32_t charcode_;
	char char_[2];

	bool mod_ctrl_;
	bool mod_alt_;
	bool mod_rshift_;
	bool mod_lshift_;
	bool mod_shift_;

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
public:
	void register_event_sink(event_sink& sink);
	void unregister_event_sink(event_sink& sink);

#if OS(WINDOWS)
	bool process_event_by_sink(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);
#endif

private:
	typedef std::list<event_sink*> event_sinks;
	event_sinks event_sinks_;
};

}} // aspect::gui

#endif // __GUI_HPP__
