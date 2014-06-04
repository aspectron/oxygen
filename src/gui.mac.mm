#include "oxygen.hpp"
#include "gui.mac.hpp"

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
//    setModeCursor(window);
}

- (void)mouseDown:(NSEvent *)event
{
	window->handle_input(event);
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
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
    [self mouseMoved:event];
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
    [self mouseMoved:event];
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
//    const NSRect contentRect = [window->view frame];
//    const NSRect fbRect = [window->view convertRectToBacking: contentRect];

//    _glfwInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
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

void window::create(creation_args args)
{
	style_ = args.style;
	style_mask_ = 0;

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

#if TARGET(DEBUG)
	args.caption += " (DEBUG)";
#endif
	[object setTitle:[NSString stringWithUTF8String:args.caption.c_str()]];

	view = [[content_view alloc] initWithWindow:this];
	[object setContentView: view];

	delegate = [[window_delegate alloc] initWithWindow:this];
	[object setDelegate: delegate];
	[object setAcceptsMouseMovedEvents:YES];

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
	[object orderOut:nil];
	[object setDelegate:nil];
	[delegate release];
	delegate = nil;

	[view release];
	view = nil;

	[object close];
	object = nil;
}

rectangle<int> window::rect() const
{
	NSRect const rect = [object frame];

	return rectangle<int>(rect.origin.x, rect.origin.y,
		rect.size.width, rect.size.height);
}

void window::set_rect(rectangle<int> const& rect)
{
	NSRect const frame_rect = NSMakeRect(rect.left, rect.top, rect.width, rect.height);
	[object setFrame:frame_rect];
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
//	[object toggleFullScreen:nil];
}

void window::set_focus()
{
	[object makeKeyAndOrderFront:nil];
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
	NSButton* icon = [object standardWindowButton:NSWindowDocumentIconButton];

	[icon setImage:image];

	[image autorelease];
	[image_filename release];
}

void window::use_as_splash_screen(std::string const& filename)
{
	NSString* image_filename = [NSString stringWithUTF8String:filename.c_str()];
	NSImage* image = [[NSImage alloc] initWithContentsOfFile:image_filename];

	show_frame(false);

	NSRect rect = [object frame];
	view = [[NSImageView alloc] initWithFrame:rect];
	[view setImageScaling:NSImageScaleAxesIndependently];
	[view setImage:image];
	[object setContentView:view];

	[image autorelease];
	[image_filename release];
}

void window::handle_input(event const& e)
{
	input_event const inp_e(e);
	on_input(inp_e);
}

void window::handle_resize()
{
	id view = [object contentView];
	if (!view) view = object;

    NSRect const rect = [view frame];
	size_.width = rect.size.width;
	size_.height = rect.size.height;

	on_resize(size_);
}

void window::handle_close()
{
	if (style_ & GWS_APPWINDOW)
	{
		runtime::terminate();
	}
	on_event("close");
}

input_event::input_event(event const& e)
{
	NSEventType const type = [e type];

	type_and_state_ = type_and_state(type, [e modifierFlags]);

	switch (type)
	{
	case NSLeftMouseDown:
	case NSLeftMouseUp:
	case NSRightMouseDown:
	case NSRightMouseUp:
	case NSOtherMouseDown:
	case NSOtherMouseUp:
	case NSScrollWheel:
	case NSMouseMoved:
		type_and_state_ |= ([e buttonNumber] << BUTTON_SHIFT) & BUTTON_MASK;
		data_.mouse.x = [e locationInWindow].x;
		data_.mouse.y = [e locationInWindow].y;
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
		data_.key.vk_code = data_.key.key_code = data_.key.scan_code = [e keyCode];
		data_.key.char_code = [[e charactersIgnoringModifiers] characterAtIndex:0];
		repeats_ = [e isARepeat]? 1 : 0;
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