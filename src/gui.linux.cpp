#include "oxygen.hpp"
#include "gui.linux.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/glx.h>

using namespace v8;

namespace aspect { namespace gui {

Display* g_display = nullptr;
int g_screen = 0;
XIM g_input_method = nullptr;

static XContext window_context;
static boost::thread process_events_thread;
static bool is_running = false;

void window::init()
{
	XInitThreads();
	g_display = XOpenDisplay(nullptr);
	if (!g_display)
	{
		throw std::runtime_error("Failed to open a connection with the X server");
	}

	g_screen = DefaultScreen(g_display);
	g_input_method = XOpenIM(g_display, nullptr, nullptr, nullptr);

	is_running = true;
	process_events_thread = boost::thread(&window::process_events);
}

void window::cleanup()
{
	is_running = false;
	if (process_events_thread.joinable()) process_events_thread.join();

	if (g_input_method)
	{
		XCloseIM(g_input_method);
		g_input_method = nullptr;
	}

	g_screen = 0;

	if (g_display)
	{
		XCloseDisplay(g_display);
		g_display = nullptr;
	}
}

void window::process_events()
{
	while (is_running)
	{
		while (XPending(g_display) > 0)
		{
			XEvent event;
	
			XNextEvent(g_display, &event);

			XPointer window_ptr = nullptr;
			XFindContext(g_display, event.xany.window, window_context, &window_ptr);
			if (window_ptr)
			{
				reinterpret_cast<window*>(window_ptr)->process_event(event);
			}
		}
		boost::this_thread::yield();
	}
}

static unsigned long const ms_event_mask  =
	FocusChangeMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
	PointerMotionMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask |
	EnterWindowMask | LeaveWindowMask;

static unsigned score_config(creation_args const& args, graphics_settings const& settings,
	int color_bits, int depth_bits, int stencil_bits, int antialiasing_level)
{
	return
		abs(static_cast<int>(args.bpp  - color_bits)) +
		abs(static_cast<int>(settings.depth_bits - depth_bits)) +
		abs(static_cast<int>(settings.stencil_bits - stencil_bits)) +
		abs(static_cast<int>(settings.antialiasing_level - antialiasing_level));
}

static bool create_context(creation_args const& args, XVisualInfo& ChosenVisual, graphics_settings& settings,
	XVisualInfo Template = XVisualInfo(), unsigned long mask = 0)
{
	// Get all the visuals matching the template
	Template.screen = g_screen;
	int NbVisuals = 0;
	XVisualInfo* Visuals = XGetVisualInfo(g_display, mask | VisualScreenMask, &Template, &NbVisuals);
	if (!Visuals || (NbVisuals == 0))
	{
		if (Visuals)
			XFree(Visuals);
		std::cerr << "There is no valid visual for the selected screen" << std::endl;
		return false;
	}

	// Find the best visual
	int          best_score = 0xFFFF;
	XVisualInfo* best_visual = NULL;
	while (!best_visual)
	{
		for (int i = 0; i < NbVisuals; ++i)
		{
			// Get the current visual attributes
			int rgba, doublebuffer, red, green, blue, alpha, depth, stencil, multisampling, samples;
			glXGetConfig(g_display, &Visuals[i], GLX_RGBA,               &rgba);
			glXGetConfig(g_display, &Visuals[i], GLX_DOUBLEBUFFER,       &doublebuffer);
			glXGetConfig(g_display, &Visuals[i], GLX_RED_SIZE,           &red);
			glXGetConfig(g_display, &Visuals[i], GLX_GREEN_SIZE,         &green);
			glXGetConfig(g_display, &Visuals[i], GLX_BLUE_SIZE,          &blue);
			glXGetConfig(g_display, &Visuals[i], GLX_ALPHA_SIZE,         &alpha);
			glXGetConfig(g_display, &Visuals[i], GLX_DEPTH_SIZE,         &depth);
			glXGetConfig(g_display, &Visuals[i], GLX_STENCIL_SIZE,       &stencil);
			glXGetConfig(g_display, &Visuals[i], GLX_SAMPLE_BUFFERS_ARB, &multisampling);
			glXGetConfig(g_display, &Visuals[i], GLX_SAMPLES_ARB,        &samples);

			// First check the mandatory parameters
			if ((rgba == 0) || (doublebuffer == 0))
				continue;

			// Evaluate the current configuration
			int const color = red + green + blue + alpha;
			int const score = score_config(args, settings, color, depth, stencil, multisampling? samples : 0);

			// Keep it if it's better than the current best
			if (score < best_score)
			{
				best_score  = score;
				best_visual = &Visuals[i];
			}
		}

		// If no visual has been found, try a lower level of antialiasing
		if (!best_visual)
		{
			if (settings.antialiasing_level > 2)
			{
				std::cerr << "Failed to find a pixel format supporting "
					<< settings.antialiasing_level << " antialiasing levels ; trying with 2 levels" << std::endl;
				settings.antialiasing_level = 2;
			}
			else if (settings.antialiasing_level > 0)
			{
				std::cerr << "Failed to find a pixel format supporting antialiasing ; antialiasing will be disabled" << std::endl;
				settings.antialiasing_level = 0;
			}
			else
			{
				std::cerr << "Failed to find a suitable pixel format for the window -- cannot create OpenGL context" << std::endl;
				return false;
			}
		}
	}

#if 0
	// Create the OpenGL context
	myGLContext = glXCreateContext(g_display, best_visual, glXGetCurrentContext(), true);
	if (myGLContext == NULL)
	{
		std::cerr << "Failed to create an OpenGL context for this window" << std::endl;
		return false;
	}

#endif
	// Update the creation settings from the chosen format
	int depth, stencil;
	glXGetConfig(g_display, best_visual, GLX_DEPTH_SIZE,   &depth);
	glXGetConfig(g_display, best_visual, GLX_STENCIL_SIZE, &stencil);
	settings.depth_bits = static_cast<unsigned int>(depth);
	settings.stencil_bits = static_cast<unsigned int>(stencil);

	// Assign the chosen visual, and free the temporary visuals array
	ChosenVisual = *best_visual;
	// current_visual_ =*best_visual;
	XFree(Visuals);

#if 0
	// Activate the context
	SetActive(true);

	// Enable multisampling if needed
	if (Params.AntialiasingLevel > 0)
		glEnable(GL_MULTISAMPLE_ARB);
#endif

	return true;
}

/*
bool create_pbuffer(XVisualInfo *visual_info)
{
	//const int screen = DefaultScreen(g_display);
	int config_count = 0;
	GLXFBConfig *configs = glXGetFBConfigs(g_display, g_screen, &config_count);
	GLXFBConfig config = 0;

	for(int i = 0; i < config_count; i++)
	{
		int visual_id = 0;
		if(glXGetFBConfigAttrib( g_display, configs[i], GLX_VISUAL_ID, &visual_id) == 0)
		{
			if(visual_id == static_cast<int>(visual_info->visualid))
			{
				config = configs[i];
				break;
			}
		}
	}

	if(!config)
	{
		error("creating pbuffer","unable to match visual id with compatible FBConfig");
		return false;
	}

"""""""""""""""""""""""""""""""""""""
	// Create PBuffer
	const PixelViewport& pvp = _window->getPixelViewport();
	const int attributes[] = { GLX_PBUFFER_WIDTH, pvp.w,
								GLX_PBUFFER_HEIGHT, pvp.h,
								GLX_LARGEST_PBUFFER, True,
								GLX_PRESERVED_CONTENTS, True,
								0 };

	XID pbuffer = glXCreatePbuffer( display, config, attributes );
	if ( !pbuffer )
	{
		_window->setErrorMessage( "Could not create PBuffer" );
		return false;
	}   
				   
	XFlush( display );
	setXDrawable( pbuffer );

}
*/

window::window(v8::Arguments const& v8_args)
	: window_(0)
	, atom_close_(0)
	, previous_video_mode_(-1)
	, hidden_cursor_(0)
	, input_context_(nullptr)
{
	creation_args const args(v8_args);
	create(args);
}

void window::create(creation_args const& args)
{
	style_ = args.style;
	if (style_ & GWS_APPWINDOW)
		style_ |= GWS_TITLEBAR | GWS_RESIZE | GWS_CLOSE;

	bool const fullscreen = (style_ & GWS_FULLSCREEN) != 0;

	// Compute position and size
	int const width = width_ = args.width;
	int const height = height_ = args.height;

	int const left = fullscreen? 0 : args.left;
	int const top = fullscreen? 0 : args.top;

	// Switch to fullscreen if necessary
	if (fullscreen)
	{
		switch_to_fullscreen(video_mode(args.width, args.height, args.bpp));
	}

	// Create the rendering context
	gui::graphics_settings settings;
	if (!create_context(args, current_visual_, settings))
	{
		return;
	}

	// Create a new color map with the chosen visual
	Colormap ColMap = XCreateColormap(g_display, RootWindow(g_display, g_screen), current_visual_.visual, AllocNone);

	// Define the window attributes
	XSetWindowAttributes Attributes;
	Attributes.event_mask        = ms_event_mask;
	Attributes.colormap          = ColMap;
	Attributes.override_redirect = fullscreen;

	// Create the window
	window_ = XCreateWindow(g_display,
		RootWindow(g_display, g_screen),
		left, top,
		width, height,
		0,
		current_visual_.depth,
		InputOutput,
		current_visual_.visual,
		CWEventMask | CWColormap | CWOverrideRedirect, &Attributes);
	if (!window_)
	{
		throw std::runtime_error("Failed to create window");
	}
	XSaveContext(g_display, window_, window_context, reinterpret_cast<XPointer>(this));


	// Set the window's name
	XStoreName(g_display, window_, args.caption.c_str());

	// Set the window's style (tell the windows manager to change our window's decorations and functions according to the requested style)
	if (!fullscreen)
	{
		Atom WMHintsAtom = XInternAtom(g_display, "_MOTIF_WM_HINTS", false);
		if (WMHintsAtom)
		{
			static const unsigned long MWM_HINTS_FUNCTIONS   = 1 << 0;
			static const unsigned long MWM_HINTS_DECORATIONS = 1 << 1;

			//static const unsigned long MWM_DECOR_ALL         = 1 << 0;
			static const unsigned long MWM_DECOR_BORDER      = 1 << 1;
			static const unsigned long MWM_DECOR_RESIZEH     = 1 << 2;
			static const unsigned long MWM_DECOR_TITLE       = 1 << 3;
			static const unsigned long MWM_DECOR_MENU        = 1 << 4;
			static const unsigned long MWM_DECOR_MINIMIZE    = 1 << 5;
			static const unsigned long MWM_DECOR_MAXIMIZE    = 1 << 6;

			//static const unsigned long MWM_FUNC_ALL          = 1 << 0;
			static const unsigned long MWM_FUNC_RESIZE       = 1 << 1;
			static const unsigned long MWM_FUNC_MOVE         = 1 << 2;
			static const unsigned long MWM_FUNC_MINIMIZE     = 1 << 3;
			static const unsigned long MWM_FUNC_MAXIMIZE     = 1 << 4;
			static const unsigned long MWM_FUNC_CLOSE        = 1 << 5;

			struct WMHints
			{
				unsigned long Flags;
				unsigned long Functions;
				unsigned long Decorations;
				long          InputMode;
				unsigned long State;
			};

			WMHints Hints;
			Hints.Flags       = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
			Hints.Decorations = 0;
			Hints.Functions   = 0;


			if (style_ & GWS_TITLEBAR)
			{
				Hints.Decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MINIMIZE | MWM_DECOR_MENU;
				Hints.Functions   |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
			}
			if (style_ & GWS_RESIZE)
			{
				Hints.Decorations |= MWM_DECOR_MAXIMIZE | MWM_DECOR_RESIZEH;
				Hints.Functions   |= MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;
			}

			if (style_ & GWS_CLOSE)
			{
				Hints.Decorations |= 0;
				Hints.Functions   |= MWM_FUNC_CLOSE;
			}

			const unsigned char* HintsPtr = reinterpret_cast<const unsigned char*>(&Hints);
			XChangeProperty(g_display, window_, WMHintsAtom, WMHintsAtom, 32, PropModeReplace, HintsPtr, 5);
		}

		// This is a hack to force some windows managers to disable resizing
#if 0 // TODO!
		if (!(style_ & GWS_RESIZE))
		{
			XSizeHints XSizeHints;
			XSizeHints.flags      = PMinSize | PMaxSize;
			XSizeHints.min_width  = XSizeHints.max_width  = width;
			XSizeHints.min_height = XSizeHints.max_height = height;
			XSetWMNormalHints(g_display, window_, &XSizeHints); 
		}
#endif
	}

	// Do some common initializations
	_init();

	// In fullscreen mode, we must grab keyboard and mouse inputs
	if (fullscreen)
	{
		XGrabPointer(g_display, window_, true, 0, GrabModeAsync, GrabModeAsync, window_, None, CurrentTime);
		XGrabKeyboard(g_display, window_, true, GrabModeAsync, GrabModeAsync, CurrentTime);
	}
}

void window::_init()
{
	// Make sure the "last key release" is initialized with invalid values
//	myLastKeyReleaseEvent.type = -1;

	// Get the atom defining the close event
	atom_close_ = XInternAtom(g_display, "WM_DELETE_WINDOW", false);
	XSetWMProtocols(g_display, window_, &atom_close_, 1);

	// Create the input context
	if (g_input_method)
	{
		input_context_ = XCreateIC(g_input_method,
			XNClientWindow,  window_,
			XNFocusWindow,   window_,
			XNInputStyle,    XIMPreeditNothing  | XIMStatusNothing,
			nullptr);

		if (!input_context_)
			std::cerr << "Failed to create input context for window -- TextEntered event won't be able to return unicode" << std::endl;
	}

	// Show the window
	XMapWindow(g_display, window_);
	XFlush(g_display);

	// Create the hiden cursor
	create_hidden_cursor();

	// Set our context as the current OpenGL context for rendering
//	SetActive();

	// Flush the commands queue
	XFlush(g_display);
}

void window::create_hidden_cursor()
{
	// Create the cursor's pixmap (1x1 pixels)
	Pixmap CursorPixmap = XCreatePixmap(g_display, window_, 1, 1, 1);
	GC GraphicsContext = XCreateGC(g_display, CursorPixmap, 0, nullptr);
	XDrawPoint(g_display, CursorPixmap, GraphicsContext, 0, 0);
	XFreeGC(g_display, GraphicsContext);

	// Create the cursor, using the pixmap as both the shape and the mask of the cursor
	XColor Color;
	Color.flags = DoRed | DoGreen | DoBlue;
	Color.red = Color.blue = Color.green = 0;
	hidden_cursor_ = XCreatePixmapCursor(g_display, CursorPixmap, CursorPixmap, &Color, &Color, 0, 0);

	// We don't need the pixmap any longer, free it
	XFreePixmap(g_display, CursorPixmap);
}

bool window::switch_to_fullscreen(video_mode const& mode)
{
	bool result = false;

	// Check if the XRandR extension is present
	int Version;
	if (XQueryExtension(g_display, "RANDR", &Version, &Version, &Version))
	{
		// Get the current configuration
		XRRScreenConfiguration* Config = XRRGetScreenInfo(g_display, RootWindow(g_display, g_screen));
		if (Config)
		{
			// Get the available screen sizes
			int NbSizes;
			XRRScreenSize* Sizes = XRRConfigSizes(Config, &NbSizes);
			if (Sizes && NbSizes > 0)
			{
				// Search a matching size
				for (int i = 0; i < NbSizes; ++i)
				{
					if (Sizes[i].width == mode.width && Sizes[i].height == mode.height)
					{
						// Get the current rotation
						Rotation current_rotation;
						previous_video_mode_ = XRRConfigCurrentConfiguration(Config, &current_rotation);

						// Switch to fullscreen mode
						XRRSetScreenConfig(g_display, Config, RootWindow(g_display, g_screen), i, current_rotation, CurrentTime);
						result = true;
						break;
					}
				}
			}

			// Free the configuration instance
			XRRFreeScreenConfigInfo(Config);
		}
		else
		{
			// Failed to get the screen configuration
			std::cerr << "Failed to get the current screen configuration for fullscreen mode, switching to window mode" << std::endl;
		}
	}
	else
	{
		// XRandr extension is not supported : we cannot use fullscreen mode
		std::cerr << "Fullscreen is not supported, switching to window mode" << std::endl;
	}

	return result;
}

void window::destroy()
{
	if (!window_)
	{
		return;
	}

	// Cleanup graphical resources
	_cleanup();
	XDeleteContext(g_display, window_, window_context);	

	// Destroy the input context
	if (input_context_)
	{
		XDestroyIC(input_context_);
		input_context_ = nullptr;
	}

	// Destroy the window
	XDestroyWindow(g_display, window_);
	XFlush(g_display);
	window_ = 0;

	if (style_ & GWS_APPWINDOW)
	{
		is_running = false;
	}
}

void window::_cleanup()
{
	// Restore the previous video mode (in case we were running in fullscreen)
	if (previous_video_mode_ >= 0)
	{
		// Get current screen info
		XRRScreenConfiguration* Config = XRRGetScreenInfo(g_display, RootWindow(g_display, g_screen));
		if (Config)
		{
			// Get the current rotation
			Rotation current_rotation;
			XRRConfigCurrentConfiguration(Config, &current_rotation);

			// Reset the video mode
			XRRSetScreenConfig(g_display, Config, RootWindow(g_display, g_screen), previous_video_mode_, current_rotation, CurrentTime);

			// Free the configuration instance
			XRRFreeScreenConfigInfo(Config);
		} 

		// Reset the fullscreen window
		previous_video_mode_ = -1;
	}

	// Unhide the mouse cursor (in case it was hidden)
	show_mouse_cursor(true);

	// Destroy the OpenGL context
// 					if (myGLContext)
// 					{
// 						glXDestroyContext(g_display, myGLContext);
// 						myGLContext = NULL;
// 					}
}

void window::show_mouse_cursor(bool show)
{
	XDefineCursor(g_display, window_, show? None : hidden_cursor_);
	XFlush(g_display);
}

void window::show(bool visible)
{
	visible? XMapWindow(g_display, window_) : XUnmapWindow(g_display, window_);
	XFlush(g_display);
}

void window::process_event(XEvent const& event)
{
	switch (event.type)
	{
	case DestroyNotify:
		// The window is about to be destroyed : we must cleanup resources
		_cleanup();
		break;

	case FocusIn:
		// Update the input context
		if (input_context_)
		{
			XSetICFocus(input_context_);
		}
		break;

	case FocusOut:
		// Update the input context
		if (input_context_)
		{
			XUnsetICFocus(input_context_);
		}
		break;

	// Resize event
	case ConfigureNotify:
		if (event.xconfigure.width != width_ || event.xconfigure.height != height_)
		{
			width_  = event.xconfigure.width;
			height_ = event.xconfigure.height;
			on_resize(width_, height_);
		}
		break;

	// Close event
	case ClientMessage:
		if (event.xclient.format == 32 && event.xclient.data.l[0] == atom_close_)
		{
			destroy();
			on_event("close");
		}
		break;
/*
	// Key down event
	case KeyPress:
		{
			// Get the keysym of the key that has been pressed
			static XComposeStatus KeyboardStatus;
			char Buffer[32];
			KeySym Sym;
			XLookupString(&WinEvent.xkey, Buffer, sizeof(Buffer), &Sym, &KeyboardStatus);

			// Fill the event parameters
			Event Evt;
			Evt.Type        = Event::KeyPressed;
			Evt.Key.Code    = KeysymToSF(Sym);
			Evt.Key.Alt     = WinEvent.xkey.state & Mod1Mask;
			Evt.Key.Control = WinEvent.xkey.state & ControlMask;
			Evt.Key.Shift   = WinEvent.xkey.state & ShiftMask;
			SendEvent(Evt);

			// Generate a TextEntered event
			if (!XFilterEvent(&WinEvent, None))
			{
#ifdef X_HAVE_UTF8_STRING
				if (input_context_)
				{
					Status ReturnedStatus;
					Uint8  KeyBuffer[16];
					int Length = Xutf8LookupString(input_context_, &WinEvent.xkey, reinterpret_cast<char*>(KeyBuffer), sizeof(KeyBuffer), NULL, &ReturnedStatus);
					if (Length > 0)
					{
						Uint32 Unicode[2]; // just in case, but 1 character should be enough
						const Uint32* End = Unicode::UTF8ToUTF32(KeyBuffer, KeyBuffer + Length, Unicode);

						if (End > Unicode)
						{
							Event TextEvt;
							TextEvt.Type         = Event::TextEntered;
							TextEvt.Text.Unicode = Unicode[0];
							SendEvent(TextEvt);
						}
					}
				}
				else
#endif
				{
					static XComposeStatus ComposeStatus;
					char KeyBuffer[16];
					if (XLookupString(&WinEvent.xkey, KeyBuffer, sizeof(KeyBuffer), NULL, &ComposeStatus))
					{
						Event TextEvt;
						TextEvt.Type         = Event::TextEntered;
						TextEvt.Text.Unicode = static_cast<Uint32>(KeyBuffer[0]);
						SendEvent(TextEvt);
					}
				}
			}
		}
		break;

	// Key up event
	case KeyRelease:
		{
			// Get the keysym of the key that has been pressed
			char Buffer[32];
			KeySym Sym;
			XLookupString(&WinEvent.xkey, Buffer, 32, &Sym, NULL);

			// Fill the event parameters
			Event Evt;
			Evt.Type        = Event::KeyReleased;
			Evt.Key.Code    = KeysymToSF(Sym);
			Evt.Key.Alt     = WinEvent.xkey.state & Mod1Mask;
			Evt.Key.Control = WinEvent.xkey.state & ControlMask;
			Evt.Key.Shift   = WinEvent.xkey.state & ShiftMask;

			SendEvent(Evt);
		}
		break;

	// Mouse button pressed
	case ButtonPress:
		{
			unsigned int Button = WinEvent.xbutton.button;
			if ((Button == Button1) || (Button == Button2) || (Button == Button3) || (Button == 8) || (Button == 9))
			{
				Event Evt;
				Evt.Type          = Event::MouseButtonPressed;
				Evt.MouseButton.X = WinEvent.xbutton.x;
				Evt.MouseButton.Y = WinEvent.xbutton.y;
				switch (Button)
				{
				case Button1 : Evt.MouseButton.Button = Mouse::Left;     break;
				case Button2 : Evt.MouseButton.Button = Mouse::Middle;   break;
				case Button3 : Evt.MouseButton.Button = Mouse::Right;    break;
				case 8 :       Evt.MouseButton.Button = Mouse::XButton1; break;
				case 9 :       Evt.MouseButton.Button = Mouse::XButton2; break;
				}
				SendEvent(Evt);
			}
		}
		break;

	// Mouse button released
	case ButtonRelease:
		{
			unsigned int Button = WinEvent.xbutton.button;
			if ((Button == Button1) || (Button == Button2) || (Button == Button3) || (Button == 8) || (Button == 9))
			{
				Event Evt;
				Evt.Type          = Event::MouseButtonReleased;
				Evt.MouseButton.X = WinEvent.xbutton.x;
				Evt.MouseButton.Y = WinEvent.xbutton.y;
				switch (Button)
				{
				case Button1 : Evt.MouseButton.Button = Mouse::Left;     break;
				case Button2 : Evt.MouseButton.Button = Mouse::Middle;   break;
				case Button3 : Evt.MouseButton.Button = Mouse::Right;    break;
				case 8 :       Evt.MouseButton.Button = Mouse::XButton1; break;
				case 9 :       Evt.MouseButton.Button = Mouse::XButton2; break;            
				}
				SendEvent(Evt);
			}
			else if ((Button == Button4) || (Button == Button5))
			{
				Event Evt;
				Evt.Type             = Event::MouseWheelMoved;
				Evt.MouseWheel.Delta = WinEvent.xbutton.button == Button4 ? 1 : -1;
				SendEvent(Evt);
			}
		}
		break;

	// Mouse moved
	case MotionNotify:
		{
			Event Evt;
			Evt.Type        = Event::MouseMoved;
			Evt.MouseMove.X = WinEvent.xmotion.x;
			Evt.MouseMove.Y = WinEvent.xmotion.y;
			SendEvent(Evt);
		}
		break;

	// Mouse entered
	case EnterNotify:
		{
			Event Evt;
			Evt.Type = Event::MouseEntered;
			SendEvent(Evt);
		}
		break;

	// Mouse left
	case LeaveNotify:
		{
			Event Evt;
			Evt.Type = Event::MouseLeft;
			SendEvent(Evt);
		}
		break;
*/
	}
}

}} // aspect::gui
