#include "oxygen/oxygen.hpp"
#include "oxygen/display.hpp"

#include <IOKit/graphics/IOGraphicsLib.h>

namespace aspect {  namespace gui {

static unsigned bpp_from_pixel_encoding(CFStringRef pixel_encoding)
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

static std::string display_name(CGDirectDisplayID id)
{
	std::string result = "(unknown)";

	CFDictionaryRef info = IODisplayCreateInfoDictionary(CGDisplayIOServicePort(id), kIODisplayOnlyPreferredName);
 	CFDictionaryRef names = (CFDictionaryRef)CFDictionaryGetValue(info, CFSTR(kDisplayProductName));

	CFStringRef value;
	if (names && CFDictionaryGetValueIfPresent(names, CFSTR("en_US"), (const void**) &value))
	{
		CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(value),kCFStringEncodingUTF8) + 1;
		result.resize(size);
		CFStringGetCString(value, &result[0], size, kCFStringEncodingUTF8);
	}

	CFRelease(info);

	return result.c_str();
}

static rectangle<int> to_rect(NSRect const& target, NSRect const& frame)
{
	return rectangle<int>(target.origin.x, NSMaxY(frame) - NSMaxY(target),
		target.size.width, target.size.height);
}

static display init(NSScreen* screen, NSWindow* window = nullptr)
{
	display result;

	NSDictionary* descr = [screen deviceDescription];
	result.id = [[descr objectForKey:@"NSScreenNumber"] unsignedIntegerValue];


	result.name = display_name(result.id);

	if (NSAppKitVersionNumber < NSAppKitVersionNumber10_7)
	{
		result.scale = 1;
	}
	else
	{
		result.scale = window? [window backingScaleFactor] : [screen backingScaleFactor];
	}

	NSWindowDepth const screen_depth = [screen depth];
	result.color_depth = NSBitsPerPixelFromDepth(screen_depth);
	result.color_depth_per_component = NSBitsPerSampleFromDepth(screen_depth);

	NSRect const screen_frame = [screen frame];
	result.rect = to_rect(screen_frame, screen_frame);
	result.work_rect = to_rect([screen visibleFrame], screen_frame);
	return result;
}

static display::mode make_mode(CGDisplayModeRef display_mode)
{
	unsigned const width = CGDisplayModeGetWidth(display_mode);
	unsigned const height = CGDisplayModeGetHeight(display_mode);
	unsigned const freq = (unsigned)CGDisplayModeGetRefreshRate(display_mode);

	CFStringRef pixel_encoding = CGDisplayModeCopyPixelEncoding(display_mode);
	unsigned const bpp = bpp_from_pixel_encoding(pixel_encoding);
	CFRelease(pixel_encoding);

	return display::mode(width, height, bpp, freq);
}

std::vector<display> display::enumerate()
{
	std::vector<display> result;
/*
	uint32_t count = 0;
	CGGetActiveDisplayList(0, nullptr, &count);
	std::vector<CGDirectDisplayID> ids(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		CGDirectDisplayID id = ids[i];
		if (CGDisplayIsActive(id))
		{
			result.emplace_back(init(id));
		}
	}
*/
	NSArray* screens = [NSScreen screens];
	for (unsigned i = 0, count = [screens count]; i < count; ++i)
	{
		NSScreen* screen = [screens objectAtIndex:i];
		result.emplace_back(init(screen));

	}
	return result;
}

display display::primary()
{
	return init([NSScreen screens][0]);
}

display display::from_window(window const* w)
{
	NSWindow* window = w? w->object : nullptr;
	return init(window? [window screen] : [NSScreen mainScreen], window);
}

std::vector<display::mode> display::modes() const
{
	std::vector<display::mode> result;

	CFArrayRef display_modes = CGDisplayCopyAllDisplayModes(id, nullptr);
	for (CFIndex i = 0, count = CFArrayGetCount(display_modes); i < count; ++i)
	{
		CGDisplayModeRef display_mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(display_modes, i);
		result.emplace_back(make_mode(display_mode));
	}
	CFRelease(display_modes);

	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());
	return result;
}

display::mode display::current_mode() const
{
	CGDisplayModeRef current_mode = CGDisplayCopyDisplayMode(id);
	display :mode const result = make_mode(current_mode);

	CGDisplayModeRelease(current_mode);

	return result;
}

}} // aspect::gui
