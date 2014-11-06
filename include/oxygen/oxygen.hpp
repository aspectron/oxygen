#include "jsx/core.hpp"
#include "jsx/v8_core.hpp"
#include "jsx/async_queue.hpp"
#include "jsx/events.hpp"
#include "jsx/runtime.hpp"
#include "jsx/v8_main_loop.hpp"
#include "jsx/geometry.hpp"

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

#include "oxygen/display.hpp"
#include "oxygen/gui.hpp"
#if OS(WINDOWS)
#include "oxygen/gui.windows.hpp"
#elif OS(DARWIN)
#include "oxygen/gui.mac.hpp"
#else
#include "oxygen/gui.x11.hpp"
#endif
