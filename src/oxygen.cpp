#include "oxygen.hpp"
#include "library.hpp"


using namespace v8;
using namespace v8::juice;

// ----------------------------------------------------------------------

Handle<Value> get_screen_size(Arguments const&)
{
	HandleScope scope;

	Handle<Object> o = Object::New();

	o->Set(String::New("width"), convert::UInt32ToJS(GetSystemMetrics(SM_CXSCREEN)));
	o->Set(String::New("height"), convert::UInt32ToJS(GetSystemMetrics(SM_CYSCREEN)));

	return scope.Close(o);
}

// ----------------------------------------------------------------------

V8_IMPLEMENT_CLASS_BINDER(aspect::gui::window, aspect_window);

DECLARE_LIBRARY_ENTRYPOINTS(oxygen_install, oxygen_uninstall);

void oxygen_install(Handle<Object> target)
{
#if OS(WINDOWS)
	aspect::gui::init((HINSTANCE)GetModuleHandle(NULL));
#else
	aspect::gui::init();
#endif

	using namespace aspect::gui;

	ClassBinder<aspect::gui::window> *binder_window = new ClassBinder<aspect::gui::window>(target);
	V8_SET_CLASS_BINDER(aspect::gui::window, binder_window);
	(*binder_window)
		.BindMemFunc<void, &window::test_function_binding>("test_function_binding")
		.BindMemFunc<void, &window::destroy_window>("destroy")
		.BindMemFunc<Handle<Value>, string const&, Handle<Value>, &window::on>("on")
		.BindMemFunc<Handle<Value>, string const&, &window::off>("off")
		.BindMemFunc<&window::get_client_rect>("get_client_rect")
		.BindMemFunc<&window::get_window_rect>("get_window_rect")
		.BindMemFunc<void, uint32_t, uint32_t, uint32_t, uint32_t, &window::set_window_rect>("set_window_rect")
		.BindMemFunc<void, bool, &window::show_frame>("show_frame")
		.BindMemFunc<void, bool, &window::set_topmost>("set_topmost")
//		.BindMemFunc<void, std::string, &window::use_as_splash_screen>("use_as_splash_screen")
		.BindMemFunc<void, std::string const&, &window::load_icon_from_file>("load_icon_from_file")
		.Seal();

	// ---

	V8_DECLARE_FUNCTION(target, get_screen_size);


}

void oxygen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::gui::window);

	aspect::gui::cleanup();
}

namespace v8 { namespace juice {

aspect::gui::window* WeakJSClassCreatorOps<aspect::gui::window>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	using namespace aspect;
	using namespace aspect::gui;

//		video_mode mode(1280,720,32);
	if(!args.Length())
		throw std::runtime_error("Window constructor requires configuration object as an argument");

	window::creation_args ca;

	Handle<Object> o = args[0]->ToObject();

	ca.width = convert::JSToUInt32(o->Get(String::New("width")));
	if(!ca.width || ca.width > 1024*10)
		ca.width = 640;
	ca.height = convert::JSToUInt32(o->Get(String::New("height")));
	if(!ca.height || ca.height > 1024*10)
		ca.height = 480;
	ca.bpp = convert::JSToUInt32(o->Get(String::New("bpp")));
	if(!ca.bpp)
		ca.bpp = 32;

	ca.caption = convert::JSToStdString(o->Get(String::New("caption")));

	ca.style = convert::JSToUInt32(o->Get(String::New("style")));
	if(!ca.style)
		ca.style = AWS_TITLEBAR | AWS_RESIZE | AWS_CLOSE | AWS_APPWINDOW;

	Handle<Value> splash = o->Get(String::New("splash"));
	if(splash->IsString())
		ca.splash = convert::JSToStdString(splash);

	Handle<Value> frame = o->Get(String::New("frame"));
	if(!frame.IsEmpty())
		ca.frame = convert::JSToBool(frame);
	else
		ca.frame = true;

//	boost::shared_ptr<window> ptr(new aspect::gui::window(&ca));
//	ptr->self_ = ptr;
//	return ptr.get();
	return new aspect::gui::window(&ca);
}

void WeakJSClassCreatorOps<aspect::gui::window>::Dtor( aspect::gui::window *o )
{
//	o->self_.reset();
//	delete o;
	o->release();
}

}} // ::v8::juice

