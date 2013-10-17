#include "oxygen.hpp"

#if OS(WINDOWS)

#include "shellapi.h"

using namespace v8;

namespace aspect { namespace gui {

static wchar_t const WINDOW_CLASS_NAME[] = L"jsx_generic";

static boost::thread window_thread_;

void post_thread_message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	DWORD const thread_id = ::GetThreadId(window_thread_.native_handle());
	::PostThreadMessage(thread_id, msg, wParam, lParam);
}

void window::init()
{
	WNDCLASSW wc = {}; // window's class struct

	// we must specify the attribs for the window's class
	wc.style			= CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;	// style bits (CS_OWNDC very important for OGL)
	wc.lpfnWndProc		= window::window_proc;					// window procedure to use
	wc.cbClsExtra		= 0;								// no extra data
	wc.cbWndExtra		= 0;								// no extra data
	wc.hInstance		= (HINSTANCE)GetModuleHandle(NULL);						// associated instance
	wc.hIcon			= NULL;								// use default icon temporarily
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);		// use default cursor (Windows owns it, so don't unload)
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);// background brush (don't destroy, let windows handle it)
	wc.lpszClassName	= WINDOW_CLASS_NAME;					// class name

	// this will create or not create a menu for the main window (depends on app settings)
	wc.lpszMenuName = NULL; // (APP_ALLOW_MENU) ? MAKEINTRESOURCE(IDR_MAINFRAME) : NULL;

	// now, we can register this class
	RegisterClassW(&wc);

	window_thread_ = boost::thread(&window::message_loop);
}

void window::cleanup()
{
	post_thread_message(WM_USER, 0, 0);
	window_thread_.join();
	
	UnregisterClassW(WINDOW_CLASS_NAME, NULL);
}

