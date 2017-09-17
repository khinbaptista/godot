#pragma once

#ifdef X11_ENABLED

#if defined(VULKAN_ENABLED)

#include "drivers/vulkan/vk_instance.h"
#include "os/os.h"
#include <X11/Xlib.h>

class VkInstance_X11 : public VkInstance {
private:
	::Display *x11_display;
	::Window &x11_window;

public:
	virtual Error initialize();

	virtual int get_window_width();
	virtual int get_window_height();

	VkInstance_X11(::Display *, ::Window &);
	~VkInstance_X11();
};

#endif // defined(VULKAN_ENABLED)

#endif // X11_ENABLED
