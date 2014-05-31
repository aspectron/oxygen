#ifndef OXYGEN_VIDEO_MODES_HPP_INCLUDED
#define OXYGEN_VIDEO_MODES_HPP_INCLUDED

#include <set>
#include <tuple>
#include <boost/operators.hpp>

namespace aspect { namespace gui {

/// Video mode information
struct OXYGEN_API video_mode : boost::totally_ordered<video_mode>
{
public:

	unsigned width;      ///< display width in pixels
	unsigned height;     ///< display height in pixels
	unsigned bpp;        ///< display color depth, bits per pixel
	unsigned frequency;  ///< display refresh rate, Hz

	video_mode()
		: width(0)
		, height(0)
		, bpp(0)
		, frequency(0)
	{
	}

	video_mode(unsigned width, unsigned height, unsigned bpp, unsigned frequency)
		: width(width)
		, height(height)
		, bpp(bpp)
		, frequency(frequency)
	{
	}

	bool operator==(video_mode const& other) const
	{
		return width == other.width && height == other.height
			&& bpp == other.bpp && frequency == other.frequency;
	}

	bool operator<(video_mode const& other) const
	{
		return std::tie(bpp, width, height, frequency)
			< std::tie(other.bpp, other.width, other.height, other.frequency);
	}

	bool is_valid() const;
	bool is_current() const;
};

typedef std::set<video_mode> video_modes;

void init_supported_video_modes(video_modes& modes);

video_mode get_current_video_mode();

}} // aspect::gui

#endif // OXYGEN_VIDEO_MODES_HPP_INCLUDED