void window::message_loop()
{
	MSG msg;

	// to create a message queue in this thread
	::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while (::GetMessage(&msg, NULL, 0, 0) >= 0)
	{
		if (!msg.hwnd && msg.message == WM_USER)
		{
			// msg for the thread
			if (msg.wParam && msg.lParam)
			{
				window* w = reinterpret_cast<window*>(msg.wParam);
				creation_args const* args = reinterpret_cast<creation_args const*>(msg.lParam);

				w->create(*args);
			}
			else
			{
				// stop the thread
				return;
			}
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT CALLBACK window::window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// Associate handle and Window instance when the creation message is received
	if (message == WM_NCCREATE)
	{
		window* wnd = (window*)((CREATESTRUCTW*)lparam)->lpCreateParams;
		wnd->hwnd_ = hwnd;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wnd);
	}

	LRESULT result = 0;
	bool proceed = false;
	if (window* wnd = (window*)GetWindowLongPtr(hwnd, GWLP_USERDATA))
	{
		if (wnd->hwnd_ == hwnd)
		{
			proceed = wnd->process_event(message, wparam, lparam, result);
		}
	}

	/*
	// don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
	if (message == WM_CLOSE)
		return 0;
	*/

	return proceed? result : ::DefWindowProc(hwnd, message, wparam, lparam);
}

void window::init(creation_args const& args)
{
	hwnd_ = nullptr;
	width_ = height_ = 0;
	cursor_ = LoadCursor(NULL, IDC_ARROW);
	fullscreen_ = false;
	message_handling_enabled_ = false;
	drag_accept_files_enabled_ = false;

	post_thread_message(WM_USER, (WPARAM)this, (LPARAM)&args);

	// wait for the window create completion
	while (!hwnd_)
	{
		boost::this_thread::yield();
	}
}

void window::create(creation_args args)
{
	style_ = args.style;

	// Choose the window style according to the Style parameter
	DWORD window_style = WS_CLIPCHILDREN;
	if (style_ == GWS_NONE)
	{
		window_style |= WS_POPUP;
	}
	else
	{
		if (style_ & GWS_TITLEBAR) window_style |= WS_CAPTION | WS_MINIMIZEBOX;
		if (style_ & GWS_RESIZE)   window_style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		if (style_ & GWS_CLOSE)    window_style |= WS_SYSMENU;
	}

	if (style_ & GWS_APPWINDOW)
	{
		window_style |= WS_OVERLAPPEDWINDOW;
	}
	RECT window_rect;
	window_rect.left = args.left;
	window_rect.top = args.top;
	window_rect.right = window_rect.left + args.width;
	window_rect.bottom = window_rect.top + args.height;

	// In windowed mode, adjust width and height so that window will have the requested client area
	if (!(style_ & GWS_FULLSCREEN))
	{
		AdjustWindowRect(&window_rect, window_style, false);
	}

	if (args.splash.empty() && (args.style & GWS_HIDDEN) == 0)
	{
		window_style |= WS_VISIBLE;
	}

	std::wstring caption = args.caption;
#if TARGET(DEBUG)
	caption += L" (DEBUG)";
#endif

	hwnd_ = CreateWindowW(WINDOW_CLASS_NAME, caption.c_str(), window_style,
		window_rect.left, window_rect.top,
		window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
		NULL, NULL, GetModuleHandle(NULL), this);
	// Switch to fullscreen if requested
	if ((style_ & GWS_FULLSCREEN))
	{
		video_mode const mode(args.width, args.height, args.bpp);
		switch_to_fullscreen(mode);
	}

	// Get the actual size of the window, which can be smaller even after the call to AdjustWindowRect
	// This happens when the window is bigger than the desktop
	update_window_size();

	if (!args.splash.empty())
	{
		use_as_splash_screen(args.splash);
		ShowWindow(hwnd_, SW_SHOW);
	}
}

void window::destroy()
{
	if (hwnd_)
	{
		if(fullscreen_)
		{
			ChangeDisplaySettings(NULL,0);
			show_mouse_cursor(true);
		}
		::DestroyWindow(hwnd_);
		hwnd_ = NULL;
	}
}

void window::update_window_size()
{
	RECT client_rect;
	if (GetClientRect(hwnd_, &client_rect))
	{
		width_  = client_rect.right - client_rect.left;
		height_ = client_rect.bottom - client_rect.top;
	}
}

void window::show_mouse_cursor(bool show)
{
	cursor_= (show? LoadCursor(NULL, IDC_ARROW) : NULL);
}

bool window::process_event(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result)
{
	if (!hwnd_)
	{
		return false;
	}

	if (process_event_by_sink(message | SINKING, wparam, lparam, result))
	{
		return true;
	}

	switch (message)
	{
	case WM_SIZE:
		update_window_size();
		on_resize(width_, height_);
		break;

	case WM_DESTROY:
		if (drag_accept_files_enabled_)
		{
			DragAcceptFiles(hwnd_, FALSE);
		}
		break;
	case WM_CLOSE:
		if (style_ & GWS_APPWINDOW)
		{
			PostQuitMessage(0);
		}
		on_event("close");
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
	case WM_KEYUP:
	case WM_KEYDOWN:
	case WM_CHAR:
		on_input(input_event(message, wparam, lparam));
		break;

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP) wparam;
			uint32_t const count = DragQueryFile(hDrop, (UINT)-1, NULL, NULL);
			shared_wstrings files = boost::make_shared<wstrings>(count);
			for (uint32_t i = 0; i < count; ++i)
			{
				std::wstring& file = (*files)[i];
				size_t const len = DragQueryFile(hDrop, i, NULL, 0) + 1;
				file.resize(len);
				DragQueryFile(hDrop, i, &file[0], file.size());
				if (file.back() == 0) file.pop_back();
			}
			DragFinish(hDrop);
			runtime::main_loop().schedule(boost::bind(&window::drag_accept_files, this, files));
		}
		break;

	case WM_SETCURSOR:
		::SetCursor(cursor_);
		result = TRUE;
		return true;

	case WM_PAINT:
		if (splash_bitmap_)
		{
			HDC hdc = ::GetDC(hwnd_);

			BITMAPFILEHEADER const* bfh = (BITMAPFILEHEADER*)splash_bitmap_.get();
			BITMAPINFO const* bmi = (BITMAPINFO*)(bfh + 1);

			void const* data = ((uint8_t*)bfh + bfh->bfOffBits);

			::SetDIBitsToDevice(hdc, 0, 0, bmi->bmiHeader.biWidth, bmi->bmiHeader.biHeight,
				0, 0, 0, bmi->bmiHeader.biHeight, data, bmi, DIB_RGB_COLORS);

			::ReleaseDC(hwnd_, hdc);
			result = 0;
			return true;
		}
		break;
	}

	if (process_event_by_sink(message, wparam, lparam, result))
	{
		return true;
	}

	if (message_handling_enabled_)
	{
		runtime::main_loop().schedule(boost::bind(&window::v8_process_message, this,
			(uint32_t)message, (uint32_t)wparam, (uint32_t)lparam));
		return true;
	}
	return false;
}

