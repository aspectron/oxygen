#include "oxygen.hpp"

#if OS(WINDOWS)

#include "shellapi.h"

using namespace v8;
using namespace v8::juice;

namespace aspect { namespace gui {

HBRUSH gs_hbrush = NULL;
HINSTANCE gs_hinstance = NULL;
LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

void init(HINSTANCE hinstance)
{
	gs_hinstance = hinstance;

	WNDCLASS wc = {0};			// window's class struct
	HBRUSH hBrush = NULL;		// will contain the background color of the main window

	// set the background color to black (this may be changed to whatever is needed)
	gs_hbrush = CreateSolidBrush(RGB(0, 0, 0));

	// we must specify the attribs for the window's class
	wc.style			= CS_HREDRAW|CS_VREDRAW|CS_OWNDC|CS_DBLCLKS;	// style bits (CS_OWNDC very important for OGL)
	wc.lpfnWndProc		= (WNDPROC)window_proc;					// window procedure to use
	wc.cbClsExtra		= 0;								// no extra data
	wc.cbWndExtra		= 0;								// no extra data
	wc.hInstance		= gs_hinstance;						// associated instance
	wc.hIcon			= NULL;								// use default icon temporarily
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);		// use default cursor (Windows owns it, so don't unload)
	wc.hbrBackground	= hBrush;							// background brush (don't destroy, let windows handle it)
	wc.lpszClassName	= L"jsx_generic";//APP_CLASSNAME;					// class name

	// this will create or not create a menu for the main window (depends on app settings)
	wc.lpszMenuName = NULL; // (APP_ALLOW_MENU) ? MAKEINTRESOURCE(IDR_MAINFRAME) : NULL;

	// now, we can register this class
	RegisterClass(&wc);

	// start windows event processing thread
//	gs_oxygen_thread.reset(new oxygen_thread());
	oxygen_thread::start();
}

void cleanup(void)
{
//	gs_oxygen_thread.reset();
	oxygen_thread::stop();

	// clean-up (windows specific items)
	if(gs_hbrush != NULL) DeleteObject(gs_hbrush);
}



LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

	// Associate handle and Window instance when the creation message is received
	if (message == WM_CREATE)
	{
		LONG_PTR _pwnd = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lparam)->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, _pwnd);
	}

	window *pwnd = window::pwnd_from_hwnd(hwnd);
	if (pwnd)
	{
//		boost::shared_ptr<window> test = pwnd->shared_from_this();
		pwnd->process_event(message, wparam, lparam);
	}

	// don't forward the WM_CLOSE message to prevent the OS from automatically destroying the window
	if (message == WM_CLOSE)
		return 0;

	return ::DefWindowProc(hwnd, message, wparam, lparam);
}

// ------------------------------------------------------	



// ------------------------------------------------------	
/*
boost::shared_ptr<window> window::shared_from_this()
{
	return boost::enable_shared_from_this<window>::shared_from_this();
}
*/

window::window(const creation_args *args)
:	hwnd_(NULL),
	window_created_(NULL),
	style_(0),
	fullscreen_(false),
	message_handling_enabled_(false),
	drag_accept_files_enabled_(false)
{
//	window_thread_ = gs_oxygen_thread;

	oxygen_thread::schedule(boost::bind(&window::create_window_impl, this, args));

	// TODO - WHAT IF CREATE WINDOW WILL FAIL?  IT WILL RESULT IN hwnd_ BEING NULL AND A DEADLOCK!

	while(aspect::utils::atomic_is_null(window_created_)) //hwnd_))
	{
//		printf("waiting...\n");
//		boost::this_thread::yield();
		Sleep(5);
	}
}

