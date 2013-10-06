#include "oxygen.hpp"

namespace aspect {  namespace gui {


creation_args::creation_args(v8::Arguments const& args)
{
	using namespace v8;

	HandleScope scope;

	Handle<Object> options = args[0]->ToObject();
	if (options.IsEmpty())
	{
		throw std::runtime_error("Window constructor requires configuration object as an argument");
	}

	if (get_option(options, "width", width = 640))
	{
		width = min(width, 1024*10u);
	}
	if (get_option(options, "height", height = 480))
	{
		height = min(height, 1024*10u);
	}

	if (!get_option(options, "left", left))
	{
		left = max(int(get_current_video_mode().width - width) / 2, 0);
	}
	if (!get_option(options, "top", top))
	{
		top = max(int(get_current_video_mode().height - height) / 2, 0);
	}

	get_option(options, "bpp", bpp = 32);
	get_option(options, "style", style = GWS_TITLEBAR | GWS_RESIZE | GWS_CLOSE | GWS_APPWINDOW);
	get_option(options, "caption", caption);
	get_option(options, "splash", splash);
}

#if OS(WINDOWS)
bool window_base::process_event_by_sink(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result)
{
	for (event_sinks::iterator it = event_sinks_.begin(), end =event_sinks_.end(); it != end; ++it)
	{
		if ( (*it)->process_events(message, wparam, lparam, result))
		{
			return true;
		}
	}
	return false;
}
#endif

}} // aspect::gui
