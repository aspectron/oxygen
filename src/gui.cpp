#include "oxygen.hpp"

namespace aspect {  namespace gui {

//boost::shared_ptr<oxygen_thread>	gs_oxygen_thread;
//oxygen_thread* oxygen_thread::global_ = NULL;

boost::shared_ptr<oxygen_thread>	oxygen_thread::global_;


void event_sink::assoc(window *w) 
{ 
	window_ = w; 
}

void event_sink::unregister(void) 
{ 
	if(window_) 
	{
		window_->unregister_event_sink(shared_from_this()); 
		window_ = NULL;
	}
}


class oxygen_thread::main_loop : boost::noncopyable
{
	public:

		explicit main_loop(boost::posix_time::time_duration& update_interval)
			: is_terminating_(false)
			, update_interval_(update_interval)
		{
		}

		typedef oxygen_thread::callback callback;

		/// Schedule callback call in the main loop
		bool schedule(callback cb)
		{
			_aspect_assert(cb);
			if ( cb && !is_terminating_ )
			{
				callbacks_.push(cb);
				return true;
			}
			return false;
		}

		void terminate()
		{
			is_terminating_ = true;
			callbacks_.push(callback());
		}

		/// Is the main loop terminating?
		bool is_terminating() const { return is_terminating_; }

		void run()
		{
//			aspect::utils::set_thread_name("oxygen");

			while ( !is_terminating_ )
			{
				// printf(".");

				boost::posix_time::ptime const start = boost::posix_time::microsec_clock::local_time();

	//			Berkelium::update();

				// TODO - MAKE THIS BLOCKING???
#if OS(WINDOWS)
				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
#else

				// TODO - ITERATE THROUGH ALL WINDOWS
				std::vector<boost::shared_ptr<window>> &window_list = window::get_window_list();
				std::vector<boost::shared_ptr<window>>::iterator iter;
				for(iter = window_list.begin(); iter != window_list.end(); iter++)
					(*iter)->process_events();

#endif

				execute_callbacks();

				boost::posix_time::ptime const finish = boost::posix_time::microsec_clock::local_time();

				boost::posix_time::time_duration const period = update_interval_ - (finish - start);
				// THIS HANGS???
#if OS(WINDOWS)
				Sleep(5);
#else
				boost::this_thread::sleep(period);
#endif

			}
			callbacks_.clear();
		}

	private:

		void execute_callbacks()
		{
			size_t const MAX_CALLBACKS_HANDLED = 100;
			callback cb;
			for (size_t cb_handled = 0; callbacks_.try_pop(cb) && cb_handled < MAX_CALLBACKS_HANDLED; ++cb_handled)
			{
				if ( !cb )
				{
					break;
				}
				try
				{
					cb();
				}
				catch (...)
				{
					///TODO: handle exceptions
				}
			}
		}

		threads::concurrent_queue<callback> callbacks_;
		bool is_terminating_; //TODO: std::atomic<bool>is_terminating_;

		boost::posix_time::time_duration& update_interval_;
};


oxygen_thread::oxygen_thread()
{

	_aspect_assert(!oxygen_thread::global_.get());
	if ( oxygen_thread::global_.get() )
		throw new std::runtime_error("Only one instance of oxygen_thread object is allowed");

	task_queue_.reset(new async_queue("OXYGEN",1));

	// TODO - currently main loop is rigged to check for events at a rate of 30/s
	boost::posix_time::time_duration interval(boost::posix_time::microseconds(1000000 / 30));

	main_loop_.reset(new main_loop(interval));
	thread_ = boost::thread(&oxygen_thread::main, this);
}


oxygen_thread::~oxygen_thread()
{
	main_loop_->terminate();
	thread_.join();
	main_loop_.reset();

	task_queue_.reset();

//	_aspect_assert(oxygen_thread::global_.get() == this);
}

bool oxygen_thread::schedule(callback cb)
{
	return global()->main_loop_->schedule(cb);
}

void oxygen_thread::main()
{

//	printf("WINDOWS THREAD RUNNING...\n");

	main_loop_->run();
}

void oxygen_thread::start(void) 
{ 
	_aspect_assert(!global_.get() && "oxygen_thread is already initialized"); 
	global_.reset(new oxygen_thread()); 
}

void oxygen_thread::stop(void)
{
	_aspect_assert(global_.get() && "oxygen_thread is not initialized"); 
	global_.reset();				
}


// ------------------------------------------------------------------


window_base::window_base()
: width_(0), height_(0)
{
	window_thread_ = oxygen_thread::global();
}

window_base::~window_base()
{
	window_thread_.reset();
}


void window_base::register_event_sink(boost::shared_ptr<event_sink> sink)
{
	event_sinks_.push_back(sink);
	sink->assoc((window*)this);
}

void window_base::unregister_event_sink(boost::shared_ptr<event_sink> sink)
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


void window_base::on(std::string const& name, v8::Handle<v8::Value> fn)
{
	event_handlers_.on(name,fn);
}

void window_base::off(std::string const& name)
{
	event_handlers_.off(name);
}

} }  // aspect::gui
