#pragma once

#if defined(VULKAN_ENABLED)

#include "typedefs.h"
#include "vulkan/vulkan.hpp"
#include <vector>

class VkInstance {

private:
	static VkInstance *singleton;

protected:
	const uint32_t vulkan_api_version = VK_MAKE_VERSION(1, 0, 30);

	const char *application_name = "Godot Engine Editor";
	const uint32_t application_version = VK_MAKE_VERSION(3, 0, 0);

	const char *engine_name = "Godot Engine";
	const uint32_t engine_version = VK_MAKE_VERSION(3, 0, 0);

#ifdef DEBUG_ENABLED
	const bool enable_validation_layers = true;
#else
	const bool enable_validation_layers = false;
#endif

	vk::Instance instance;
	vk::DebugReportCallbackEXT debug_callback;
	vk::Surface surface;

	const std::vector<const char *> validation_layers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	std::vector<const char *> extensions = {
		"VK_KHR_surface"
	};

public:
	static VkInstance *get_singleton();
	vk::Instance &vk(); // get vulkan instance object
	vk::Surface &get_surface();

	virtual Error initialize() = 0;

	virtual void setup_debug_callback();
	bool check_validation_layer_support();

	VkInstance();
	~VkInstance();
};

#endif
