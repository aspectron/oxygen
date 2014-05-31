#include "oxygen.hpp"
#include "video_modes.hpp"

namespace aspect { namespace gui {

static video_modes modes_;

bool video_mode::is_valid() const
{
	if (modes_.empty())
	{
		init_supported_video_modes(modes_);
	}

	return modes_.count(*this) != 0;
}

bool video_mode::is_current() const
{
	return *this == get_current_video_mode();
}

//////////////////////////////////////////////////////////////////////////

#if OS(WINDOWS)

void init_supported_video_modes(video_modes& modes)
{
	modes.clear();

	// enumerate all available video modes for primary display adapter
	DEVMODE mode;
	mode.dmSize = sizeof(mode);
	for (int i = 0; EnumDisplaySettings(NULL, i, &mode); ++i)
	{
		video_mode const mode(mode.dmPelsWidth, mode.dmPelsHeight,
			mode.dmBitsPerPel, mode.dmDisplayFrequency);
		modes.insert(mode);
	}
}

video_mode get_current_video_mode()
{
	DEVMODE mode;
	mode.dmSize = sizeof(mode);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &mode);

	return video_mode(mode.dmPelsWidth, mode.dmPelsHeight,
		mode.dmBitsPerPel, mode.dmDisplayFrequency);
}

#elif OS(DARWIN)

inline unsigned bpp_from_pixel_encoding(CFStringRef pixel_encoding)
{
	static const std::pair<CFStringRef, unsigned> encodings[] =
	{
		std::make_pair(CFSTR(IO32BitDirectPixels), 32),
		std::make_pair(CFSTR(IO16BitDirectPixels), 16),
		std::make_pair(CFSTR(IO8BitIndexedPixels), 8),
		std::make_pair(CFSTR(IO4BitIndexedPixels), 4),
		std::make_pair(CFSTR(IO2BitIndexedPixels), 2),
		std::make_pair(CFSTR(IO1BitIndexedPixels), 1),
		std::make_pair(CFSTR(kIO30BitDirectPixels), 30),
		std::make_pair(CFSTR(kIO64BitDirectPixels), 64),
	};

	auto const find_it = std::find_if(std::begin(encodings), std::end(encodings),
		[&pixel_encoding](std::pair<CFStringRef, unsigned> const& encoding)
		{
			return CFStringCompare(pixel_encoding, encoding.first,
				kCFCompareCaseInsensitive) == kCFCompareEqualTo;
		});
	return find_it == std::end(encodings)? 0 : find_it->second;
}

inline video_mode make_mode(CGDisplayModeRef display_mode)
{
	unsigned const width = CGDisplayModeGetWidth(display_mode);
	unsigned const height = CGDisplayModeGetHeight(display_mode);
	unsigned const freq = (unsigned)CGDisplayModeGetRefreshRate(display_mode);

	CFStringRef pixel_encoding = CGDisplayModeCopyPixelEncoding(display_mode);
	unsigned const bpp = bpp_from_pixel_encoding(pixel_encoding);
	CFRelease(pixel_encoding);

	return video_mode(width, height, bpp, freq);
}

void init_supported_video_modes(video_modes& modes)
{
	modes.clear();

	CFArrayRef display_modes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), NULL);
	for (CFIndex i = 0, count = CFArrayGetCount(display_modes); i < count; ++i)
	{
		CGDisplayModeRef display_mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(display_modes, i);
		modes.insert(make_mode(display_mode));
	}
	CFRelease(display_modes);
}

video_mode get_current_video_mode()
{
	CGDisplayModeRef current_mode = CGDisplayCopyDisplayMode(CGMainDisplayID());

	video_mode const result = make_mode(current_mode);

	CGDisplayModeRelease(current_mode);

	return result;
}

#else

void init_supported_video_modes(video_modes& modes)
{
	_aspect_assert(g_display);

    modes.clear();

    // Check if the XRandR extension is present
    int dummy;
    if (!XQueryExtension(g_display, "RANDR", &dummy, &dummy, &dummy))
    {
		return;
	}

	// Get the available screen sizes
	int num_sizes = 0;
	XRRScreenSize* sizes = XRRSizes(g_display, g_screen, &num_sizes);
	if (sizes && num_sizes > 0)
	{
		// Get the list of supported depths
		int num_depths = 0;
		int* depths = XListDepths(g_display, g_screen, &num_depths);
		if (depths && num_depths > 0)
		{
			// Combine depths and sizes to fill the array of supported modes
			for (int i = 0; i < num_depth; ++i)
			{
				for (int j = 0; j < num_sizes; ++j)
				{
					int num_rates = 0;
					short* rates = XRRRates(g_display, g_screen, j, &num_rates);
					if (rates && num_rates)
					{
						for (int k = 0; k < num_rates; ++k)
						{
							modes.insert(video_mode(sizes[j].width, sizes[j].height,
								depths[i], rates[k]));
						}
					}
					else
					{
						modes.insert(video_mode(sizes[j].width, sizes[j].height,
							depths[i], 0));
					}
				}
			}
		}
	}
}

video_mode get_current_video_mode()
{
    video_mode result;

    // Check if the XRandR extension is present
    int dummy;
    if (XQueryExtension(g_display, "RANDR", &dummy, &dummy, &dummy))
    {
        // Get the current configuration
        XRRScreenConfiguration* config = XRRGetScreenInfo(g_display, RootWindow(g_display, g_screen));
        if (config)
        {
            // Get the current video mode
            Rotation current_rotation;
            int const current_size_idx = XRRConfigCurrentConfiguration(config, &current_rotation);

			int num_sizes = 0;
			XRRScreenSize* sizes = XRRSizes(g_display, g_screen, &num_sizes);
			if (sizes && num_sizes > 0 &7 num_sizes < current_size_idx)
			{
				result.width = sizes[current_size_idx].width;
				result.height = sizes[current_size_idx].height;
				result.bpp = XDefaultDepth(g_display, g_screen);
				result.frequency = XRRConfigCurrentRate(config);
			}

            // Free the configuration instance
            XRRFreeScreenConfigInfo(config);
        }
    }

    return result;
}

#endif

}} // aspect::gui