void window::use_as_splash_screen(std::wstring const& filename)
{
	if (filename.empty())
	{
		splash_bitmap_.reset();
		show_frame(true);
		::InvalidateRect(hwnd_, NULL, FALSE);
	}
	else
	{
		boost::filesystem::path const fn = filename;
		boost::iostreams::mapped_file_source file(fn);
		if (file.is_open())
		{
			splash_bitmap_.reset(new uint8_t[file.size()]);
			memcpy(splash_bitmap_.get(), file.data(), file.size());
		}
		file.close();

		show_frame(false);

		BITMAPFILEHEADER const* bfh = (BITMAPFILEHEADER*)splash_bitmap_.get();
		BITMAPINFO const* bmi = (BITMAPINFO*)(bfh+1);
		int const width  = bmi->bmiHeader.biWidth;
		int const height = bmi->bmiHeader.biHeight;
		int const left   = (get_current_video_mode().width - width)  / 2;
		int const top    = (get_current_video_mode().height - height) / 2;

		::SetWindowPos(hwnd_, HWND_TOPMOST, left, top, width, height, SWP_SHOWWINDOW);
		::InvalidateRect(hwnd_, NULL, FALSE);
	}
}

void window::v8_process_message(uint32_t message, uint32_t wparam, uint32_t lparam)
{
	v8::Handle<v8::Value> args[3] = { v8pp::to_v8(message), v8pp::to_v8(wparam), v8pp::to_v8(lparam) };
	event_handlers_.call("message", v8pp::to_v8(this)->ToObject(), 3, args);
}

void window::show( bool visible )
{
	::ShowWindow(hwnd_, visible? SW_SHOW : SW_HIDE);
}

