#pragma once

#if defined(VULKAN_ENABLED)

#include "typedefs.h"
#include <vector>
#include <vulkan/vulkan.hpp>

struct VkQueueFamilyIndices {
	int graphics = -1;
	int present = -1;

	_FORCE_INLINE_ bool is_complete() {
		return graphics >= 0 && present >= 0;
	};
}

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

	const vk::PhysicalDeviceType physical_device_type = vk::PhysicalDeviceType::eDiscreteGpu;
	const vk::PhysicalDeviceFeatures physical_device_features = {};
	bool is_device_suitable(vk::PhysicalDevice);
	VkQueueFamilyIndices find_queue_families(vk::PhysicalDevice);

	vk::Instance instance;
	vk::DebugReportCallbackEXT debug_callback;
	vk::Surface surface;

	vk::PhysicalDevice physical_device;
	vk::Device device;

	vk::Queue graphics_queue;
	vk::Queue present_queue;

	const std::vector<const char *> validation_layers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	std::vector<const char *> instance_extensions = {
		"VK_KHR_surface"
	};

	std::vector<const char *> device_extensions = {
		"VK_KHR_swapchain"
	};

	void pick_physical_device();
	void create_logical_device();

public:
	static VkInstance *get_singleton();
	vk::Instance &vk(); // get vulkan instance object
	vk::Surface &get_surface();
	vk::PhysicalDevice &get_physical_device();
	vk::Device &get_device();
	vk::Queue &get_queue_graphics();
	vk::Queue &get_queue_present();

	virtual int get_window_width() = 0;
	virtual int get_window_height() = 0;
	virtual Error initialize() = 0;

	virtual void setup_debug_callback();
	bool check_validation_layer_support();

	VkInstance();
	~VkInstance();
};

#endif
