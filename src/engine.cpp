#include "oxygen.hpp"

using namespace v8;
using namespace v8::juice;

namespace v8 { namespace juice {

aspect::engine * WeakJSClassCreatorOps<aspect::engine>::Ctor( v8::Arguments const & args, std::string & exceptionText )
{
	if(!args.Length())
		throw std::runtime_error("oxygen::engine() requires hydrogen::window object");

	aspect::gui::window *window = convert::CastFromJS<aspect::gui::window>(args[0]);
	if(!window)
		throw std::runtime_error("oxygen::engine() - constructor argument is not a window");
//	boost::shared_ptr<aspect::gui::window> ptr(window);
//	return new aspect::engine(ptr);
//	return new aspect::engine(window->shared_from_this());
	return new aspect::engine(window);
}

void WeakJSClassCreatorOps<aspect::engine>::Dtor( aspect::engine *o )
{
	delete o;
}

}} // ::v8::juice

namespace aspect 
{

// engine *engine::global_ = NULL;

class engine::main_loop : boost::noncopyable
{
public:
	explicit main_loop(boost::posix_time::time_duration& update_interval)
		: is_terminating_(false)
		, update_interval_(update_interval)
	{
	}

	typedef engine::callback callback;

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
		aspect::utils::set_thread_name("thorium");

		while ( !is_terminating_ )
		{
			boost::posix_time::ptime const start = boost::posix_time::microsec_clock::local_time();

//			Berkelium::update();

			// TODO - UPDATE!


			execute_callbacks();

			boost::posix_time::ptime const finish = boost::posix_time::microsec_clock::local_time();

			boost::posix_time::time_duration const period = update_interval_ - (finish - start);
			boost::this_thread::sleep(period);
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


engine::engine(aspect::gui::window* target_window)
: window_(target_window)
{

	// TODO - v8 PERSISTENT HANDLE!

/*
	_aspect_assert(!engine::global_);
	if ( engine::global_ )
	{
		throw new std::runtime_error("Only one instance of engine object is allowed");
	}
	engine::global_ = this;
*/

//	task_queue_.reset(new async_queue(cfg_.task_thread_count));
	task_queue_.reset(new async_queue(1));


	boost::posix_time::time_duration interval(boost::posix_time::microseconds(1000000 / 30));

	main_loop_.reset(new main_loop(interval));
	thread_ = boost::thread(&engine::main, this);
}


engine::~engine()
{
	main_loop_->terminate();
	thread_.join();
	main_loop_.reset();

	task_queue_.reset();

//	_aspect_assert(thorium::global_ == this);
//	thorium::global_ = NULL;

// 	for (clients::iterator iter = clients_.begin(), end = clients_.end(); iter != end; ++iter)
// 	{
// 		delete *iter;
// 	}
// 
// 	istorage::cleanup();
}


void engine::main()
{
	// main thread

	// TODO - init

	printf("OXYGEN ENGINE RUNNING!\n");

	main_loop_->run();


	// TODO - shutdown
}




} // namespace aspect