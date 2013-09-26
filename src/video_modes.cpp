#include "oxygen.hpp"
#include "video_modes.hpp"

#if OS(LINUX)
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#endif

namespace aspect {

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
	DEVMODE Win32Mode;
	Win32Mode.dmSize = sizeof(DEVMODE);
	for (int Count = 0; EnumDisplaySettings(NULL, Count, &Win32Mode); ++Count)
	{
		video_mode const mode(Win32Mode.dmPelsWidth, Win32Mode.dmPelsHeight, Win32Mode.dmBitsPerPel);
		modes.insert(mode);
	}
}

video_mode get_current_video_mode()
{
	DEVMODE Win32Mode;
	Win32Mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &Win32Mode);

	return video_mode(Win32Mode.dmPelsWidth, Win32Mode.dmPelsHeight, Win32Mode.dmBitsPerPel);
}

#endif

#if OS(LINUX)

void init_supported_video_modes(video_modes& modes)
{
    // First, clear array to fill
    modes.clear();

	_aspect_assert(aspect::gui::g_display);

	Display *Disp   = aspect::gui::g_display;
	int      Screen = aspect::gui::g_screen; 

    // Check if the XRandR extension is present
    int Version;
    if (XQueryExtension(Disp, "RANDR", &Version, &Version, &Version))
    {
        // Get the current configuration
        XRRScreenConfiguration* Config = XRRGetScreenInfo(Disp, RootWindow(Disp, Screen));
        if (Config)
        {
            // Get the available screen sizes
            int NbSizes;
            XRRScreenSize* Sizes = XRRConfigSizes(Config, &NbSizes);
            if (Sizes && (NbSizes > 0))
            {
                // Get the list of supported depths
                int NbDepths = 0;
                int* Depths = XListDepths(Disp, Screen, &NbDepths);
                if (Depths && (NbDepths > 0))
                {
                    // Combine depths and sizes to fill the array of supported modes
                    for (int i = 0; i < NbDepths; ++i)
                    {
                        for (int j = 0; j < NbSizes; ++j)
                        {
                            // Convert to video_mode
                            video_mode const mode(Sizes[j].width, Sizes[j].height, Depths[i]);
                            modes.insert(mode);
                        }
                    }
                }
            }

            // Free the configuration instance
            XRRFreeScreenConfigInfo(Config);
        }
        else
        {
            // Failed to get the screen configuration
            std::cerr << "Failed to get the list of available video modes" << std::endl;
        }
    }
    else
    {
        // XRandr extension is not supported : we cannot get the video modes
        std::cerr << "Failed to get the list of available video modes" << std::endl;
    }
}


////////////////////////////////////////////////////////////
/// Get current desktop video mode
////////////////////////////////////////////////////////////
video_mode get_current_video_mode()
{
    video_mode DesktopMode;

    // Get the display and screen from WindowImplUnix
//     WindowImplX11::OpenDisplay(false);
//     Display* Disp   = WindowImplX11::ourDisplay;
//     int      Screen = WindowImplX11::ourScreen;


	_aspect_assert(aspect::gui::g_display);

	Display* Disp   = aspect::gui::g_display;
	int      Screen = aspect::gui::g_screen; 


    // Check if the XRandR extension is present
    int Version;
    if (XQueryExtension(Disp, "RANDR", &Version, &Version, &Version))
    {
        // Get the current configuration
        XRRScreenConfiguration* Config = XRRGetScreenInfo(Disp, RootWindow(Disp, Screen));
        if (Config)
        {
            // Get the current video mode
            Rotation CurrentRotation;
            int CurrentMode = XRRConfigCurrentConfiguration(Config, &CurrentRotation);

            // Get the available screen sizes
            int NbSizes;
            XRRScreenSize* Sizes = XRRConfigSizes(Config, &NbSizes);
            if (Sizes && (NbSizes > 0))
                DesktopMode = video_mode(Sizes[CurrentMode].width, Sizes[CurrentMode].height, DefaultDepth(Disp, Screen));

            // Free the configuration instance
            XRRFreeScreenConfigInfo(Config);
        }
    }

    return DesktopMode;
}

#endif // OS(LINUX)

} // aspect