void window::create_window_impl( const creation_args *args) //video_mode mode, const std::string& caption, unsigned long requested_style ) 
{
//	boost::shared_ptr<window> test = shared_from_this();
//	printf("ENTERED CREATE_WINDOW_IMPL\n");

	// Compute position and size
	int left   = (GetDeviceCaps(GetDC(NULL), HORZRES) - args->width)  / 2;
	int top    = (GetDeviceCaps(GetDC(NULL), VERTRES) - args->height) / 2;
	int width  = width_ = args->width;
	int height = height_ = args->height;

	style_ = args->style;

	// Choose the window style according to the Style parameter
	DWORD style = WS_CLIPCHILDREN; // WS_VISIBLE;
	if (style_ == GWS_NONE)
	{
		style |= WS_POPUP; // WS_OVERLAPPEDWINDOW; // WS_POPUP;
	}
	else
	{
		if (style_ & GWS_TITLEBAR) style |= WS_CAPTION | WS_MINIMIZEBOX;
		if (style_ & GWS_RESIZE)   style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
		if (style_ & GWS_CLOSE)    style |= WS_SYSMENU;
	}

	if(style_ & GWS_APPWINDOW)
		style |= WS_OVERLAPPEDWINDOW;

	// In windowed mode, adjust width and height so that window will have the requested client area
/*
	if (!(style_ & AWS_FULLSCREEN))
	{
		RECT rect = {0, 0, width, height};
		AdjustWindowRect(&rect, style, false);
		width  = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
*/
#if 0
	wchar_t wcs_caption[256];
	int nb_chars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, caption.c_str(), static_cast<int>(caption.size()), wcs_caption, sizeof(wcs_caption) / sizeof(*wcs_caption));
	wcs_caption[nb_chars] = L'\0';
	hwnd_ = CreateWindowW(L"jsx", wcs_caption, style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);
#else

//	DWORD style =  args->frame ? (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) : (WS_POPUP | WS_CLIPCHILDREN); // | WS_VISIBLE;

	if(!args->splash.length())
		style |= WS_VISIBLE;

	std::string caption = args->caption;
#if TARGET(DEBUG)
	caption += " (DEBUG)";
#endif

	hwnd_ = CreateWindowA("jsx_generic", caption.c_str(), style, left, top, width, height, NULL, NULL, GetModuleHandle(NULL), this);

//	printf("WINDOW CREATED: %08x\n",(int)hwnd_);

#endif				  

	// Switch to fullscreen if requested
/*	if (style_ & AWS_FULLSCREEN)
	{
		video_mode mode(args->width,args->height,args->bpp);
		switch_to_fullscreen(mode);
	}
*/
	// Get the actual size of the window, which can be smaller even after the call to AdjustWindowRect
	// This happens when the window is bigger than the desktop
	update_window_size();
	RECT client_rect;
//	GetClientRect(*this, &client_rect);
//	width_  = client_rect.right - client_rect.left;
//	height_ = client_rect.bottom - client_rect.top;

	HDC hdc = GetDC(hwnd_);
	HBRUSH hBrush = CreateSolidBrush(0);
	FillRect(hdc,&client_rect,hBrush);
	DeleteObject(hBrush);
	ReleaseDC(hwnd_,hdc);

	if(args->splash.length())
	{
		use_as_splash_screen(args->splash);
		ShowWindow(*this,SW_SHOW);
	}

	window_created_ = hwnd_;
}

window::~window()
{
	if(hwnd_)
	{
		destroy_window();
		while(aspect::utils::atomic_is_not_null(hwnd_))
			boost::this_thread::yield();
	}

//	window_thread_.reset();
}

void window::destroy_window()
{
	if(hwnd_)
		oxygen_thread::schedule(boost::bind(&window::destroy_window_impl, this));
}

void window::destroy_window_impl( void )
{
	if(hwnd_)
	{
// 		if(fullscreen_)
// 		{
// 			ChangeDisplaySettings(NULL,0);
// 			show_mouse_cursor(true);
// 		}

		::DestroyWindow(hwnd_);
		hwnd_ = NULL;
	}

}

void window::update_window_size(void)
{
	RECT client_rect;
	GetClientRect(*this, &client_rect);
	width_  = client_rect.right - client_rect.left;
	height_ = client_rect.bottom - client_rect.top;
}

