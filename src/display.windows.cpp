#include "oxygen/oxygen.hpp"
#include "oxygen/display.hpp"

namespace aspect {  namespace gui {

static display init(HMONITOR monitor)
{
	display result;

	result.scale = 1;

	MONITORINFOEXW monitor_info;
	memset(&monitor_info, 0, sizeof(monitor_info));
	monitor_info.cbSize = sizeof(monitor_info);
	GetMonitorInfoW(monitor, &monitor_info);

	result.name = monitor_info.szDevice;

	HDC hdc = CreateDCW(L"DISPLAY", monitor_info.szDevice, NULL, NULL);
	result.color_depth = GetDeviceCaps(hdc, BITSPIXEL);
	result.color_depth_per_component = result.color_depth >= 24? 8 : 0;
	DeleteDC(hdc);

	result.rect.left = monitor_info.rcMonitor.left;
	result.rect.top = monitor_info.rcMonitor.top;
	result.rect.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
	result.rect.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;

	result.work_rect.left = monitor_info.rcWork.left;
	result.work_rect.top = monitor_info.rcWork.top;
	result.work_rect.width = monitor_info.rcWork.right - monitor_info.rcWork.left;
	result.work_rect.height = monitor_info.rcWork.bottom - monitor_info.rcWork.top;

	return result;
}

std::vector<display> display::enumerate()
{
	std::vector<display> result;

	DISPLAY_DEVICEW device = {};
	device.cb = sizeof (device);

	DEVMODEW devmode = {};
	devmode.dmSize = sizeof(devmode);

	size_t primary_index = 0;
	for (size_t i = 0; EnumDisplayDevicesW(nullptr, i, &device, 0); ++i)
	{
		if ((device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) ||
			!(device.StateFlags & DISPLAY_DEVICE_ACTIVE))
		{
			continue;
		}

		if (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
		{
			primary_index = i;
		}

		if (EnumDisplaySettingsExW(device.DeviceName, ENUM_CURRENT_SETTINGS, &devmode, EDS_ROTATEDMODE))
		{
			POINT pt;
			pt.x = devmode.dmPosition.x;
			pt.y = devmode.dmPosition.y;
			HMONITOR hmonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
			if (hmonitor)
			{
				result.push_back(init(hmonitor));
			}
		}
	}

	if (primary_index != 0)
	{
		// make primary display first
		std::swap(result[0], result[primary_index]);
		_aspect_assert(result[0] == primary());
	}

	return result;
}

display display::primary()
{
	// primary display has origin (0, 0)
	POINT const pt = { 0, 0 };
	HMONITOR hmonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
	return init(hmonitor);
}

display display::from_window(window const* w)
{
	HWND hwnd = w? static_cast<HWND>(*w) : GetActiveWindow();
	HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	return init(hmonitor);
}

std::vector<display::mode> display::modes() const
{
	std::vector<mode> result;

	DEVMODE devmode;
	devmode.dmSize = sizeof(devmode);
	for (int i = 0; EnumDisplaySettingsW(name.c_str(), i, &devmode); ++i)
	{
		result.emplace_back(mode(devmode.dmPelsWidth, devmode.dmPelsHeight,
			devmode.dmBitsPerPel, devmode.dmDisplayFrequency));
	}
	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());
	return result;
}

display::mode display::current_mode() const
{
	DEVMODE devmode;
	devmode.dmSize = sizeof(devmode);
	EnumDisplaySettingsW(name.c_str(), ENUM_CURRENT_SETTINGS, &devmode);
	
	return mode(devmode.dmPelsWidth, devmode.dmPelsHeight,
			devmode.dmBitsPerPel, devmode.dmDisplayFrequency);
}

}} // aspect::gui
