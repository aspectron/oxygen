#include "core.hpp"
#include "v8_core.hpp"
#include "math.hpp"
#include "async_queue.hpp"
#include "events.hpp"
#include "runtime.hpp"
#include "v8_main_loop.hpp"

#include <iostream>
#include <vector>

#if OS(WINDOWS)
//	#pragma warning ( disable : 4251 )
#if defined(OXYGEN_EXPORTS)
#define OXYGEN_API __declspec(dllexport)
#else
#define OXYGEN_API __declspec(dllimport)
#endif
#elif __GNUC__ >= 4
# define OXYGEN_API __attribute__((visibility("default")))
#else
#define OXYGEN_API // nothing, symbols in a shared library are exported by default
#endif

#include "video_modes.hpp"
#include "gui.hpp"
#if OS(WINDOWS)
#include "gui.windows.hpp"
#elif OS(LINUX)
#include "gui.linux.hpp"
#endif
