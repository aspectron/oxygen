#ifndef OXYGEN_DISPLAY_HPP_INCLUDED
#define OXYGEN_DISPLAY_HPP_INCLUDED

#include <vector>
#include <tuple>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreGraphics/CGDirectDisplay.h>
#else
#include <X11/extensions/Xrandr.h>
#endif

#include "nitrogen/geometry.hpp"

namespace aspect { namespace gui {

class window;

/// Display information
struct OXYGEN_API display
{
	/// Enumerate all available displays
	static std::vector<display> enumerate();

	/// Get primary display
	static display primary();

	/// Get display information for a window belongs to
	/// If no window is supplied, use the display for the active window
	static display from_window(window const* w = nullptr);

#if defined(_WIN32)
	std::wstring name;
#elif defined(__APPLE__)
	CGDirectDisplayID id;
	std::string name;
#else
	RRCrtc crtc;
	RROutput output;
	std::string name;
#endif

	bool operator==(display const& other) const
	{
		return name == other.name;
	}

	bool operator!=(display const& other) const
	{
		return !(*this == other);
	}

	/// Ratio between physical and logical pixels
	float scale;

	/// Color depth in bits per pixel
	unsigned color_depth;

	/// Color depth per component, assumed the colors are balanced equally
	unsigned color_depth_per_component;

	/// Screen rectangle
	rectangle<int> rect;

	/// Available rectangle
	rectangle<int> work_rect;

	/// Display mode information
	struct mode
	{
	public:

		unsigned width;      ///< display width in pixels
		unsigned height;     ///< display height in pixels
		unsigned bpp;        ///< display color depth, bits per pixel
		unsigned frequency;  ///< display refresh rate, Hz

		mode(unsigned width, unsigned height, unsigned bpp, unsigned frequency)
			: width(width)
			, height(height)
			, bpp(bpp)
			, frequency(frequency)
		{
		}

		bool operator==(mode const& other) const
		{
			return width == other.width && height == other.height
				&& bpp == other.bpp && frequency == other.frequency;
		}

		bool operator<(mode const& other) const
		{
			return std::tie(bpp, width, height, frequency)
				< std::tie(other.bpp, other.width, other.height, other.frequency);
		}
	};

	/// List of supported modes
	std::vector<mode> modes() const;

	/// Current mode
	mode current_mode() const;
};

}} // aspect::gui

template<>
struct v8pp::convert<aspect::gui::display::mode>
{
	using from_type =  aspect::gui::display::mode;
	using to_type = v8::Handle<v8::Object>;

	static bool is_valid(v8::Isolate*, v8::Handle<v8::Value> value)
	{
		return value->IsObject();
	}

	static from_type from_v8(v8::Isolate* isolate, v8::Handle<v8::Value> value)
	{
		if (!value->IsObject())
		{
			throw std::invalid_argument("expected Object");
		}

		v8::HandleScope scope(isolate);
		v8::Local<v8::Object> obj = value->ToObject();

		unsigned width, height, bpp, frequency = 0;
		if (!get_option(isolate, obj, "width", width) ||
			!get_option(isolate, obj, "height", height) ||
			!get_option(isolate, obj, "colorDepth", bpp))
		{
			throw std::invalid_argument("expected {width, height, colorDepth, frequency} object");
		}
		get_option(isolate, obj, "frequency", frequency);
		return from_type(width, height, bpp, frequency);
	}

	static to_type to_v8(v8::Isolate* isolate, aspect::gui::display::mode const& value)
	{
		v8::EscapableHandleScope scope(isolate);

		v8::Local<v8::Object> obj = v8::Object::New(isolate);
		set_option(isolate, obj, "width", value.width);
		set_option(isolate, obj, "height", value.height);
		set_option(isolate, obj, "colorDepth", value.bpp);
		set_option(isolate, obj, "frequency", value.frequency);

		return scope.Escape(obj);
	}
};

#endif // OXYGEN_DISPLAY_HPP_INCLUDED