void window::switch_to_fullscreen(video_mode const& mode)
{
	DEVMODE DevMode;
	DevMode.dmSize       = sizeof(DEVMODE);
	DevMode.dmPelsWidth  = mode.width;
	DevMode.dmPelsHeight = mode.height;
	DevMode.dmBitsPerPel = mode.bpp;
	DevMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

	// Apply fullscreen mode
	if (ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		std::cerr << "Failed to change display mode for fullscreen" << std::endl;
		return;
	}

	// Change window style (no border, no titlebar, ...)
	SetWindowLong(hwnd_, GWL_STYLE,   WS_POPUP);
	SetWindowLong(hwnd_, GWL_EXSTYLE, WS_EX_APPWINDOW);

	// And resize it so that it fits the entire screen
	SetWindowPos(hwnd_, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
	ShowWindow(hwnd_, SW_SHOW);

	// SetPixelFormat can fail (really ?) if window style doesn't contain these flags
	long style = GetWindowLong(hwnd_, GWL_STYLE);
	SetWindowLong(hwnd_, GWL_STYLE, style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	fullscreen_ = true;

	update_window_size();
}

/*
void window::toggle_fullscreen(void)
{
	if(fullscreen_)
	{
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE
		SetWindowLong(hwnd_, GWL_STYLE,   WS_POPUP);
		SetWindowLong(hwnd_, GWL_EXSTYLE, WS_EX_APPWINDOW);
	}
	else
	{
		// Change window style (no border, no titlebar, ...)
		SetWindowLong(hwnd_, GWL_STYLE,   WS_POPUP);
		SetWindowLong(hwnd_, GWL_EXSTYLE, WS_EX_APPWINDOW);

		ShowWindow(hwnd_,SW_MAXIMIZE);
	}
}
*/

void window::set_focus()
{
	::SetForegroundWindow(hwnd_);
	::SetFocus(hwnd_);
}

void window::show_frame(bool show)
{	
	if(show)
	{
		SetWindowLong(hwnd_, GWL_STYLE,   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE);
//		SetWindowLong(hwnd_, GWL_EXSTYLE, WS_EX_APPWINDOW);
	}
	else
	{
		SetWindowLong(hwnd_, GWL_STYLE,   WS_POPUP | WS_CLIPCHILDREN | WS_VISIBLE);
//		SetWindowLong(hwnd_, GWL_EXSTYLE, WS_EX_APPWINDOW);
//		SetWindowPos(hwnd_, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOREPOSITION);
	}

	update_window_size();
}

void window::set_window_rect(uint32_t left, uint32_t top, uint32_t width, uint32_t height)
{
	SetWindowPos(hwnd_, NULL, left, top, width, height, SWP_SHOWWINDOW);
	update_window_size();
}

void window::set_topmost(bool topmost)
{
	SetWindowPos(hwnd_, topmost? HWND_TOP : NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION);
}

static Handle<Value> to_v8(RECT const& rc)
{
	HandleScope scope;

	Handle<Object> o = Object::New();

	set_option(o, "left",   rc.left);
	set_option(o, "top",    rc.top);
	set_option(o, "width",  rc.right - rc.left);
	set_option(o, "height", rc.bottom - rc.top);

	return scope.Close(o);
}

v8::Handle<v8::Value> window::get_window_rect(Arguments const&)
{
	RECT rc;
	GetWindowRect(hwnd_, &rc);

	return to_v8(rc);
}

v8::Handle<v8::Value> window::get_client_rect( v8::Arguments const& )
{
	RECT rc;
	GetClientRect(hwnd_, &rc);

	return to_v8(rc);
}

window& window::on(std::string const& name, Handle<Value> fn)
{
	if (name == "message")
	{
		message_handling_enabled_ = true;
	}
	else if (name == "drag_accept_files")
	{
		runtime::main_loop().schedule(boost::bind(&window::drag_accept_files_enable_impl, this, true));
	}

	event_handlers_.on(name, fn);
	return *this;
}

window& window::off(std::string const& name)
{
	if (name == "message")
	{
		message_handling_enabled_ = false;
	}
	else if (name == "drag_accept_files")
	{
		runtime::main_loop().schedule(boost::bind(&window::drag_accept_files_enable_impl, this, false));
	}

	event_handlers_.off(name);
	return *this;
}

void window::load_icon_from_file(std::wstring const& file)
{
	HANDLE hIcon = LoadImageW(NULL, file.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	if (hIcon)
	{
		PostMessage(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		return;
	}

	hIcon = LoadImageW(NULL, file.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	if (hIcon)
	{
		PostMessage(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}
}

void window::drag_accept_files_enable_impl(bool enable)
{
	drag_accept_files_enabled_ = enable;
	::DragAcceptFiles(hwnd_, enable);
}

void window::drag_accept_files(shared_wstrings files)
{
	HandleScope scope;

	v8::Handle<v8::Value> args[1] = { v8pp::to_v8(*files) };
	event_handlers_.call("drag_accept_files", v8pp::to_v8(this)->ToObject(), 1, args);
}

input_event::input_event(UINT message, WPARAM wparam, LPARAM lparam)
	: type_and_state_(UNKNOWN)
{
	if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST)
	{
		type_and_state_ = mouse_type_and_state(message, wparam);
		data_.mouse.x = (int)LOWORD(lparam);
		data_.mouse.y = (int)HIWORD(lparam);
		data_.mouse.dx = (message == WM_MOUSEWHEEL? 0 : GET_WHEEL_DELTA_WPARAM(wparam));
		data_.mouse.dy = (message == WM_MOUSEHWHEEL? 0 : GET_WHEEL_DELTA_WPARAM(wparam));
		repeats_ = (type() == MOUSE_CLICK? 2 : 0);
	}
	else if (message >= WM_KEYDOWN && message <= WM_CHAR)
	{
		type_and_state_ = key_type_and_state(message);
		data_.key.vk_code = wparam;
		data_.key.scancode = (lparam >> 16) & 0xFF;
		repeats_ = LOWORD(lparam);
	}
}

uint32_t input_event::mouse_type_and_state(UINT message, WPARAM wparam)
{
	uint32_t result = 0;

	// type and button
	switch (message)
	{
	case WM_MOUSEMOVE:
		result = MOUSE_MOVE;
		break;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		result = MOUSE_WHEEL;
		break;
	case WM_LBUTTONDOWN:
		result = MOUSE_DOWN | (1 << BUTTON_SHIFT);
		break;
	case WM_MBUTTONDOWN:
		result = MOUSE_DOWN | (2 << BUTTON_SHIFT);
		break;
	case WM_RBUTTONDOWN:
		result = MOUSE_DOWN | (3 << BUTTON_SHIFT);
		break;
	case WM_XBUTTONDOWN:
		result = MOUSE_DOWN | ((3 + HIWORD(wparam)) << BUTTON_SHIFT);
		break;
	case WM_LBUTTONUP:
		result = MOUSE_UP | (1 << BUTTON_SHIFT);
		break;
	case WM_MBUTTONUP:
		result = MOUSE_UP | (2 << BUTTON_SHIFT);
		break;
	case WM_RBUTTONUP:
		result = MOUSE_UP | (3 << BUTTON_SHIFT);
		break;
	case WM_XBUTTONUP:
		result = MOUSE_UP | ((3 + HIWORD(wparam)) << BUTTON_SHIFT);
		break;
	case WM_LBUTTONDBLCLK:
		result = MOUSE_CLICK | (1 << BUTTON_SHIFT);
		break;
	case WM_MBUTTONDBLCLK:
		result = MOUSE_CLICK | (2 << BUTTON_SHIFT);
		break;
	case WM_RBUTTONDBLCLK:
		result = MOUSE_CLICK | (3 << BUTTON_SHIFT);
		break;
	case WM_XBUTTONDBLCLK:
		result = MOUSE_CLICK | ((3 + HIWORD(wparam)) << BUTTON_SHIFT);
		break;
	default:
		_aspect_assert(false && "unknown mouse message");
		result = UNKNOWN;
		break;
	}

	// state
	wparam = GET_KEYSTATE_WPARAM(wparam);
	result |= (wparam & MK_CONTROL? CTRL_DOWN : 0);
	result |= (HIWORD(GetKeyState(VK_MENU))? ALT_DOWN : 0);
	result |= (wparam & MK_SHIFT? SHIFT_DOWN : 0);
	result |= (wparam & MK_LBUTTON? LBUTTON_DOWN : 0);
	result |= (wparam & MK_MBUTTON? MBUTTON_DOWN : 0);
	result |= (wparam & MK_RBUTTON? RBUTTON_DOWN : 0);
	result |= (wparam & MK_XBUTTON1? XBUTTON1_DOWN : 0);
	result |= (wparam & MK_XBUTTON2? XBUTTON2_DOWN : 0);

	return result;
}

uint32_t input_event::key_type_and_state(UINT message)
{
	uint32_t result = 0;

	// type
	switch (message)
	{
	case WM_KEYDOWN:
		result = KEY_DOWN;
		break;
	case WM_KEYUP:
		result = KEY_UP;
		break;
	case WM_CHAR:
		result = KEY_CHAR;
		break;
	default:
		_aspect_assert(false && "unknown key message");
		result = UNKNOWN;
		break;
	}

	// state
	result |= (HIWORD(GetAsyncKeyState(VK_CONTROL))? CTRL_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_MENU))? ALT_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_SHIFT))? SHIFT_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_LBUTTON))? LBUTTON_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_MBUTTON))? MBUTTON_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_RBUTTON))? RBUTTON_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_XBUTTON1))? XBUTTON1_DOWN : 0);
	result |= (HIWORD(GetAsyncKeyState(VK_XBUTTON2))? XBUTTON2_DOWN : 0);

	return result;
}

}} // namespace aspect::gui

#endif // OS(WINDOWS)