void window::show_mouse_cursor( bool show )
{
	if(show)
		cursor_ = LoadCursor(NULL,IDC_ARROW);
	else
		cursor_ = NULL;

	::SetCursor(cursor_);
}

// static void window::process_events( void )
// {
// 	MSG msg;
// 	//					  while (GetMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
// 	while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
// 	{
// 		TranslateMessage(&msg);
// 		DispatchMessage(&msg);
// 	}
// }

// void window::process_events_blocking( void )
// {
// 	MSG msg;
// 	while (GetMessage(&msg, NULL, 0, 0))
// 		//while (PeekMessage(&msg, hwnd_, 0, 0, PM_REMOVE))
// 	{
// 		TranslateMessage(&msg);
// 		DispatchMessage(&msg);
// 	}
// }

void window::process_event( UINT message, WPARAM wparam, LPARAM lparam )
{
	if(hwnd_ == NULL)
		return;

	std::vector<boost::shared_ptr<event_sink>>::iterator iter;
	for(iter = event_sinks_.begin(); iter != event_sinks_.end(); iter++)
		if(!(*iter)->process_events(message,wparam,lparam))
			return;

	switch(message)
	{
		case WM_SIZE:
		{
			update_window_size();

			if(event_handlers_.has("resize"))
				runtime::main_loop().schedule(boost::bind(&window::v8_process_resize, this, width_, height_));

		} break;

		case WM_DESTROY:
		{
			if(drag_accept_files_enabled_)
				DragAcceptFiles(*this, FALSE);
		} break;

		case WM_CLOSE:
		{
/*			if(style_ & AWS_APPWINDOW)
			{
//				set_terminating();
				PostQuitMessage(true);
			}
*/

			if(event_handlers_.has("close"))
				runtime::main_loop().schedule(boost::bind(&window::v8_process_event, this, std::string("close")));


//			if(event_handlers_.has("close"))
//				runtime::main_loop().schedule(boost::bind(&window::v8_process_event, this, (uint32_t)message, (uint32_t)wparam, (uint32_t)lparam));

		} break;

		case WM_MOUSEMOVE: 
			{
				boost::shared_ptr<input_event> e = boost::make_shared<input_event>("mousemove");//
				e->cursor_.x = (double)LOWORD(lparam);
				e->cursor_.y = (double)HIWORD(lparam);
				runtime::main_loop().schedule(boost::bind(&window::v8_process_input_event, this, e));
			} break;
#if 0
		case WM_LBUTTONDOWN: { delegate_->mouse_button(0,true); } break;
		case WM_MBUTTONDOWN: { delegate_->mouse_button(1,true); } break;
		case WM_RBUTTONDOWN: { delegate_->mouse_button(2,true); } break;
		case WM_LBUTTONUP: { delegate_->mouse_button(0,false); } break;
		case WM_MBUTTONUP: { delegate_->mouse_button(1,false); } break;
		case WM_RBUTTONUP: { delegate_->mouse_button(2,false); } break;

		case WM_LBUTTONDBLCLK: { delegate_->mouse_button(0,true,2);  delegate_->mouse_button(0,false,2); } break;
		case WM_MBUTTONDBLCLK: { delegate_->mouse_button(1,true,2); delegate_->mouse_button(1,false,2); } break;
		case WM_RBUTTONDBLCLK: { delegate_->mouse_button(2,true,2); delegate_->mouse_button(2,false,2); } break;

		case WM_MOUSEWHEEL:
			{
				signed short delta = HIWORD(wparam);
				delegate_->mouse_wheel(0,delta);
			} break;

		case WM_MOUSEHWHEEL:
			{
				signed short delta = HIWORD(wparam);
				delegate_->mouse_wheel(delta,0);
			} break;
#endif

		case WM_KEYUP:
			{
				if(event_handlers_.has("keyup"))
				{
					boost::shared_ptr<input_event> e = boost::make_shared<input_event>("keyup");
					e->vk_code_ = (uint32_t)wparam;
					e->scancode_ = MapVirtualKey(e->vk_code_, MAPVK_VK_TO_VSC);
					if(isalnum(e->vk_code_))
					{
						e->charcode_ = e->vk_code_;
						e->char_[0] = (char)wparam;
						e->char_[1] = 0;
					}
					runtime::main_loop().schedule(boost::bind(&window::v8_process_input_event, this, e));
				}

			} break;

		case WM_KEYDOWN:
			{
				if(event_handlers_.has("keydown"))
				{
					boost::shared_ptr<input_event> e = boost::make_shared<input_event>("keydown");
					e->vk_code_ = (uint32_t)wparam;
					e->scancode_ = MapVirtualKey(e->vk_code_, MAPVK_VK_TO_VSC);
					if(isalnum(e->vk_code_))
					{
						e->charcode_ = e->vk_code_;
						e->char_[0] = (char)wparam;
						e->char_[1] = 0;
					}
					runtime::main_loop().schedule(boost::bind(&window::v8_process_input_event, this, e));
				}

			} break;

		case WM_CHAR:
			{
//				uint32_t code = wparam;
//				std::string text;
//				delegate_->text_event_2(code, text);
				if(event_handlers_.has("char"))
				{
					boost::shared_ptr<input_event> e = boost::make_shared<input_event>("keydown");
					e->charcode_ = (uint32_t)wparam;
					e->char_[0] = (char)wparam;
					e->char_[1] = 0;
					// e->scancode_ = MapVirtualKey(e->vk_code_, MAPVK_VK_TO_VSC);
					runtime::main_loop().schedule(boost::bind(&window::v8_process_input_event, this, e));
				}

			} break;


		case WM_DROPFILES:
			{
					HDROP hDrop = (HDROP) wparam;
					boost::shared_ptr<std::vector<std::string>> files = boost::make_shared<std::vector<std::string>>();
					uint32_t count = DragQueryFile(hDrop, (UINT)-1, NULL, NULL);
					for(uint32_t i = 0; i < count; i++)
					{
						char buffer[1024];
						DragQueryFileA(hDrop,i,buffer,sizeof(buffer));
						files->push_back(buffer);
					}
					DragFinish(hDrop);
					runtime::main_loop().schedule(boost::bind(&window::drag_accept_files, this, files));
					
			} break;

		case WM_PAINT:
			{
//				PAINTSTRUCT ps;

				if(splash_bitmap_.get())
				{
					HDC hdc = ::GetDC(*this);

					BITMAPFILEHEADER *bfh = (BITMAPFILEHEADER *)splash_bitmap_.get();
					
					BITMAPINFO *bmi = (BITMAPINFO *)(bfh+1);

					void *data = ((uint8_t*)bfh + bfh->bfOffBits);


					::SetDIBitsToDevice(hdc,0,0,bmi->bmiHeader.biWidth,bmi->bmiHeader.biHeight,
						0,0,
						0,bmi->bmiHeader.biHeight,
						data, bmi, DIB_RGB_COLORS);

					::ReleaseDC(*this, hdc);
				}


			} break;
	}

//	if(message_handlers_.has((uint32_t)message))
	if(message_handling_enabled_)
		runtime::main_loop().schedule(boost::bind(&window::v8_process_message, this, (uint32_t)message, (uint32_t)wparam, (uint32_t)lparam));
}


