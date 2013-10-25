#ifndef __GUI_HPP__
#define __GUI_HPP__

#if !OS(WINDOWS)
#include <X11/Xlib.h>
#endif

namespace aspect { namespace gui {

enum window_style
{
	GWS_NONE        = 0x00000000,
	GWS_TITLEBAR    = 0x00000001,
	GWS_RESIZE      = 0x00000002,
	GWS_CLOSE       = 0x00000004,
	GWS_FULLSCREEN  = 0x00000008,
	GWS_APPWINDOW   = 0x00000020,
	GWS_HIDDEN      = 0x00000040,
};

struct OXYGEN_API creation_args
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

	creation_args();
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

class OXYGEN_API input_event
{
public:
#if OS(WINDOWS)
	input_event(UINT message, WPARAM wparam, LPARAM lparam);
#else
	explicit input_event(XKeyEvent const& xkey);
	explicit input_event(XButtonEvent const& xbutton);
	explicit input_event(XMotionEvent const& xmotion);
#endif

	enum event_type
	{
		UNKNOWN,
		KEY_DOWN, KEY_UP, KEY_CHAR,
		MOUSE_MOVE, MOUSE_WHEEL, MOUSE_DOWN, MOUSE_UP, MOUSE_CLICK,
	};

	// Event type
	event_type type() const { return static_cast<event_type>(type_and_state_ & TYPE_MASK); }

	// Is mouse event
	bool is_mouse() const { return type() >= MOUSE_MOVE && type() <= MOUSE_CLICK; }

	// Is key event
	bool is_key() const { return type() >= KEY_DOWN && type() <= KEY_CHAR; }

	// Even type string
	std::string type_str() const { return type_to_str(type()); }

	// Ctrl key is pressed down
	bool ctrl() const { return (type_and_state_ & CTRL_DOWN) != 0; }
	// Alt key is pressed down
	bool alt() const { return (type_and_state_ & ALT_DOWN) != 0; }
	// Shift key is pressed down
	bool shift() const { return (type_and_state_ & SHIFT_DOWN) != 0; }

	// Left mouse button is pressed down
	bool lbutton() const { return (type_and_state_ & LBUTTON_DOWN) != 0; }
	// Middle mouse button is pressed down
	bool mbutton() const { return (type_and_state_ & MBUTTON_DOWN) != 0; }
	// Right mouse button is pressed down
	bool rbutton() const { return (type_and_state_ & RBUTTON_DOWN) != 0; }
	// X1 mouse button is pressed down
	bool xbutton1() const { return (type_and_state_ & XBUTTON1_DOWN) != 0; }
	// X2 mouse button is pressed down
	bool xbutton2() const { return (type_and_state_ & XBUTTON2_DOWN) != 0; }

public:
// Mouse events

	// Mouse button number (0 -none, 1 - left, 2 -middle, 3 - right, 4, 5, ... - X1, X2, ...)
	uint32_t button() const { return (type_and_state_ & BUTTON_MASK) >> BUTTON_SHIFT; }

	// Mouse X coordinate
	int x() const { return data_.mouse.x; }
	// Mouse Y coordinate
	int y() const { return data_.mouse.y; }

	// Delta X for mouse wheel
	int dx() const { return data_.mouse.dx; }

	// Delta Y for mouse wheel
	int dy() const { return data_.mouse.dy; }

public:
// Key events

	// Virtual key code
	uint32_t vk_code() const { return data_.key.vk_code; }

	// OEM scan code
	uint32_t scancode() const { return data_.key.scancode; }

	// Character for KEY_CHAR event
	int character() const { return static_cast<int>(data_.key.vk_code); }

	// Click count for MOUSE_CLICK event, repeat count for KEY_DOWN event
	uint32_t repeats() const { return repeats_; }

public:
// V8 support

	// Convert input event to V8 value
	v8::Handle<v8::Value> to_v8() const;

	// Create an input_event form V8 value
	static input_event from_v8(v8::Handle<v8::Value>);

private:
	input_event() {}

	static std::string type_to_str(event_type type);
	static event_type type_from_str(std::string const& str);

	static uint32_t const TYPE_MASK   = 0x000000FF;
	static uint32_t const TYPE_SHIFT = 0;

	static uint32_t const BUTTON_MASK = 0x0000FF00;
	static uint32_t const BUTTON_SHIFT = 8;

	static uint32_t const STATE_MASK  = 0xFFFF0000;
	static uint32_t const STATE_SHIFT = 16;

#if OS(WINDOWS)
	static uint32_t mouse_type_and_state(UINT message, WPARAM wparam);
	static uint32_t key_type_and_state(UINT message);
#else
	static uint32_t type_and_state(int type, unsigned int state);
#endif

	enum state_flags
	{
		CTRL_DOWN     = 0x0001 << STATE_SHIFT,
		ALT_DOWN      = 0x0002 << STATE_SHIFT,
		SHIFT_DOWN    = 0x0004 << STATE_SHIFT,
		LBUTTON_DOWN  = 0x0008 << STATE_SHIFT,
		MBUTTON_DOWN  = 0x0010 << STATE_SHIFT,
		RBUTTON_DOWN  = 0x0020 << STATE_SHIFT,
		XBUTTON1_DOWN = 0x0040 << STATE_SHIFT,
		XBUTTON2_DOWN = 0x0080 << STATE_SHIFT,
	};

	uint32_t type_and_state_;

	union
	{
		struct
		{
			int x, y;
			int dx, dy;
		} mouse;

		struct key_data
		{
			uint32_t vk_code;
			uint32_t scancode;
		} key;
	} data_;

	uint32_t repeats_;
};

class event_sink;

class OXYGEN_API window_base
{
	friend class event_sink;
public:

	typedef v8pp::class_<window_base, v8pp::no_factory> js_class;

	window_base()
		: width_(0)
		, height_(0)
		, style_(0)
	{
	}

	uint32_t width() const { return width_; }
	uint32_t height() const { return height_; }

#if OS(WINDOWS)
	// message sinking mask
	static UINT const SINKING = 0x80000000;

	bool process_event_by_sink(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result);
#endif

protected:
	void on_resize(uint32_t width, uint32_t height);
	void on_input(input_event const& e);
	void on_event(std::string const& type);

	uint32_t width_, height_;
	unsigned style_;
	aspect::event_handler<std::string> event_handlers_;

private:
//V8 handlers
	void on_resize_v8(uint32_t width, uint32_t height);
	void on_input_v8(input_event e);
	void on_event_v8(std::string type);

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
	/// Process a message after window handler, return true on the message handled
	/// Unmask message with window::SINKING to process the message before the window handler
	virtual bool process_events(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result) = 0;
#endif

private:
	window_base& window_;
};

}} // aspect::gui

namespace v8pp {

namespace detail {

template<>
struct from_v8<::aspect::gui::input_event>
{
	typedef ::aspect::gui::input_event result_type;

	static result_type exec(v8::Handle<v8::Value> value)
	{
		return ::aspect::gui::input_event::from_v8(value);
	}
};

template<typename U>
struct from_v8_ref<::aspect::gui::input_event, U> : from_v8<::aspect::gui::input_event> {};

} // detail

inline v8::Handle<v8::Value> to_v8(::aspect::gui::input_event const& ev)
{
	return ev.to_v8();
}

} //v8pp

#endif // __GUI_HPP__
