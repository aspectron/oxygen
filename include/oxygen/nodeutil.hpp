#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include <v8.h>
#include <uv.h>

/// Call a function/lambda f as the Node task in its thread
template<typename F>
void call_in_node(F&& f)
{
	class node_task : uv_async_t
	{
	public:
		static void run(F&& f)
		{
			node_task* task = new node_task(std::forward<F>(f));
			uv_async_init(uv_default_loop(), task, &node_task::call);
			task->data = task;
			uv_async_send(task);
		}
	private:
		F f;

		explicit node_task(F&& f)
			: f(std::move(f))
		{
		}

		// uv_async_t implementation
		static void call(uv_async_t* handle)
		{
			node_task* task = static_cast<node_task*>(handle->data);
			task->f();
			uv_close((uv_handle_t*)handle, &node_task::destroy);
		}

		static void destroy(uv_handle_t* handle)
		{
			node_task* task = static_cast<node_task*>(handle->data);
			delete task;
		}
	};

	node_task::run(std::forward<F>(f));
}

/// Event emitter
class event_emitter
{
public:
	void on(std::string const& name, v8::Handle<v8::Function> callback)
	{
		std::unique_lock<std::mutex> lock(mtx_);

		v8::Isolate* isolate = callback->GetIsolate();
		v8::HandleScope scope(isolate);

		auto cb = v8::UniquePersistent<v8::Function>(isolate, callback);

		auto range = handlers_.equal_range(name);
		range.first = std::find_if(range.first, range.second,
			[&cb](auto const& handler) { return handler.second == cb; });
		if (range.first == range.second)
		{
			handlers_.insert(range.second, std::make_pair(name, cb.Pass()));
		}
	}

	void off(std::string const& name, v8::Handle<v8::Function> callback)
	{
		std::unique_lock<std::mutex> lock(mtx_);

		auto range = handlers_.equal_range(name);
		if (name.empty())
		{
			range.first = handlers_.begin();
			range.second = handlers_.end();
		}
		else if (!callback.IsEmpty())
		{
			v8::Isolate* isolate = callback->GetIsolate();
			v8::HandleScope scope(isolate);
			auto cb = v8::UniquePersistent<v8::Function>(isolate, callback);
			range.first = std::find_if(range.first, range.second,
				[&cb](auto const& handler) { return handler.second == cb; });
		}
		handlers_.erase(range.first, range.second);
	}

	bool has(std::string const& name) const
	{
		std::unique_lock<std::mutex> lock(mtx_);
		return handlers_.find(name) != handlers_.end();
	}

	void emit(std::string const& name,
		v8::Handle<v8::Object> recv, int argc, v8::Handle<v8::Value> argv[])
	{
		if (!recv.IsEmpty())
		{
			std::unique_lock<std::mutex> lock(mtx_);

			v8::Isolate* isolate = recv->GetIsolate();
			v8::HandleScope scope(isolate);

			auto range = handlers_.equal_range(name);
			for (auto it = range.first; it != range.second; ++it)
			{
				it->second.Get(isolate)->Call(recv, argc, argv);
			}
		}
	}

private:
	mutable std::mutex mtx_;
	std::unordered_multimap<std::string, v8::UniquePersistent<v8::Function>> handlers_;
};
