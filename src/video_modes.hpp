#ifndef __VIDEO_MODES_HPP__
#define __VIDEO_MODES_HPP__

#include <set>
#include <tuple>
#include <boost/operators.hpp>

namespace aspect
{

struct OXYGEN_API video_mode : boost::totally_ordered<video_mode>
{
public:

	unsigned width;
	unsigned height;
	unsigned bpp;

	video_mode()
		: width(0)
		, height(0)
		, bpp(0)
	{
	}

	video_mode(unsigned width, unsigned height, unsigned bpp)
		: width(width)
		, height(height)
		, bpp(bpp)
	{
	}

	bool operator==(video_mode const& other) const
	{
		return width == other.width && height == other.height && bpp == other.bpp;
	}

	bool operator<(video_mode const& other) const
	{
		return std::tie(bpp, width, height) < std::tie(other.bpp, other.width, other.height);
	}

	bool is_valid() const;
	bool is_current() const;
};

typedef std::set<video_mode> video_modes;

void init_supported_video_modes(video_modes& modes);

video_mode get_current_video_mode();

}

#endif // __VIDEO_MODES_HPP__
