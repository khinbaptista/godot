#pragma once

#ifdef X11_ENABLED

#if defined(VULKAN_ENABLED)

#include "drivers/vulkan/instance_vk.h"
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

#endif // defined(VULKAN_ENABLED)

#endif // X11_ENABLED
