#include "oxygen.hpp"

//#include "video_modes.hpp"
//#include "gui.window.hpp"
//#include "gui.xf86.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
// #include <X11/extensions/Xrandr.h>

using namespace v8;
using namespace v8::juice;

namespace aspect
{
namespace gui 
{

Display		*g_display = NULL;
int			g_screen = 0;
XIM			g_input_method = NULL;
unsigned long  window::ms_event_mask  = FocusChangeMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
										PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
										EnterWindowMask | LeaveWindowMask;
std::vector<boost::shared_ptr<window>> window::window_list_;

void init(void)
{
	g_display = XOpenDisplay(NULL);
	if(g_display)
	{
		g_screen = DefaultScreen(g_display);

		// Get the input method (XIM) object
		g_input_method = XOpenIM(g_display, NULL, NULL, NULL);
	}
	else
	{
		std::cerr << "Failed to open a connection with the X server" << std::endl;
	}

	oxygen_thread::start();
}

void cleanup(void)
{
	oxygen_thread::stop();

	if (g_input_method)
		XCloseIM(g_input_method);

	XCloseDisplay(g_display);
	g_display = NULL;

}

Bool check_event(::Display*, XEvent* event, XPointer user_data)
{
	// Just check if the event matches the window
	return event->xany.window == reinterpret_cast< ::Window >(user_data);
}


v8::Handle<v8::Value> window::on(std::string const& name, v8::Handle<v8::Value> fn)
{
	window_base::on(name,fn);
	return convert::CastToJS(this);
}

v8::Handle<v8::Value> window::off(std::string const& name)
{
	window_base::off(name);
	return convert::CastToJS(this);
}


} } // aspect::gui
