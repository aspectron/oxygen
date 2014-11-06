#include "oxygen/oxygen.hpp"
#include "oxygen/display.hpp"

namespace aspect {  namespace gui {

XRRModeInfo const* mode_info(XRRScreenResources const* sr, RRMode id)
{
	XRRModeInfo const* result = std::find_if(sr->modes, sr->modes + sr->nmode,
			[id](XRRModeInfo const& info) { return info.id == id; });
	return (result == sr->modes + sr->nmode)? nullptr : result;
}

static display init(XRRScreenResources* sr, RROutput output)
{
	display result;

	XRROutputInfo* oi = XRRGetOutputInfo(g_display, sr, output);
	if (oi->connection == RR_Connected)
	{
		result.scale = 1;
		result.name = oi->name;
		result.color_depth = XDefaultDepth(g_display, g_screen);
		result.color_depth_per_component = result.color_depth >= 24? 8 : 0;

		result.crtc = oi->crtc;
		result.output = output;

		XRRCrtcInfo* ci = XRRGetCrtcInfo(g_display, sr, oi->crtc);
		result.rect.left = ci->x;
		result.rect.top = ci->y;
		result.rect.width = ci->width;
		result.rect.height = ci->height;
		if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
		{
			std::swap(result.rect.width, result.rect.height);
		}
		XRRFreeCrtcInfo(ci);

		result.work_rect = result.rect;
	}
	XRRFreeOutputInfo(oi);
	return result;
}

static display::mode make_mode(XRRModeInfo const* mi, XRRCrtcInfo const* ci)
{
	unsigned width = mi->width;
	unsigned height = mi->height;
	unsigned bpp = DefaultDepth(g_display, g_screen);
	unsigned frequency = 0;
	if (ci->rotation == RR_Rotate_90 || ci->rotation == RR_Rotate_270)
	{
		std::swap(width, height);
	}
	if (mi->hTotal && mi->vTotal)
	{
		double const s = mi->hTotal * mi->vTotal;
		frequency = static_cast<unsigned>(mi->dotClock / s);
	}
	return display::mode(width, height, bpp, frequency);
}

std::vector<display> display::enumerate()
{
	std::vector<display> result;

	if (randr.is_available)
	{
		RROutput const primary = XRRGetOutputPrimary(g_display, g_root);

		XRRScreenResources* sr = XRRGetScreenResources(g_display, g_root);
		for (int i = 0; i < sr->ncrtc; ++i)
		{
			XRRCrtcInfo* info = XRRGetCrtcInfo(g_display, sr, sr->crtcs[i]);

			if (info->noutput)
			{
				RROutput* output = std::find_if(info->outputs, info->outputs + info->noutput,
					[primary](RROutput output) { return output == primary; });
				if (output == info->outputs + info->noutput)
				{
					output = info->outputs;
				}

				display disp = init(sr, *output);
				if (!disp.name.empty())
				{
					result.emplace_back(disp);
				}
			}
			XRRFreeCrtcInfo(info);
		}
		XRRFreeScreenResources(sr);

		std::vector<display>::iterator it = std::find_if(result.begin(), result.end(),
			[primary](display const& disp) { return disp.output == primary; });
		if (it != result.begin() && it != result.end())
		{
			std::iter_swap(it, result.begin());
		}
	}
	else
	{
		result.push_back(primary());
	}

	return result;
}

display display::primary()
{
	return from_window(nullptr);
}

display display::from_window(window const* w)
{
	Window root = w? *w : g_root;
	display result;
	if (randr.is_available)
	{
		XRRScreenResources* sr = XRRGetScreenResources(g_display, root);
		RROutput primary = XRRGetOutputPrimary(g_display, g_root);
		result = init(sr, primary);
		XRRFreeScreenResources(sr);
	}
	else
	{
		result.scale = 1;
		result.color_depth = XDefaultDepth(g_display, g_screen);
		result.color_depth_per_component = result.color_depth >= 24? 8 : 0;
		result.rect = result.work_rect = rectangle<int>(0, 0,
			DisplayWidthMM(g_display, g_screen), DisplayHeightMM(g_display, g_screen));
	}
	return result;
}

std::vector<display::mode> display::modes() const
{
	std::vector<display::mode> result;
	if (randr.is_available)
	{
		XRRScreenResources* sr = XRRGetScreenResources(g_display, g_root);
		XRRCrtcInfo* ci = XRRGetCrtcInfo(g_display, sr, crtc);
		XRROutputInfo* oi = XRRGetOutputInfo(g_display, sr, output);

		for (int i = 0; i < oi->nmode; ++i)
		{
			XRRModeInfo const* mi = mode_info(sr, oi->modes[i]);
			if (mi && !(mi->modeFlags & RR_Interlace))
			{
				result.emplace_back(make_mode(mi, ci));
			}
		}

		XRRFreeOutputInfo(oi);
		XRRFreeCrtcInfo(ci);
		XRRFreeScreenResources(sr);

		std::sort(result.begin(), result.end());
		result.erase(std::unique(result.begin(), result.end()), result.end());
	}
	else
	{
		result.emplace_back(current_mode());
	}
	return result;
}

display::mode display::current_mode() const
{
	if (randr.is_available)
	{
		XRRScreenResources* sr = XRRGetScreenResources(g_display, g_root);
		XRRCrtcInfo* ci =  XRRGetCrtcInfo(g_display, sr, crtc);

		XRRModeInfo const* mi = mode_info(sr, ci->mode);
		display::mode const result = make_mode(mi, ci);

		XRRFreeCrtcInfo(ci);
		XRRFreeScreenResources(sr);

		return result;
	}
	else
	{
		return display::mode(DisplayWidth(g_display, g_screen), DisplayHeight(g_display, g_screen),
			DefaultDepth(g_display, g_screen), 0);
	}
}

}} // aspect::gui
