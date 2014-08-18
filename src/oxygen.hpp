#include "core.hpp"
#include "v8_core.hpp"
#include "async_queue.hpp"
#include "events.hpp"
#include "runtime.hpp"
#include "v8_main_loop.hpp"

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

#include "geometry.hpp"
#include "display.hpp"
#include "gui.hpp"
#if OS(WINDOWS)
#include "gui.windows.hpp"
#elif OS(DARWIN)
#include "gui.mac.hpp"
#else
#include "gui.x11.hpp"
#endif
