#pragma once

#if defined(VULKAN_ENABLED)

#include "typedefs.h"
#include "version.h"
#include <algorithm>
#include <vector>
#include <vulkan/vulkan.hpp>

struct VkQueueFamilyIndices {
	int graphics = -1;
	int present = -1;

	VkQueueFamilyIndices();
	VkQueueFamilyIndices(vk::PhysicalDevice);

	_FORCE_INLINE_ bool is_complete() {
		return graphics >= 0 && present >= 0;
	};
}

struct SwapchainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> present_modes;

	SwapchainSupportDetails();
	SwapchainSupportDetails(vk::PhysicalDevice);
};

class VkInstance {

private:
	static VkInstance *singleton;

protected:
	const uint32_t vulkan_api_version = VK_MAKE_VERSION(1, 0, 30);

	// TODO: get from project config
	const char *application_name = "Godot Engine Editor";
	const uint32_t application_version = VK_MAKE_VERSION(3, 0, 0);

	const char *engine_name = VERSION_NAME;
	const uint32_t engine_version =
#ifdef VERSION_PATCH
			VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
#else
			VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, 0);
#endif

	/*
		eImmediate:		single buffer
		eFifo:			double buffer; only mode guaranteed to be available
		eFifoRelaxed:	double buffer, but new images replace current one (tearing)
		eMailbox:		triple buffer, new images replace the last image in queue (no tearing)
	*/
	vk::PresentModeKHR preferred_present_mode = vk::PresentModeKHR::eMailbox;

#ifdef DEBUG_ENABLED
	const bool enable_validation_layers = true;
#else
	const bool enable_validation_layers = false;
#endif

	bool is_device_suitable(vk::PhysicalDevice);
	bool check_device_extensions(vk::PhysicalDevice);

	vk::Instance instance;
	vk::DebugReportCallbackEXT debug_callback;
	vk::SurfaceKHR surface;

	vk::PhysicalDevice physical_device;
	vk::Device device;
	vk::Queue graphics_queue;
	vk::Queue present_queue;

	vk::SwapchainKHR swapchain;
	std::vector<vk::Image> swapchain_images;
	vk::Format swapchain_image_format;
	vk::Extent2D swapchain_extent;

	const std::vector<const char *> validation_layers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	std::vector<const char *> instance_extensions = {
		"VK_KHR_surface"
	};

	std::vector<const char *> device_extensions = {
		"VK_KHR_swapchain"
	};

	void create_instance();
	void pick_physical_device();
	void create_logical_device();
	void create_swapchain();

public:
	static VkInstance *get_singleton();
	vk::Instance vk(); // get vulkan instance object
	vk::SurfaceKHR get_surface();
	vk::PhysicalDevice get_physical_device();
	vk::Device get_device();
	vk::Queue get_queue_graphics();
	vk::Queue get_queue_present();
	vk::SwapchainKHR get_swapchain();

	virtual int get_window_width() = 0;
	virtual int get_window_height() = 0;
	virtual Error initialize() = 0;

	virtual void setup_debug_callback();
	bool check_validation_layer_support();

	VkInstance();
	~VkInstance();
};

#endif
