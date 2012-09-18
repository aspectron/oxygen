#include "oxygen.hpp"
#include "library.hpp"

using namespace v8;
using namespace v8::juice;

V8_IMPLEMENT_CLASS_BINDER(aspect::engine, aspect_engine);

DECLARE_LIBRARY_ENTRYPOINTS(oxygen_install, oxygen_uninstall);

void oxygen_install(Handle<Object> target)
{
//	HandleScope scope;

	ClassBinder<aspect::engine> *binder_engine = new ClassBinder<aspect::engine>(target);
	V8_SET_CLASS_BINDER(aspect::engine, binder_engine);
	(*binder_engine)
		.BindMemFunc<void, &aspect::engine::hello_world>("hello world!")
		.Seal();
}

void oxygen_uninstall(Handle<Object> target) 
{
	V8_DESTROY_CLASS_BINDER(aspect::engine);
}