void window::use_as_splash_screen(std::string filename)
{
	if(!filename.length())
		return;

	FILE *fp = fopen(filename.c_str(), "rb");
	if(!fp)
		return;
	fseek(fp,0,SEEK_END);
	long len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	if(!len)
	{
		fclose(fp);
		return;
	}
	splash_bitmap_.reset((uint8_t*)malloc(len));
	fread(splash_bitmap_.get(),len,1,fp);
	fclose(fp);

	show_frame(false);

	BITMAPFILEHEADER *bfh = (BITMAPFILEHEADER *)splash_bitmap_.get();
	BITMAPINFO *bmi = (BITMAPINFO *)(bfh+1);
	int width  = bmi->bmiHeader.biWidth;
	int height = bmi->bmiHeader.biHeight;
	int left   = (GetDeviceCaps(GetDC(NULL), HORZRES) - width)  / 2;
	int top    = (GetDeviceCaps(GetDC(NULL), VERTRES) - height) / 2;

	::SetWindowPos(*this,HWND_TOPMOST,left,top,width,height,SWP_SHOWWINDOW);
	::InvalidateRect(*this,NULL,FALSE);
}

// void window::v8_process_event(std::string event)
// {
// 
// }

void window::v8_process_message(uint32_t message, uint32_t wparam, uint32_t lparam)
{
	v8::Handle<v8::Value> args[3] = { convert::CastToJS(message), convert::CastToJS(wparam), convert::CastToJS(lparam) };
	event_handlers_.call("message", convert::CastToJS(this)->ToObject(), 3, args);
}

