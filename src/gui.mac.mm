#include "oxygen/oxygen.hpp"
#include "oxygen/gui.mac.hpp"

@interface window_delegate : NSObject
{
	aspect::gui::window* window;
}

- (id)initWithWindow:(aspect::gui::window*)wnd;

@end


@implementation window_delegate : NSObject

- (id)initWithWindow:(aspect::gui::window *)wnd
{
	self = [super init];
	if (self != nil)
	{
		window = wnd;
	}
	return self;
}

- (BOOL)windowShouldClose:(id)sender
{
	window->handle_close();
	return YES;
}

- (void)windowDidResize:(NSNotification*)notification
{
	window->handle_resize();
}
@end


@interface application_delegate : NSObject
@end

@implementation application_delegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
//    for (window = _glfw.windowListHead;  window;  window = window->next)
//        _glfwInputWindowCloseRequest(window);

	return NSTerminateCancel;
}

- (void)applicationDidHide:(NSNotification *)notification
{
//    for (window = _glfw.windowListHead;  window;  window = window->next)
//        _glfwInputWindowVisibility(window, GL_FALSE);
}

- (void)applicationDidUnhide:(NSNotification *)notification
{
//    for (window = _glfw.windowListHead;  window;  window = window->next)
//    {
//        if ([window->ns.object isVisible])
//            _glfwInputWindowVisibility(window, GL_TRUE);
//    }
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
//    _glfwInputMonitorChange();
}

@end


@interface content_view : NSView
{
	aspect::gui::window* window;
}

- (id)initWithWindow:(aspect::gui::window *)wnd;

@end

@implementation content_view

+ (void)initialize
{
	if (self == [content_view class])
	{
	/*
		if (cursor_ == nil)
		{
			NSImage* data = [[NSImage alloc] initWithSize:NSMakeSize(16, 16)];
			cursor_ = [[NSCursor alloc] initWithImage:data hotSpot:NSZeroPoint];
			[data release];
		}
	*/
	}
}

- (id)initWithWindow:(aspect::gui::window*)wnd
{
	self = [super init];
	if (self)
	{
		window = wnd;
	}
	return self;
}

- (BOOL)isOpaque
{
	return YES;
}

- (BOOL)canBecomeKeyView
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)cursorUpdate:(NSEvent *)event
{
//	setModeCursor(window);
}

- (void)mouseDown:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)mouseDragged:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)mouseUp:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)mouseMoved:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)rightMouseDown:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)rightMouseDragged:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)rightMouseUp:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)otherMouseDown:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)otherMouseDragged:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)otherMouseUp:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)mouseExited:(NSEvent *)event
{
//    window->ns.cursorInside = GL_FALSE;
//    _glfwInputCursorEnter(window, GL_FALSE);
}

- (void)mouseEntered:(NSEvent *)event
{
//    window->ns.cursorInside = GL_TRUE;
//    _glfwInputCursorEnter(window, GL_TRUE);
}

- (void)viewDidChangeBackingProperties
{
	window->handle_backing_change();
}

- (void)keyDown:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)flagsChanged:(NSEvent *)event
{
//	window->handle_input(event);
}

- (void)keyUp:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)scrollWheel:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)resetCursorRects
{
	// This makes the cursor dissapear when the window is
	// resized or received a drag operation
	//[self discardCursorRects];
	//[self addCursorRect:[self bounds] cursor:cursor_];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	if ((NSDragOperationGeneric & [sender draggingSourceOperationMask])
		== NSDragOperationGeneric)
	{
		[self setNeedsDisplay:YES];
		return NSDragOperationGeneric;
	}

	return NSDragOperationNone;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
	[self setNeedsDisplay:YES];
	return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
