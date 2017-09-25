#pragma once

#ifdef X11_ENABLED
#ifdef VULKAN_ENABLED

/**
	@author Khin Baptista <khin.baptista@gmail.com>
*/

#include "drivers/vulkan/instance_vk.h"
#undef CursorShape // vulkan includes xlib which requires this blegh

#include "os/os.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

class InstanceVK_X11 : public InstanceVK {
private:
	::Display *x11_display;
	::Window &x11_window;

public:
	virtual Error initialize();

	virtual int get_window_width();
	virtual int get_window_height();

	InstanceVK_X11(::Display *, ::Window &);
	~InstanceVK_X11();
};

#endif // VULKAN_ENABLED
#endif // X11_ENABLED
