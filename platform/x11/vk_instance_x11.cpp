#include "vk_instance_x11.h"

#ifdef X11_ENABLED

#if defined(VULKAN_ENABLED)

void VkInstance_X11::initialize() {
	extensions.push_back("VK_KHR_xlib_surface");

	vk::ApplicationInfo app_info = {};
	app_info.pApplicationName = application_name;
	app_info.applicationVersion = application_version;
	app_info.pEngineName = engine_name;
	app_info.engineVersion = engine_version;
	app_info.apiVersion = vulkan_api_version;

	vk::InstanceCreateInfo instance_info = {};
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = nullptr;
	instance_info.enabledExtensionCount = extensions.count();
	instance_info.ppEnabledExtensionNames = extensions.data();

	if (enable_validation_layers)
		if (!check_validation_layer_support()) {
			ERR_PRINTS("Vulkan validation layers enabled but not supported");
		} else {
			instance_info.enabledLayerCount = validation_layers.count();
			instance_info.ppEnabledLayerNames = validation_layers.data();
		}

	instance = vk::createInstance(&instance_info);
	ERR_FAIL_COND(!instance);

	setup_debug_callback();
}

VkInstance_X11::VkInstance_X11(::Display *display, ::Window &window)
	: x11_window(window) {
	x11_display = display;
}

VkInstance_X11::~VkInstance_X11() {}

#endif // defined(VULKAN_ENABLED)
#endif // X11_ENABLED