/*
    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSArray* files = [pasteboard propertyListForType:NSFilenamesPboardType];

    int height;
    _glfwPlatformGetWindowSize(window, NULL, &height);
    _glfwInputCursorMotion(window,
                           [sender draggingLocation].x,
                           height - [sender draggingLocation].y);

    const int count = [files count];
    if (count)
    {
        NSEnumerator* e = [files objectEnumerator];
        char** names = calloc(count, sizeof(char*));
        int i;

        for (i = 0;  i < count;  i++)
            names[i] = strdup([[e nextObject] UTF8String]);

        _glfwInputDrop(window, count, (const char**) names);

        for (i = 0;  i < count;  i++)
            free(names[i]);
        free(names);
    }
*/
	return YES;
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
	[self setNeedsDisplay:YES];
}

@end

@interface native_window : NSWindow {}
@end

@implementation native_window

- (BOOL)canBecomeKeyWindow
{
	// Required for NSBorderlessWindowMask windows
	return YES;
}

@end

@interface application : NSApplication
@end

@implementation application

// From http://cocoadev.com/index.pl?GameKeyboardHandlingAlmost
// This works around an AppKit bug, where key up events while holding
// down the command key don't get sent to the key window.
- (void)sendEvent:(NSEvent *)event
{
	if ([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask))
		[[self keyWindow] sendEvent:event];
	else
		[super sendEvent:event];
}

@end

namespace aspect { namespace gui {

void window::init()
{
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
//	id app_delegate = [application_delegate new];
//	[NSApp setDelegate:app_delegate];
}

void window::cleanup()
{
//	[NSApp setDelegate:nil];
//	[app_delegate release];
}

window::window(v8::FunctionCallbackInfo<v8::Value> const& args)
	: window_base(runtime::instance(args.GetIsolate()))
{
	create(creation_args(args));
}

void window::create(creation_args args)
{
	style_ = args.style;
	style_mask_ = 0;
	fullscreen_ = false;

	if (style_ & GWS_APPWINDOW)
	{
		style_mask_ = NSTitledWindowMask | NSResizableWindowMask
			| NSClosableWindowMask | NSMiniaturizableWindowMask;
	}
	if (style_ & GWS_TITLEBAR) style_mask_ |= NSTitledWindowMask;
	if (style_ & GWS_RESIZE)   style_mask_ |= NSResizableWindowMask;
	if (style_ & GWS_CLOSE)    style_mask_ |= NSClosableWindowMask | NSMiniaturizableWindowMask;

	NSRect rect = NSMakeRect(args.left, args.top, args.width, args.height);
	rect = [NSWindow contentRectForFrameRect:rect styleMask:style_mask_];

	object = [[native_window alloc] initWithContentRect:rect styleMask:style_mask_
		backing:NSBackingStoreBuffered defer:YES];
	[object retain];

#if TARGET(DEBUG)
	args.caption += " (DEBUG)";
#endif
	[object setTitle:[NSString stringWithUTF8String:args.caption.c_str()]];

	view = [[content_view alloc] initWithWindow:this];
	[view retain];
	[object setContentView: view];

	delegate = [[window_delegate alloc] initWithWindow:this];
	[delegate retain];
	[object setDelegate:(id)delegate];

	[object setAcceptsMouseMovedEvents:YES];
	[object setOpaque:YES];

	if (!args.icon.empty())
	{
		load_icon_from_file(args.icon);
	}

	if (!args.splash.empty())
	{
		use_as_splash_screen(args.splash);
	}

	if (!(args.style & GWS_HIDDEN))
	{
		show(true);
	}

	handle_resize();
}

void window::destroy()
{
	[object setDelegate:nil];
	[delegate release];
	delegate = nil;

	[object setContentView:nil];
	[view release];
	view = nil;

	[object release];
	object = nil;
}

rectangle<float> window::rect() const
{
	NSRect const rect = [object frame];

	return rectangle<float>(rect.origin.x, rect.origin.y,
		rect.size.width, rect.size.height);
}

void window::set_rect(rectangle<float> const& rect)
{
	NSRect const frame_rect = NSMakeRect(rect.left, rect.top, rect.width, rect.height);
	[object setFrame:frame_rect display:YES];
}

void window::show(bool visible)
{
	if (visible)
	{
		[NSApp activateIgnoringOtherApps:YES];
		[object makeKeyAndOrderFront:nil];
	}
	else
	{
		[object orderOut:nil];
	}
}

void window::toggle_fullscreen()
{
	dispatch_sync(dispatch_get_main_queue(), ^{
		fullscreen_ = !fullscreen_;
		if (fullscreen_)
		{
			// set no border
			style_mask_ = [object styleMask];
			[object setStyleMask:NSBorderlessWindowMask];

			// set level above the menu bar
			level_ = [object level];
			[object setLevel:NSMainMenuWindowLevel + 1];

			// set window rect as the window screen rect
			NSRect screen_rect = [[object screen] frame];
			rect_ = rect();
			set_rect(rectangle<float>(screen_rect.origin.x, screen_rect.origin.y,
				screen_rect.size.width, screen_rect.size.height));
		}
		else
		{
			[object setStyleMask:style_mask_];
			[object setLevel:level_];
			set_rect(rect_);
		}
		// restore key focus after changing the window level
		[object orderOut:nil];
		[object makeKeyAndOrderFront:nil];
		[object setHidesOnDeactivate:fullscreen_];
	});
}

void window::set_focus()
{
	[object makeKeyAndOrderFront:nil];
}

void window::set_cursor(NSCursor* cursor)
{
	[cursor set];
}

void window::set_stock_cursor(cursor_id id)
{
	NSCursor* cursor = nil;
	switch (id)
	{
	case ARROW:
		cursor = [NSCursor arrowCursor];
		break;
	case INPUT:
		cursor = [NSCursor IBeamCursor];
		break;
	case HAND:
		cursor = [NSCursor pointingHandCursor];
		break;
	case CROSS:
		cursor = [NSCursor crosshairCursor];
		break;
	case MOVE:
		cursor = [NSCursor resizeUpDownCursor];
		break;
	case WAIT:
	default:
		return;
	}

	set_cursor(cursor);
}

void window::show_frame(bool show)
{
	[object setStyleMask:show? style_mask_ : NSBorderlessWindowMask];
}

void window::set_topmost(bool topmost)
{
	[object setLevel:topmost? NSFloatingWindowLevel : NSNormalWindowLevel];
}

void window::load_icon_from_file(std::string const& filename)
{
	NSString* image_filename = [NSString stringWithUTF8String:filename.c_str()];
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:image_filename];

