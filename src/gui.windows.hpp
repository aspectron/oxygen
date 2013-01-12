#ifndef __GUI_WINXX_HPP__
#define __GUI_WINXX_HPP__

#if OS(WINDOWS)

#include <iostream>
#include <vector>

#include "async_queue.hpp"
#include "events.hpp"
#include "runtime.hpp"
#include "v8_main_loop.hpp"

#include "boost/enable_shared_from_this.hpp"

namespace aspect
{
	namespace gui 
	{

		class OXYGEN_API window : public shared_ptr_object<window>//, public event_handler<uint32_t> // public boost::enable_shared_from_this<window>//, public window_base
		{



			public:

				V8_DECLARE_CLASS_BINDER(window);

				// boost::shared_ptr<window> shared_from_this();

				class event_sink : public shared_ptr_object<event_sink> //boost::enable_shared_from_this<event_sink>
				{
					public:
						event_sink()
							: window_(NULL)
						{

						}
						virtual ~event_sink() { }
						
						virtual bool process_events(UINT message, WPARAM wparam, LPARAM lparam) = 0;
						void assoc(window *w) { window_ = w; }
						void unregister() 
						{ 
							if(window_) 
							{
								window_->unregister_event_sink(shared_from_this()); 
								window_ = NULL;
							}
						}

					private:

						window *window_;
				};

				class input_event
				{
					public:

						input_event(std::string const& type)
							: type_(type)
						{
							init();
						}

						void init()
						{
							vk_code_ = 0; scancode_ = charcode_ = 0;
							char_[0] = char_[1] = 0;
							mod_ctrl_ = HIWORD(GetAsyncKeyState(VK_CONTROL)) ? true : false;
							mod_alt_ =  HIWORD(GetAsyncKeyState(VK_MENU)) ? true : false;
							mod_lshift_ = HIWORD(GetAsyncKeyState(VK_SHIFT)) ? true : false;
							mod_rshift_ = HIWORD(GetAsyncKeyState(VK_LSHIFT)) ? true : false;
							mod_shift_ = mod_lshift_ || mod_rshift_;
						}

						std::string type_;
						math::vec2 cursor_;
						uint32_t vk_code_;
						uint32_t scancode_;
						uint32_t charcode_;
						char char_[2];

						bool mod_ctrl_;
						bool mod_alt_;
						bool mod_rshift_;
						bool mod_lshift_;
						bool mod_shift_;


				};

				typedef struct _creation_args
				{
					uint32_t width, height, bpp, style;
					std::string caption;
					bool frame;
					std::string splash;
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


				void process_events(void);
				void process_events_blocking(void);
				void v8_process_message(uint32_t message, uint32_t wparam, uint32_t lparam);
				void v8_process_input_event(boost::shared_ptr<input_event> e);
				void v8_process_event(std::string const&);

//				boost::shared_ptr<window> self_;

				void register_event_sink(boost::shared_ptr<event_sink>& sink)
				{
					event_sinks_.push_back(sink);
					sink->assoc(this);
				}

				void unregister_event_sink(boost::shared_ptr<event_sink>& sink)
				{
					std::vector<boost::shared_ptr<event_sink>>::iterator iter;
					for(iter = event_sinks_.begin(); iter != event_sinks_.end(); iter++)
					{
						if((*iter).get() == sink.get())
						{
							event_sinks_.erase(iter);
							return;
						}
					}

				}

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
				uint32_t width_;
				uint32_t height_;

				// ~~~

				HDC					hdc_;

				std::vector<boost::shared_ptr<event_sink>> event_sinks_;

				event_handler<uint32_t>			message_handlers_;
				event_handler<std::string>		event_handlers_;

				bool	message_handling_enabled_;	// user has installed message handler

				bool	drag_accept_files_enabled_;

				boost::scoped_ptr<uint8_t> splash_bitmap_;
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
