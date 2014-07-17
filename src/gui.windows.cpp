#include "oxygen.hpp"
#include "gui.windows.hpp"

#include <boost/iostreams/device/mapped_file.hpp>

#include <windowsx.h>
#include <shellapi.h>
#include <commdlg.h>

namespace aspect { namespace gui {

static wchar_t const WINDOW_CLASS_NAME[] = L"jsx_generic";

static boost::thread window_thread_;
static boost::mutex window_create_mutex_;
static boost::condition_variable window_created_cv_;

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
	os::set_thread_name("window::message_loop");

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

				boost::mutex::scoped_lock lock(window_create_mutex_);
				w->create(*args);
				window_created_cv_.notify_one();
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

	event e(hwnd, message, wparam, lparam);
	window* wnd = (window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (wnd && wnd->hwnd_ == hwnd && wnd->process(e))
	{
		return e.result;
	}

	/*
	// don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
	if (message == WM_CLOSE)
		return 0;
	*/

	return ::DefWindowProc(hwnd, message, wparam, lparam);
}

window::window(v8::FunctionCallbackInfo<v8::Value> const& args)
	: window_base(runtime::instance(args.GetIsolate()))
{
	init(creation_args(args));
}

void window::init(creation_args const& args)
{
	hwnd_ = nullptr;
	size_.width = size_.height = 0;
	cursor_ = LoadCursor(NULL, IDC_ARROW);
	forced_cursor_ = false;
	is_cursor_visible_ = true;
	fullscreen_ = false;
	message_handling_enabled_ = false;
	drag_accept_files_enabled_ = false;
	capture_count_ = 0;

	post_thread_message(WM_USER, (WPARAM)this, (LPARAM)&args);

	// wait for the window create completion
	boost::mutex::scoped_lock lock(window_create_mutex_);
	while (!hwnd_)
	{
		window_created_cv_.wait(lock);
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

	// In windowed mode, adjust width and height so that window will have the requested client area
	if (!(style_ & GWS_FULLSCREEN))
	{
		if (args.width <= 0 || args.height <= 0)
		{
			args.left = args.top = args.width = args.height = CW_USEDEFAULT;
		}
		else
		{
			RECT window_rect = { args.left, args.top, args.left + args.width, args.top + args.height };
			AdjustWindowRect(&window_rect, window_style, false);
			args.left = window_rect.left;
			args.top = window_rect.top;
			args.width = window_rect.right - window_rect.left;
			args.height = window_rect.bottom - window_rect.top;
		}
	}

	if (args.splash.empty() && (args.style & GWS_HIDDEN) == 0)
	{
		window_style |= WS_VISIBLE;
	}

#if TARGET(DEBUG)
	args.caption += L" (DEBUG)";
#endif

	hwnd_ = CreateWindowW(WINDOW_CLASS_NAME, args.caption.c_str(), window_style,
		args.left, args.top, args.width, args.height, NULL, NULL, GetModuleHandle(NULL), this);
	// Switch to fullscreen if requested
	if ((style_ & GWS_FULLSCREEN))
	{
		video_mode const mode(args.width, args.height, args.bpp, 0);
		switch_to_fullscreen(mode);
	}

	// Get the actual size of the window, which can be smaller even after the call to AdjustWindowRect
	// This happens when the window is bigger than the desktop
	update_window_size();

	if (!args.icon.empty())
	{
		load_icon_from_file(args.icon);
	}

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
		size_.width  = client_rect.right - client_rect.left;
		size_.height = client_rect.bottom - client_rect.top;
	}
}

void window::set_cursor(HCURSOR cursor)
{
	if (!forced_cursor_)
	{
		cursor_= cursor;
	}
}

void window::set_stock_cursor(cursor_id id)
{
	LPCTSTR name = NULL;
	switch (id)
	{
	case ARROW:
		name = IDC_ARROW;
		break;
	case INPUT:
		name = IDC_IBEAM;
		break;
	case HAND:
		name = IDC_HAND;
		break;
	case CROSS:
		name = IDC_CROSS;
		break;
	case MOVE:
		name = IDC_SIZEALL;
		break;
	case WAIT:
		name = IDC_WAIT;
		break;
	default:
		return;
	}
	if (name)
	{
		forced_cursor_ = (id != ARROW);
		cursor_ = LoadCursor(NULL, name);
		::PostMessage(hwnd_, WM_SETCURSOR, 0, HTCLIENT);
	}
}

void window::show_mouse_cursor(bool show)
{
	is_cursor_visible_ = show;
	::PostMessage(hwnd_, WM_SETCURSOR, 0, HTCLIENT);
}

void window::capture_mouse(bool capture)
{
	if (capture)
	{
		if (++capture_count_ == 1) SetCapture(hwnd_);
	}
	else
	{
		if (--capture_count_ == 0) ReleaseCapture();
	}
}

void window::set_mouse_pos(int x, int y)
{
	POINT pt;
	pt.x = x; pt.y = y;
	ClientToScreen(hwnd_, &pt);
	SetCursorPos(pt.x, pt.y);
}

bool window::process(event& e)
{
	if (!hwnd_)
	{
		return false;
	}

	switch (e.message)
	{
	case WM_SIZE:
		update_window_size();
		on_resize(size_);
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
		{
			input_event const inp_e(e);
			switch (inp_e.type())
			{
			case input_event::MOUSE_DOWN:
				capture_mouse(true);
				break;
			case input_event::MOUSE_UP:
				capture_mouse(false);
				break;
			}
			on_input(inp_e);
		}
		break;

	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)e.wparam;
			UINT const count = DragQueryFile(hDrop, (UINT)-1, NULL, NULL);
			shared_wstrings files = boost::make_shared<wstrings>(count);
			for (UINT i = 0; i < count; ++i)
			{
				std::wstring& file = (*files)[i];
				UINT const len = DragQueryFile(hDrop, i, NULL, 0) + 1;
				file.resize(len);
				DragQueryFile(hDrop, i, &file[0], len);
				if (file.back() == 0) file.pop_back();
			}
			DragFinish(hDrop);
			rt_.main_loop().schedule(boost::bind(&window::drag_accept_files, this, files));
		}
		break;