	[NSApp setApplicationIconImage:image];
}

void window::use_as_splash_screen(std::string const& filename)
{
	NSString* image_filename = [NSString stringWithUTF8String:filename.c_str()];
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:image_filename];

	show_frame(false);

	NSRect rect = [object frame];
	NSImageView* image_view = [[NSImageView alloc] initWithFrame:rect];
	[image_view setImageScaling:NSImageScaleAxesIndependently];
	[image_view setImage:image];

	[view release];
	[object setContentView:image_view];
}

void window::run_file_dialog(v8::FunctionCallbackInfo<v8::Value> const& args)
{
	v8::Isolate* isolate = args.GetIsolate();
	v8::EscapableHandleScope scope(isolate);

	std::string type = "open";
	bool multiselect = false;
	NSString* title = nil;
	NSString* default_name = nil;
	NSURL* default_dir = nil;
	NSMutableArray* file_types = nil;

	if (args[0]->IsObject())
	{
		v8::Local<v8::Object> options = args[0]->ToObject();

		get_option(isolate, options, "type", type);
		if (type == "open")
		{
			get_option(isolate, options, "multiselect", multiselect);
		}

		std::string str;
		get_option(isolate, options, "title", str);
		if (!str.empty())
		{
			title = [NSString stringWithUTF8String:str.c_str()];
		}

		v8::Local<v8::Object> filter_obj;
		if (get_option(isolate, options, "filter", filter_obj))
		{
			v8::Local<v8::Array> filter_keys = filter_obj->GetPropertyNames();
			uint32_t const count = filter_keys->Length();

			if (count > 0)
			{
				file_types = [NSMutableArray arrayWithCapacity:count];
			}

			for (uint32_t i = 0; i < count; ++i)
			{
				//Handle<Value> val = filter_obj->Get(key);
				str = v8pp::from_v8<std::string>(isolate, filter_keys->Get(i));

				// using extensions without last dot
				std::string::size_type const dot_pos = str.find_last_of('.');
				if (dot_pos != str.npos)
				{
					str = str.substr(dot_pos + 1);
				}
				if (str == "*")
				{
					// allow all file types
					file_types = nil;
					break;
				}
				NSString* file_type = [NSString stringWithUTF8String:str.c_str()];
				[file_types addObject:file_type];
			}
		}

		//get_option(options, "defaultExt", default_ext);
		get_option(isolate, options, "defaultDir", str);
		if (!str.empty())
		{
			NSString* dir = [NSString stringWithUTF8String:str.c_str()];
			default_dir = [NSURL fileURLWithPath:dir isDirectory:YES];
		}

		get_option(isolate, options, "defaultName", str);
		if (!str.empty())
		{
			default_name = [NSString stringWithUTF8String:str.c_str()];
		}
	}

	// create panel in the main thread
	__block NSSavePanel* panel = nil;
	if (type == "open")
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			NSOpenPanel* open_panel = [NSOpenPanel openPanel];
			[open_panel setAllowsMultipleSelection:multiselect];
			[open_panel setCanChooseFiles:YES];
			[open_panel setCanChooseDirectories:NO];
			panel = open_panel;
		});
	}
	else if (type == "save")
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			panel = [NSSavePanel savePanel];
		});
	}
	else
	{
		throw std::invalid_argument("unknown dialog type " + type);
	}

	[panel setCanCreateDirectories:YES];
	[panel setCanSelectHiddenExtension:YES];
	if (title)
	{
		[panel setTitle:title];
	}
	if (default_name)
	{
		[panel setNameFieldLabel:default_name];
	}
	if (default_dir)
	{
		[panel setDirectoryURL:default_dir];
	}
	if (file_types)
	{
		[panel setAllowedFileTypes:file_types];
		[panel setAllowsOtherFileTypes:YES];
	}

	if ([panel runModal] == NSFileHandlingPanelOKButton)
	{
		if (type == "open")
		{
			NSArray* urls = [(NSOpenPanel*)panel URLs];
			if (multiselect)
			{
				uint32_t const count = [urls count];
				v8::Local<v8::Array> filenames = v8::Array::New(isolate, count);

				for (uint32_t i = 0; i < count; ++i)
				{
					NSURL* url = [urls objectAtIndex:i];
					char const* str = [url fileSystemRepresentation];
					filenames->Set(i, v8pp::to_v8(isolate, str));
				}
				args.GetReturnValue().Set(scope.Escape(filenames));
			}
			else
			{
				NSURL* url = [urls objectAtIndex:0];
				char const* str = [url fileSystemRepresentation];
				v8::Local<v8::Value> filename = v8pp::to_v8(isolate, str);
				args.GetReturnValue().Set(scope.Escape(filename));
			}
		}
		else if (type == "save")
		{
			NSURL* url = [panel URL];
			char const* str = [url fileSystemRepresentation];
			v8::Local<v8::Value> filename = v8pp::to_v8(isolate, str);
			args.GetReturnValue().Set(scope.Escape(filename));
		}
	}
}

