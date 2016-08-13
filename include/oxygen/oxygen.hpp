#if _MSC_VER
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
#if defined(_WIN32)
#include "oxygen/gui.windows.hpp"
#elif defined(__APPLE__)
#include "oxygen/gui.mac.hpp"
#else
#include "oxygen/gui.x11.hpp"
#endif
