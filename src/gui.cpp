#include "oxygen.hpp"

namespace aspect {  namespace gui {

boost::scoped_ptr<oxygen_thread> oxygen_thread::global_;

void event_sink::assoc(window_base*w)
{
	window_ = w;
}

void event_sink::unregister()
{
	if (window_)
	{
		window_->unregister_event_sink(*this);
		window_ = nullptr;
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

	/// Schedule callback call in the main loop
	bool schedule(oxygen_thread::callback cb)
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
		callbacks_.push(oxygen_thread::callback());
	}

	/// Is the main loop terminating?
	bool is_terminating() const { return is_terminating_; }

	void run()
	{
		// aspect::utils::set_thread_name("oxygen");
		while ( !is_terminating_ )
		{
			boost::posix_time::ptime const start = boost::posix_time::microsec_clock::local_time();

			// TODO - MAKE THIS BLOCKING???
			window::process_windows_events();

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
		oxygen_thread::callback cb;
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

	threads::concurrent_queue<oxygen_thread::callback> callbacks_;
	bool is_terminating_;

	boost::posix_time::time_duration& update_interval_;
};

oxygen_thread::oxygen_thread()
{
	_aspect_assert(!oxygen_thread::global_);
	if ( oxygen_thread::global_ )
	{
		throw new std::runtime_error("Only one instance of oxygen_thread object is allowed");
	}

	task_queue_.reset(new async_queue("OXYGEN",1));

	// TODO - currently main loop is rigged to check for events at a rate of 30/s
	boost::posix_time::time_duration interval(boost::posix_time::microseconds(1000000 / 30));

	main_loop_.reset(new main_loop(interval));
	thread_ = boost::thread(&main_loop::run, main_loop_.get());
}

oxygen_thread::~oxygen_thread()
{
	main_loop_->terminate();
	thread_.join();
	main_loop_.reset();
	task_queue_.reset();
}

bool oxygen_thread::schedule(callback cb)
{
	return global_->main_loop_->schedule(cb);
}

void oxygen_thread::start()
{ 
	_aspect_assert(!global_ && "oxygen_thread is already initialized"); 
	global_.reset(new oxygen_thread);
}

void oxygen_thread::stop()
{
	_aspect_assert(global_ && "oxygen_thread is not initialized"); 
	global_.reset();
}

// ------------------------------------------------------------------
void window_base::register_event_sink(event_sink& sink)
{
	event_sinks_.push_back(&sink);
	sink.assoc(this);
}

void window_base::unregister_event_sink(event_sink& sink)
{
	event_sinks_.remove(&sink);
}

#if OS(WINDOWS)
bool window_base::process_event_by_sink(UINT message, WPARAM wparam, LPARAM lparam, LRESULT& result)
{
	for (event_sinks::iterator it = event_sinks_.begin(), end =event_sinks_.end(); it != end; ++it)
	{
		if ( (*it)->process_events(message,wparam,lparam, result))
		{
			return true;
		}
	}
	return false;
}
#endif

}} // aspect::gui