inline rectangle<int> to_rect(NSRect const& target, NSRect const& frame)
{
	return rectangle<int>(target.origin.x, NSMaxY(frame) - NSMaxY(target),
		target.size.width, target.size.height);
}

void window::handle_input(event e)
{
	input_event const inp_e(e);
	on_input(inp_e);
}

void window::handle_resize()
{
	NSRect const rect = [view frame];

	size_.width = rect.size.width;
	size_.height = rect.size.height;

	NSRect const backing_rect = [view convertRectToBacking:rect];
	backing_size_.width = backing_rect.size.width;
	backing_size_.height = backing_rect.size.height;

	on_resize(size_);
}

void window::handle_backing_change()
{
	handle_resize();
	on_screen_change();
}

void window::handle_close()
{
	on_event("close");
}

input_event::input_event(event const& e)
{
	NSEventType const type = [e type];

	type_and_state_ = type_and_state(type, [e modifierFlags]);

	switch (type)
	{
	case NSLeftMouseDown:
	case NSLeftMouseDragged:
	case NSLeftMouseUp:
	case NSRightMouseDown:
	case NSRightMouseDragged:
	case NSRightMouseUp:
	case NSOtherMouseDown:
	case NSOtherMouseDragged:
	case NSOtherMouseUp:
	case NSScrollWheel:
	case NSMouseMoved:
		type_and_state_ |= (([e buttonNumber] + 1) << BUTTON_SHIFT) & BUTTON_MASK;
		{
			NSWindow* window = [e window];
			NSView* view = [window contentView];
			NSPoint const pt = [e locationInWindow];//[view convertPoint:[e locationInWindow] fromView:nil];
			data_.mouse.x = pt.x;
			data_.mouse.y = [view frame].size.height - pt.y;
		}
		if (type == NSScrollWheel)
		{
		    double dx, dy;

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
			if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
			{
				dx = [e scrollingDeltaX];
				dy = [e scrollingDeltaY];

				if ([e hasPreciseScrollingDeltas])
				{
					dx *= 0.1;
					dy *= 0.1;
				}
			}
			else
#endif /*MAC_OS_X_VERSION_MAX_ALLOWED*/
			{
				dx = [e deltaX];
				dy = [e deltaY];
			}

			data_.mouse.dx = dx;
			data_.mouse.dy = dy;
			repeats_ = 0;
		}
		else
		{
			data_.mouse.dx = data_.mouse.dy = 0;
			repeats_ = [e clickCount];
		}
		break;
	case NSKeyDown:
	case NSKeyUp:
		{
			data_.key.vk_code = data_.key.key_code = data_.key.scan_code = [e keyCode];
			NSString* str = [e charactersIgnoringModifiers];
			data_.key.char_code = [str length] > 0? [str characterAtIndex:0] : 0;
			repeats_ = [e isARepeat]? 1 : 0;
		}
		break;
	default:
		type_and_state_ = UNKNOWN;
		_aspect_assert(false && "unknow input event type");
		break;
	}
}

uint32_t input_event::type_and_state(int type, unsigned int modifiers)
{
	uint32_t result = 0;

	switch (type)
	{
	case NSLeftMouseDown:
	case NSRightMouseDown:
	case NSOtherMouseDown:
		result = MOUSE_DOWN;
		break;
	case NSLeftMouseUp:
	case NSRightMouseUp:
	case NSOtherMouseUp:
		result = MOUSE_UP;
		break;
	case NSScrollWheel:
		result = MOUSE_WHEEL;
		break;
	case NSMouseMoved:
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
	case NSOtherMouseDragged:
		result = MOUSE_MOVE;
		break;
	case NSKeyDown:
		result = KEY_DOWN;
		break;
	case NSKeyUp:
		result = KEY_UP;
		break;
	}

	if (modifiers & NSShiftKeyMask)     result |= SHIFT_DOWN;
	if (modifiers & NSControlKeyMask)   result |= CTRL_DOWN;
	if (modifiers & NSAlternateKeyMask) result |= ALT_DOWN;

	return result;
}

}} // aspect::gui
