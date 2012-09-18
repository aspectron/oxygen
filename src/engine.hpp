#include "core.hpp"
#include "v8_core.hpp"

#include "async_queue.hpp"
#include "events.hpp"

namespace aspect
{

	class OXYGEN_API engine
	{
		public:

			V8_DECLARE_CLASS_BINDER(engine);

			void hello_world(void) { aspect::trace("hello oxygen!"); }

			engine(aspect::gui::window*);
			virtual ~engine();


			/// Callback function to schedule in Berkelium
			typedef boost::function<void ()> callback;

			/// Schedule function call in Berkelium
			bool schedule(callback cb);

			/// Check that caller in Berkelium thread
//			static bool is_berkelium_thread()
//			{
// 				return boost::this_thread::get_id() == global_->engine_thread_.get_id();
// 			}

			void main();
			boost::thread thread_;

			class main_loop;
			boost::scoped_ptr<main_loop> main_loop_;

			boost::scoped_ptr<async_queue> task_queue_;

		
		private:

//			static radium *global_;

			// I AM UNABLE TO USE shared_ptr<> BECAUSE OF WHAT SEEMS TO BE DLL BOUNDARY PROBLEMS!
			boost::scoped_ptr<aspect::gui::window> window_;
			v8::Persistent<v8::Value> window_handle_;

//			aspect::v8_core::persistent_object_reference<aspect::gui::window>	window_;


	};


} // ::aspect

#define WEAK_CLASS_TYPE aspect::engine
#define WEAK_CLASS_NAME engine
#include <v8/juice/WeakJSClassCreator-Decl.h>