void window::v8_process_event(std::string const& type)
{
	event_handlers_.call(type, convert::CastToJS(this)->ToObject(), 0, NULL);
}

void window::v8_process_input_event(boost::shared_ptr<input_event> e)
{
	Handle<Object> o = Object::New();

	o->Set(String::New("type"), String::New(e->type_.c_str()));

	Handle<Object> modifiers = Object::New();
	o->Set(String::New("modifiers"), modifiers);
	modifiers->Set(String::New("ctrl"),  convert::BoolToJS(e->mod_ctrl_));
	modifiers->Set(String::New("alt"),   convert::BoolToJS(e->mod_alt_));
	modifiers->Set(String::New("shift"), convert::BoolToJS(e->mod_shift_));
	modifiers->Set(String::New("lshift"), convert::BoolToJS(e->mod_lshift_));
	modifiers->Set(String::New("rshift"), convert::BoolToJS(e->mod_rshift_));

	//uint32_t vk_code = wparam;
	//uint32_t scancode = MapVirtualKey(vk_code, MAPVK_VK_TO_VSC);

	o->Set(String::New("vk_code"), convert::UInt32ToJS(e->vk_code_));
	o->Set(String::New("scancode"), convert::UInt32ToJS(e->scancode_));
	o->Set(String::New("charcode"), convert::UInt32ToJS(e->charcode_));
	o->Set(String::New("char"), String::New(e->char_));


	v8::Handle<v8::Value> args[1] = { o };
	event_handlers_.call(e->type_, convert::CastToJS(this)->ToObject(), 1, args);
}

void window::v8_process_resize(uint32_t w, uint32_t h)
{
	Handle<Object> o = Object::New();
	o->Set(String::New("width"), convert::UInt32ToJS(w));
	o->Set(String::New("height"), convert::UInt32ToJS(h));

	v8::Handle<v8::Value> args[1] = { o };
	event_handlers_.call("resize", convert::CastToJS(this)->ToObject(), 1, args);
}


void window::show( bool visible )
{
	ShowWindow(*this,visible ? SW_SHOW : SW_HIDE);
}

void window::switch_to_fullscreen( const video_mode& mode )
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
	SetWindowLong(*this, GWL_STYLE,   WS_POPUP);
	SetWindowLong(*this, GWL_EXSTYLE, WS_EX_APPWINDOW);

	// And resize it so that it fits the entire screen
	SetWindowPos(*this, HWND_TOP, 0, 0, mode.width, mode.height, SWP_FRAMECHANGED);
	ShowWindow(*this, SW_SHOW);

	// SetPixelFormat can fail (really ?) if window style doesn't contain these flags
	long style = GetWindowLong(*this, GWL_STYLE);
	SetWindowLong(*this, GWL_STYLE, style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	fullscreen_ = true;

	update_window_size();
}

/*
void window::toggle_fullscreen(void)
{
	if(fullscreen_)
	{
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE
		SetWindowLong(*this, GWL_STYLE,   WS_POPUP);
		SetWindowLong(*this, GWL_EXSTYLE, WS_EX_APPWINDOW);
	}
	else
	{
		// Change window style (no border, no titlebar, ...)
		SetWindowLong(*this, GWL_STYLE,   WS_POPUP);
		SetWindowLong(*this, GWL_EXSTYLE, WS_EX_APPWINDOW);

		ShowWindow(*this,SW_MAXIMIZE);
	}
}
*/

