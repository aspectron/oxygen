#ifndef OXYGEN_GUI_HPP_INCLUDED
#define OXYGEN_GUI_HPP_INCLUDED

#if OS(WINDOWS)
#include <windows.h>
#elif OS(DARWIN)
	#ifdef __OBJC__
	#import <Cocoa/Cocoa.h>
	#else
	struct NSEvent;
	typedef void* id;
	#endif
#else
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
	int left, top, width, height;
	unsigned bpp, style;

#if OS(WINDOWS)
	std::wstring caption;
	std::wstring splash;
	std::wstring icon;
#else
	std::string caption;
	std::string splash;
	std::string icon;
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

#if OS(WINDOWS)
struct OXYGEN_API event
{
	HWND window;
	UINT message;
	WPARAM wparam;
	LPARAM lparam;
	LRESULT result;

	event(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
		: window(window)
		, message(message)
		, wparam(wparam)
		, lparam(lparam)
		, result(0)
	{
	} 
};
#elif OS(DARWIN)
typedef NSEvent* event;
#else
typedef XEvent event;
#endif

class OXYGEN_API input_event
{
public:
	explicit input_event(event const& e);

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
	int& x() { return data_.mouse.x; }

	// Mouse Y coordinate
	int y() const { return data_.mouse.y; }
	int& y() { return data_.mouse.y; }

	// Delta X for mouse wheel
	int dx() const { return data_.mouse.dx; }

	// Delta Y for mouse wheel
	int dy() const { return data_.mouse.dy; }

public:
// Key events

	// System dependent virtual key code, see key_code enum in keys.hpp
	uint32_t vk_code() const { return data_.key.vk_code; }

	// Keyboard OEM scan code
	uint32_t scan_code() const { return data_.key.scan_code; }

	// Native key code
	uint32_t key_code() const { return data_.key.key_code; }

	// Character for KEY_CHAR event
	uint32_t character() const { return data_.key.char_code; }

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
#elif OS(DARWIN)
	static uint32_t type_and_state(int type, unsigned int modifiers);
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
			uint32_t scan_code;
			uint32_t key_code;
			uint32_t char_code;
		} key;
	} data_;

	uint32_t repeats_;
};

class event_sink;

class OXYGEN_API window_base : public v8_core::event_emitter
{
	friend class event_sink;
public:
	window_base()
		: size_(0, 0)
		, style_(0)
	{
	}

	// Window size
	box<int> const& size() const { return size_; }

	int width() const { return size_.width; }
	int height() const { return size_.height; }

	enum cursor_id
	{
		ARROW, INPUT, HAND, CROSS, MOVE, WAIT,
	};
protected:
	bool preprocess_by_sink(event& e);
	bool postprocess_by_sink(event& e);

	void on_resize(box<int> const& new_size);
	void on_input(input_event const& e);
	void on_event(std::string const& type);

	box<int> size_;
	unsigned style_;

private:
//V8 handlers
	void on_resize_v8(box<int> new_size);
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

	/// Process event before window handler, return true to stop futher processing
	virtual bool preprocess(event& e) = 0;

	/// Process event after window handler, return true to stop futher processing
	virtual bool postprocess(event& e) = 0;

private:
	window_base& window_;
};

}} // aspect::gui

namespace v8pp {

template<>
struct convert<aspect::gui::input_event>
{
	typedef aspect::gui::input_event result_type;

	static bool is_valid(v8::Handle<v8::Value> value)
	{
		return value->IsObject();
	}

	static result_type from_v8(v8::Handle<v8::Value> value)
	{
		return aspect::gui::input_event::from_v8(value);
	}

	static v8::Handle<v8::Value> to_v8(aspect::gui::input_event const& ev)
	{
		return ev.to_v8();
	}
};

} //v8pp

#endif // OXYGEN_GUI_HPP_INCLUDED
