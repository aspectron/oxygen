#ifndef __GUI_HPP__
#define __GUI_HPP__

namespace aspect
{
	namespace gui
	{
		class window;

		enum window_style
		{
			GWS_NONE		= 0x00000000,
			GWS_TITLEBAR	= 0x00000001,
			GWS_RESIZE		= 0x00000002,
			GWS_CLOSE		= 0x00000004,
			GWS_FULLSCREEN	= 0x00000008,
			GWS_APPWINDOW   = 0x00000020,
		};

		typedef struct _creation_args
		{
			uint32_t left, top, width, height, bpp, style;
			std::string caption;
			//bool frame;
			//bool resizeable;
			//bool fullscreen;
			// bool appwindow;
			//uint32_t flags;
			std::string splash;
		} creation_args;

		struct graphics_settings
		{
			explicit graphics_settings(unsigned int depth = 0, unsigned int stencil = 0, unsigned int antialiasing = 0) :
				depth_bits(depth),
				stencil_bits(stencil),
				antialiasing_level(antialiasing)
			{
			}

			unsigned int depth_bits;       
			unsigned int stencil_bits;     
			unsigned int antialiasing_level;
		};

		class OXYGEN_API event_sink : public shared_ptr_object<event_sink> //boost::enable_shared_from_this<event_sink>
		{
			public:

				event_sink()
					: window_(NULL)
				{

				}
				virtual ~event_sink() { }
						
				virtual bool process_events(UINT message, WPARAM wparam, LPARAM lparam) = 0;
				void assoc(window *w);
				void unregister(void);

			private:

				window *window_;
		};

		class OXYGEN_API input_event
		{
			public:

				input_event(std::string const& type)
					: type_(type)
				{
					init();
				}

				void init()
				{
#if OS(WINDOWS)
					vk_code_ = 0; scancode_ = charcode_ = 0;
					char_[0] = char_[1] = 0;
					mod_ctrl_ = HIWORD(GetAsyncKeyState(VK_CONTROL)) ? true : false;
					mod_alt_ =  HIWORD(GetAsyncKeyState(VK_MENU)) ? true : false;
					mod_lshift_ = HIWORD(GetAsyncKeyState(VK_SHIFT)) ? true : false;
					mod_rshift_ = HIWORD(GetAsyncKeyState(VK_LSHIFT)) ? true : false;
					mod_shift_ = mod_lshift_ || mod_rshift_;
#else
					// todo-linux
#endif
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

		class OXYGEN_API oxygen_thread
		{
			public:

				void hello_world(void) { aspect::trace("hello oxygen!"); }

				oxygen_thread();
				virtual ~oxygen_thread();

				static boost::shared_ptr<oxygen_thread>& global() { return global_; }

				/// Callback function to schedule in Berkelium
				typedef boost::function<void ()> callback;

				/// Schedule function call in the windows thread
				static bool schedule(callback cb);

				// Check that caller in the windows thread
				static bool is_window_thread()
				{
					return boost::this_thread::get_id() == global_->thread_.get_id();
				}

				void main();
				boost::thread thread_;

				class main_loop;
				boost::scoped_ptr<main_loop> main_loop_;

				boost::scoped_ptr<async_queue> task_queue_;

				static void start(void);
				static void stop(void);

			private:

				// static oxygen_thread *global_;
				static boost::shared_ptr<oxygen_thread>	global_;

		};



		class OXYGEN_API window_base
		{
			public:

				window_base();
				virtual ~window_base();

				void register_event_sink(boost::shared_ptr<event_sink>& sink);
				void unregister_event_sink(boost::shared_ptr<event_sink>& sink);


				void on(std::string const& name, v8::Handle<v8::Value> fn);
				void off(std::string const& name);

			private:

				boost::shared_ptr<oxygen_thread>	window_thread_;

			protected:

				std::vector<boost::shared_ptr<event_sink>> event_sinks_;
				aspect::event_handler<std::string>		event_handlers_;
				uint32_t width_, height_;
		};


} } // aspect::gui



#endif // __GUI_HPP__