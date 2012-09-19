#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)

#include <iostream>
#include <vector>

#include "async_queue.hpp"
#include "events.hpp"

#include "boost/enable_shared_from_this.hpp"

namespace aspect
{
	namespace gui 
	{

		class OXYGEN_API window : public boost::enable_shared_from_this<window>//, public window_base
		{
			private:

				volatile HWND hwnd_;
				HCURSOR cursor_;
				bool	fullscreen_;
				unsigned long style_;
				bool terminating_;
				volatile bool valid_;
				uint32_t width_;
				uint32_t height_;

				// ~~~

				HDC					hdc_;



			public:

				V8_DECLARE_CLASS_BINDER(window);

				// boost::shared_ptr<window> shared_from_this();


				typedef struct _creation_args
				{
					uint32_t width, height, bpp, style;
					std::string caption;
				} creation_args;

				void test_function_binding(void) { printf("TEST FUNCTION BINDING INVOKED!\n"); }

				// boostshared_ptr<window> get_shared_ptr() { return shared_from_this(); }

				operator HWND () const { return hwnd_; }
				static window *pwnd_from_hwnd(HWND hwnd)
				{
#if CPU(X64)
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWLP_USERDATA) : NULL;
#else
					return hwnd ? (window*)GetWindowLongPtr(hwnd,GWL_USERDATA) : NULL;
#endif
				}

				window(const creation_args *);
				virtual ~window();
				void create_window_impl(const creation_args*);//video_mode mode, const std::string& caption, unsigned long requested_style);
				void destroy_window(void);
				void destroy_window_impl(void);
				void show_mouse_cursor(bool show);
				void process_event(UINT message, WPARAM wparam, LPARAM lparam);
				void show(bool visible);
				void switch_to_fullscreen(const video_mode& mode);

// 				  void set_position(int left, int top)
// 				  {
// 					  SetWindowPos(*this, NULL, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
// 				  }

				void get_size(uint32_t *piwidth, uint32_t *piheight)
				{
					*piwidth = width_;
					*piheight = height_;
				}


				void process_events(void);
				void process_events_blocking(void);

				boost::shared_ptr<window> self_;
		};

		class OXYGEN_API windows_thread
		{
			public:

				void hello_world(void) { aspect::trace("hello oxygen!"); }

				windows_thread();
				virtual ~windows_thread();

				static windows_thread *global() { return global_; }

				/// Callback function to schedule in Berkelium
				typedef boost::function<void ()> callback;

				/// Schedule function call in the windows thread
				static bool schedule(callback cb);

				// Check that caller in the windows thread
				static bool is_berkelium_thread()
				{
					return boost::this_thread::get_id() == global_->thread_.get_id();
				}

				void main();
				boost::thread thread_;

				class main_loop;
				boost::scoped_ptr<main_loop> main_loop_;

				boost::scoped_ptr<async_queue> task_queue_;


			private:

				static windows_thread *global_;

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