void window::show_frame(bool show)
{	
	if(show)
	{
		SetWindowLong(*this, GWL_STYLE,   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE);
//		SetWindowLong(*this, GWL_EXSTYLE, WS_EX_APPWINDOW);
	}
	else
	{
		SetWindowLong(*this, GWL_STYLE,   WS_POPUP | WS_CLIPCHILDREN | WS_VISIBLE);
//		SetWindowLong(*this, GWL_EXSTYLE, WS_EX_APPWINDOW);
//		SetWindowPos(*this, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOREPOSITION);
	}

	update_window_size();
}

void window::set_window_rect(uint32_t l, uint32_t t, uint32_t w, uint32_t h)
{
	SetWindowPos(*this, NULL, l, t, w, h, SWP_SHOWWINDOW);
	update_window_size();
}

void window::set_topmost(bool topmost)
{
	if(topmost)
		SetWindowPos(*this, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION);
	else
		SetWindowPos(*this, NULL, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREPOSITION);
}

v8::Handle<v8::Value> window::get_window_rect( v8::Arguments const& )
{
	HandleScope scope;

	Handle<Object> o = Object::New();

	RECT rc;
	GetWindowRect(*this, &rc);

	o->Set(String::New("left"), convert::UInt32ToJS(rc.left));
	o->Set(String::New("top"), convert::UInt32ToJS(rc.top));
	o->Set(String::New("width"), convert::UInt32ToJS(rc.right-rc.left));
	o->Set(String::New("height"), convert::UInt32ToJS(rc.bottom-rc.top));

	return scope.Close(o);
}

v8::Handle<v8::Value> window::get_client_rect( v8::Arguments const& )
{
	HandleScope scope;

	Handle<Object> o = Object::New();

	RECT rc;
	GetClientRect(*this, &rc);

	o->Set(String::New("left"), convert::UInt32ToJS(rc.left));
	o->Set(String::New("top"), convert::UInt32ToJS(rc.top));
	o->Set(String::New("width"), convert::UInt32ToJS(rc.right-rc.left));
	o->Set(String::New("height"), convert::UInt32ToJS(rc.bottom-rc.top));

	return scope.Close(o);
}

v8::Handle<v8::Value> window::on(std::string const& name, Handle<Value> fn)
{
	if(name == "message")
		message_handling_enabled_ = true;

	if(name == "drag_accept_files")
		runtime::main_loop().schedule(boost::bind(&window::drag_accept_files_enable_impl, this));

	window_base::on(name,fn);
	return convert::CastToJS(this);
}

v8::Handle<v8::Value> window::off(std::string const& name)
{
	window_base::off(name);
	return convert::CastToJS(this);
}

void window::load_icon_from_file( std::string const& file)
{
	runtime::main_loop().schedule(boost::bind(&window::load_icon_from_file_impl, this, file));
}

void window::load_icon_from_file_impl( std::string const& file)
{
	HANDLE hIcon = LoadImageA(NULL, file.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	if(hIcon)
		SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	hIcon = LoadImageA(NULL, file.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	if(hIcon)
		SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}

void window::drag_accept_files_enable_impl(void)
{
	drag_accept_files_enabled_ = true;
	DragAcceptFiles(*this,TRUE);
}

void window::drag_accept_files(boost::shared_ptr<std::vector<std::string>> files)
{

	Handle<Array> list = Array::New(files->size());
	for(size_t i = 0; i < files->size(); i++)
		list->Set(i, String::New((*files)[i].c_str()));

	v8::Handle<v8::Value> args[1] = { list };
	event_handlers_.call("drag_accept_files", convert::CastToJS(this)->ToObject(), 1, args);
}

} } // namespace aspect::gui

#endif // OS(WINDOWS)
