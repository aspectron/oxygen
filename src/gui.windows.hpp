#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)


namespace aspect
{
	namespace gui 
	{


		class OXYGEN_API window : public shared_ptr_object<window>, public window_base //, public event_handler<uint32_t> // public boost::enable_shared_from_this<window>//, public window_base
		{



			public:

				V8_DECLARE_CLASS_BINDER(window);

				// boost::shared_ptr<window> shared_from_this();


				operator HWND () const { return hwnd_; }
				static window *pwnd_from_hwnd(HWND hwnd)
				{
#if CPU(X64)
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWLP_USERDATA) : NULL;
#else
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWL_USERDATA) : NULL;
#endif
				}

				void test_function_binding(void) { printf("Hello World!"); }


				window(const creation_args *);
				virtual ~window();
				void create_window_impl(const creation_args*);//video_mode mode, const std::string& caption, unsigned long requested_style);
				void destroy_window(void);
				void destroy_window_impl(void);
				void show_mouse_cursor(bool show);
				void process_event(UINT message, WPARAM wparam, LPARAM lparam);
				void show(bool visible);
				void switch_to_fullscreen(const video_mode& mode);
				void update_window_size(void);

// 				  void set_position(int left, int top)
// 				  {
// 					  SetWindowPos(*this, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
// 				  }

				void get_size(uint32_t *piwidth, uint32_t *piheight)
				{
					*piwidth = width_;
					*piheight = height_;
				}


				// static void process_events(void);
				// void process_events_blocking(void);
				void v8_process_message(uint32_t message, uint32_t wparam, uint32_t lparam);
				void v8_process_input_event(boost::shared_ptr<input_event> e);
				void v8_process_event(std::string const&);

//				boost::shared_ptr<window> self_;


				v8::Handle<v8::Value> on(std::string const& name, v8::Handle<v8::Value> fn);
				v8::Handle<v8::Value> off(std::string const& name);

				void show_frame(bool show);
				void set_topmost(bool topmost);

				void set_window_rect(uint32_t l, uint32_t t, uint32_t w, uint32_t h);
				v8::Handle<v8::Value> get_window_rect(v8::Arguments const&);
				v8::Handle<v8::Value> get_client_rect(v8::Arguments const&);

				void load_icon_from_file(std::string const&);
				void load_icon_from_file_impl(std::string const&);
				void drag_accept_files_enable_impl(void);
				void drag_accept_files(boost::shared_ptr<std::vector<std::string>> files);

				void use_as_splash_screen(std::string filename);

			private:

				volatile HWND hwnd_;
				volatile HWND window_created_;
				HCURSOR cursor_;
				bool	fullscreen_;
				unsigned long style_;
				bool terminating_;
				volatile bool valid_;
// 				uint32_t width_;
// 				uint32_t height_;

				// ~~~

				HDC					hdc_;

				event_handler<uint32_t>			message_handlers_;

				bool	message_handling_enabled_;	// user has installed message handler

				bool	drag_accept_files_enabled_;

				boost::scoped_ptr<uint8_t> splash_bitmap_;

		};


		void OXYGEN_API init(HINSTANCE hInstance);
		void OXYGEN_API cleanup(void);
	}
}

#define WEAK_CLASS_TYPE aspect::gui::window
#define WEAK_CLASS_NAME window
#include <v8/juice/WeakJSClassCreator-Decl.h>


#endif // OS(WINDOWS)

#endif // __WINDOW_WINXX_HPP__
