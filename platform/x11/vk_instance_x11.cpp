#include "vk_instance_x11.h"

#if defined(X11_ENABLED) && defined(VULKAN_ENABLED)

#include <Xlib/Xutil.h> // VisualScreenMask
#include <vulkan/vulkan.hpp>

static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
	ctxErrorOccurred = true;
	return 0;
}

static void set_class_hint(Display *p_display, Window p_window) {
	XClassHint *classHint;

	/* set the name and class hints for the window manager to use */
	classHint = XAllocClassHint();
	if (classHint) {
		classHint->res_name = (char *)"Godot_Engine";
		classHint->res_class = (char *)"Godot";
	}
	XSetClassHint(p_display, p_window, classHint);
	XFree(classHint);
}

Error VkInstance_X11::initialize() {

	instance_extensions.push_back("VK_KHR_xlib_surface");

	create_instance();
	ERR_EXPLAIN("Could not create vulkan instance");
	ERR_FAIL_COND_V(!instance, ERR_UNCONFIGURED);

	{ // create window

		// https://github.com/LunarG/VulkanSamples/blob/master/demos/vulkaninfo.c#L982
		int visual_count;

		XVisualInfo vinfo_template = {};
		vi_template.screen = DefaultScreen(display);

		XVisualInfo *vi = XGetVisualInfo(display, VisualScreenMask, &vi_template, &visual_count);
		ERR_FAIL_COND(vinfo == NULL);

		XSetWindowAttributes swa = {};
		swa.colormap = XCreateColormap(x11_display, RootWindow(x11_display, vi->screen), vi->visual, AllocNone);
		swa.border_pixel = 0;
		swa.event_mask = StructureNotifyMask;

		x11_window = XCreateWindow(
				x11_display, RootWindow(x11_display, vi->screen), 0, 0,
				OS::get_singleton()->get_video_mode().width, OS::get_singleton()->get_video_mode().height,
				vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
		ERR_FAIL_COND_V(!x11_window, ERR_UNCONFIGURED);

		set_class_hint(x11_display, x11_window);
		XMapWindow(x11_display, x11_window);

		auto x_error_handler = XSetErrorHandler(&ctxErrorHandler);
		XSync(x11_display, False);
		XSetErrorHandler(x_error_handler);

		XFree(vi);
	}

	{ // create surface

		// https://github.com/LunarG/VulkanSamples/blob/master/demos/vulkaninfo.c#L982
		vk::XlibSurfaceCreateInfoKHR surface_info = {};
		surface_info.dpy = x11_display;
		surface_info.window = &x11_window;

		surface = instance.createXlibSurfaceKHR(surface_info);
		ERR_EXPLAIN("Could not create vulkan surface");
		ERR_FAIL_COND_V(!surface, ERR_UNCONFIGURED);
	}

	pick_physical_device();
	ERR_EXPLAIN("Couldn't find a suitable physical device");
	ERR_FAIL_COND_V(!physical_device, ERR_UNCONFIGURED);

	create_logical_device();
	ERR_EXPLAIN("Failed to create logical device");
	ERR_FAIL_COND_V(!device, ERR_UNCONFIGURED);

	create_swapchain();
	ERR_EXPLAIN("Failed to create swapchain");
	ERR_FAIL_COND_V(!swapchain, ERR_UNCONFIGURED);

	return OK;
}

int VkInstance_X11::get_window_width() {
	XWindowAttributes xwa;
	XGetWindowAttributes(x11_display, x11_window, &xwa);

	return xwa.height;
}

inf VkInstance_X11::get_window_height() {
	XWindowAttributes xwa;
	XGetWindowAttributes(x11_display, x11_window, &xwa);

	return xwa.height;
}

VkInstance_X11::VkInstance_X11(::Display *display, ::Window &window)
	: x11_window(window) {
	x11_display = display;
}

VkInstance_X11::~VkInstance_X11() {}

#endif // defined(X11_ENABLED) && defined(VULKAN_ENABLED)