	case WM_SETCURSOR:
		if (LOWORD(e.lparam) == HTCLIENT)
		{
			// set custom cursor for client area only
			::SetCursor(is_cursor_visible_? cursor_ : NULL);
			e.result = TRUE;
			return true;
		}
		break;

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
			e.result = 0;
			return true;
		}
		break;
	}

	if (message_handling_enabled_)
	{
		rt_.main_loop().schedule(boost::bind(&window::v8_process_message, this, e));
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

void window::v8_process_message(event e)
{
	v8::Isolate* isolate = rt_.isolate();
	v8::HandleScope scope(isolate);

	v8::Handle<v8::Value> args[3] = {
		v8pp::to_v8(isolate, e.message),
		v8pp::to_v8(isolate, e.wparam),
		v8pp::to_v8(isolate, e.lparam)
	};
	emit(isolate, "message", 3, args);
}

void window::show(bool visible)
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

void window::toggle_fullscreen()
{
	DWORD const style = GetWindowLong(hwnd_, GWL_STYLE);

	fullscreen_ = !fullscreen_;
	if (fullscreen_)
	{
		prev_placement_.length = sizeof(prev_placement_);
		GetWindowPlacement(hwnd_, &prev_placement_);

		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(MonitorFromWindow(hwnd_, MONITOR_DEFAULTTOPRIMARY), &mi);

		// If we would call SetWindowPos() with mi.rcMonitor, the window becomes
		// topmost in a kind of exclusive mode on my system with Intel HD Graphics 4600 -
		// task switching doesn't work, other windows aren't displayed. If the window rect
		// is a bit less than the monitor size, all works fine. Decreasing mi.rcMonitor.top
		// for 1px in order to resolve this issue. Decreasing rcMonitor.bottom doesn't work-
		// Windows taskbar is being still visible in this case.
		--mi.rcMonitor.top;

		SetWindowLong(hwnd_, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
		SetWindowPos(hwnd_, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(hwnd_, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPos(hwnd_, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		SetWindowPlacement(hwnd_, &prev_placement_);
	}
//	update_window_size();
}

void window::set_focus()
{
	::SetForegroundWindow(hwnd_);
	::SetFocus(hwnd_);
}

void window::show_frame(bool show)
{	
	DWORD const style = GetWindowLong(hwnd_, GWL_STYLE);
	DWORD const frame_style = WS_CAPTION | WS_THICKFRAME;

	SetWindowLong(hwnd_, GWL_STYLE, show? style | frame_style : style & ~frame_style);
	SetWindowPos(hwnd_, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	update_window_size();
}

void window::set_topmost(bool topmost)
{
	SetWindowPos(hwnd_, topmost? HWND_TOP : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION);
}

rectangle<int> window::rect() const
{
	RECT rc;
	GetWindowRect(hwnd_, &rc);
	return rectangle<int>(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

void window::set_rect(rectangle<int> const& rect)
{
	SetWindowPos(hwnd_, NULL, rect.left, rect.top, rect.width, rect.height, 0);
	update_window_size();
}

window& window::on(std::string const& name, v8::Handle<v8::Function> fn)
{
	if (name == "message")
	{
		message_handling_enabled_ = true;
	}
	else if (name == "drag_accept_files")
	{
		rt_.main_loop().schedule(boost::bind(&window::drag_accept_files_enable_impl, this, true));
	}

	window_base::on(rt_.isolate(), name, fn);
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
		rt_.main_loop().schedule(boost::bind(&window::drag_accept_files_enable_impl, this, false));
	}

	window_base::off(rt_.isolate(), name);
	return *this;
}

void window::load_icon_from_file(std::wstring const& file)
{
	HANDLE hIcon = LoadImageW(NULL, file.c_str(), IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_LOADFROMFILE);
	if (hIcon)
	{
		SendMessage(hwnd_, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}

	hIcon = LoadImageW(NULL, file.c_str(), IMAGE_ICON, 
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_LOADFROMFILE);
	if (hIcon)
	{
		SendMessage(hwnd_, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}
}

void window::drag_accept_files_enable_impl(bool enable)
{
	drag_accept_files_enabled_ = enable;
	::DragAcceptFiles(hwnd_, enable);
}

void window::drag_accept_files(shared_wstrings files)
{
	v8::Isolate* isolate = rt_.isolate();
	v8::HandleScope scope(isolate);

	v8::Handle<v8::Value> args[1] = { v8pp::to_v8(isolate, *files) };
	emit(isolate, "drag_accept_files", 1, args);
}

input_event::input_event(event const& e)
	: type_and_state_(UNKNOWN)
{
	if (e.message >= WM_MOUSEFIRST && e.message <= WM_MOUSELAST)
	{
		type_and_state_ = mouse_type_and_state(e.message, e.wparam);
		data_.mouse.x = GET_X_LPARAM(e.lparam);
		data_.mouse.y = GET_Y_LPARAM(e.lparam);
		if (e.message == WM_MOUSEWHEEL || e.message == WM_MOUSEHWHEEL)
		{
			POINT pt;
			pt.x = data_.mouse.x;
			pt.y = data_.mouse.y;
			ScreenToClient(e.window, &pt);
			data_.mouse.x = pt.x;
			data_.mouse.y = pt.y;
		}
		data_.mouse.dx = (e.message == WM_MOUSEHWHEEL? GET_WHEEL_DELTA_WPARAM(e.wparam) : 0);
		data_.mouse.dy = (e.message == WM_MOUSEWHEEL?  GET_WHEEL_DELTA_WPARAM(e.wparam) : 0);
		repeats_ = (type() == MOUSE_CLICK? 2 : 0);
	}
	else if (e.message >= WM_KEYDOWN && e.message <= WM_CHAR)
	{
		type_and_state_ = key_type_and_state(e.message);
		data_.key.vk_code = static_cast<uint32_t>(e.wparam);
		data_.key.scan_code = static_cast<uint32_t>(e.lparam);
		data_.key.key_code =  static_cast<uint32_t>(e.wparam);
		data_.key.char_code = static_cast<uint32_t>(e.message == WM_CHAR? e.wparam : 0);
		repeats_ = LOWORD(e.lparam);
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

void window::run_file_dialog(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();

	v8::EscapableHandleScope scope(isolate);

	std::string type = "open";
	bool multiselect = false;
	std::wstring filter;
	std::wstring title;
	std::wstring default_dir;
	std::wstring default_ext;

	std::vector<wchar_t> filename_buf;

	if (args[0]->IsObject())
	{
		v8::Local<v8::Object> options = args[0]->ToObject();

		get_option(isolate, options, "type", type);
		if (type == "open")
		{
			get_option(isolate, options, "multiselect", multiselect);
		}
		get_option(isolate, options, "title", title);

		v8::Local<v8::Object> filter_obj;
		if (get_option(isolate, options, "filter", filter_obj))
		{
			v8::Local<v8::Array> filter_keys = filter_obj->GetPropertyNames();
			for (uint32_t i = 0, count = filter_keys->Length(); i < count; ++i)
			{
				v8::Local<v8::Value> key = filter_keys->Get(i);
				v8::Local<v8::Value> val = filter_obj->Get(key);

				// description
				filter += v8pp::from_v8<std::wstring>(isolate, val);
				filter += L'\0';
				// mask
				filter += v8pp::from_v8<std::wstring>(isolate, key);
				filter += L'\0';
			}
		}

		get_option(isolate, options, "defaultDir", default_dir);

		get_option(isolate, options, "defaultExt", default_ext);
		if (!default_ext.empty() && default_ext[0] == L'.')
		{
			default_ext.erase(0, 1);
		}

		std::wstring default_name;
		if (get_option(isolate, options, "defaultName", default_name))
		{
			std::copy(default_name.begin(), default_name.end(), std::back_inserter(filename_buf));
		}
	}
	filename_buf.resize(8192);

	OPENFILENAMEW ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd_;
	ofn.lpstrFile = filename_buf.data();
	ofn.nMaxFile = static_cast<DWORD>(filename_buf.size());
	ofn.Flags = (multiselect? OFN_ALLOWMULTISELECT : 0) | OFN_ENABLESIZING | OFN_EXPLORER
		| OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST
		| OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_OVERWRITEPROMPT;

	if (!filter.empty())
	{
		ofn.lpstrFilter = filter.c_str();
	}
	if (!title.empty())
	{
		ofn.lpstrTitle = title.c_str();
	}
	if (!default_dir.empty())
	{
		ofn.lpstrInitialDir = default_dir.c_str();
	}
	if (!default_ext.empty())
	{
		ofn.lpstrDefExt = default_ext.c_str();
	}

	BOOL ok;
	if (type == "open")
	{
		ok = GetOpenFileNameW(&ofn);
	}
	else if (type == "save")
	{
		ok = GetSaveFileNameW(&ofn);
	}
	else
	{
		throw std::invalid_argument("unknown dialog type " + type);
	}

	if (ok)
	{
		if (multiselect)
		{
			v8::Local<v8::Array> filenames = v8::Array::New(isolate);

			std::wstring const dirname = ofn.lpstrFile;
			ofn.lpstrFile += dirname.size() + 1;

			uint32_t filecount = 0;
			for (filecount = 0; *ofn.lpstrFile; ++filecount)
			{
				std::wstring const filename = ofn.lpstrFile;
				ofn.lpstrFile += filename.size() + 1;
				filenames->Set(filecount, v8pp::to_v8(isolate, dirname + L'\\' + filename));
			}

			if (filecount == 0)
			{
				// single selection
				filenames->Set(0, v8pp::to_v8(isolate, dirname));
			}

			args.GetReturnValue().Set(scope.Escape(filenames));
		}
		else
		{
			v8::Local<v8::Value> filename = v8pp::to_v8<wchar_t const*>(isolate, ofn.lpstrFile);
			args.GetReturnValue().Set(scope.Escape(filename));
		}
	}
}

screen_info::screen_info(window* w)
{
	scale = 1;

	HWND hwnd = w? static_cast<HWND>(*w) : GetActiveWindow();
	HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFOEXW monitor_info;
	memset(&monitor_info, 0, sizeof(monitor_info));
	monitor_info.cbSize = sizeof(monitor_info);
	GetMonitorInfoW(hmonitor, &monitor_info);

	HDC hdc = CreateDCW(L"DISPLAY", monitor_info.szDevice, NULL, NULL);
	color_depth = GetDeviceCaps(hdc, BITSPIXEL);
	color_depth_per_component = color_depth >= 24? 8 : 0;
	DeleteDC(hdc);

	rect.left = monitor_info.rcMonitor.left;
	rect.top = monitor_info.rcMonitor.top;
	rect.width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
	rect.height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;

	work_rect.left = monitor_info.rcWork.left;
	work_rect.top = monitor_info.rcWork.top;
	work_rect.width = monitor_info.rcWork.right - monitor_info.rcWork.left;
	work_rect.height = monitor_info.rcWork.bottom - monitor_info.rcWork.top;
}

}} // namespace aspect::gui
