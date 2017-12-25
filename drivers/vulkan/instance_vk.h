#pragma once

#include "typedefs.h"
#include "version.h"

#include <algorithm>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.h"

struct VkQueueFamilyIndices {
	int graphics = -1;
	int present = -1;

	VkQueueFamilyIndices();
	VkQueueFamilyIndices(vk::PhysicalDevice);

	_FORCE_INLINE_ bool is_complete() {
		return graphics >= 0 && present >= 0;
	}
};

struct SwapchainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> present_modes;

	SwapchainSupportDetails();
	SwapchainSupportDetails(vk::PhysicalDevice);
};

class InstanceVK {

private:
	static InstanceVK *singleton;

protected:
	const uint32_t vulkan_api_version = VK_MAKE_VERSION(1, 0, 30);

	// TODO: get from project config
	const char *application_name = "Godot Engine Editor";
	const uint32_t application_version = VK_MAKE_VERSION(3, 0, 0);

	const char *engine_name = _MKSTR(VERSION_NAME);
	const uint32_t engine_version =
#ifdef VERSION_PATCH
			VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
#else
			VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, 0);
#endif

#ifdef DEBUG_ENABLED
	const bool enable_validation_layers = true;
#else
	const bool enable_validation_layers = false;
#endif

	bool is_device_suitable(vk::PhysicalDevice);
	bool check_device_extensions(vk::PhysicalDevice);

	bool use_vsync;

	vk::Instance instance;
	vk::DebugReportCallbackEXT debug_callback;
	vk::SurfaceKHR surface;
	// @TODO: look into fullscreen surface (display surface)
	// It's platform-independent, interface directly to the display (no OS)
	// (ref: 'Vulkan Programming Guide' book, p. 139)

	VmaAllocator allocator;

	vk::PhysicalDevice physical_device;
	vk::Device device;
	vk::Queue graphics_queue;
	vk::Queue present_queue;

	vk::SwapchainKHR swapchain;
	vk::Extent2D swapchain_extent;
	vk::Format swapchain_format;
	std::vector<vk::Image> swapchain_images;
	std::vector<vk::ImageView> swapchain_imageviews;
	std::vector<vk::Framebuffer> swapchain_framebuffers;

	vk::RenderPass renderpass;

	vk::CommandPool command_pool;
	//std::vector<vk::CommandBuffer> command_buffers;

	const std::vector<const char *> validation_layers = {
		"VK_LAYER_LUNARG_api_dump",
		"VK_LAYER_LUNARG_monitor",
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
	void create_renderpass();
	void create_framebuffers();
	void create_command_pool();
	void create_command_buffers();

public:
	static InstanceVK *get_singleton();

	vk::Instance vk(); // get vulkan instance object
	vk::SurfaceKHR get_surface();

	vk::PhysicalDevice get_physical_device();
	vk::PhysicalDeviceLimits get_device_limits();
	vk::Device get_device();
	VmaAllocator get_allocator();

	vk::Queue get_queue_graphics();
	vk::Queue get_queue_present();

	vk::SwapchainKHR get_swapchain();
	vk::Extent2D get_swapchain_extent();
	vk::Format get_swapchain_format();

	//vk::RenderPass get_renderpass();
	//vk::Framebuffer get_framebuffer();

	void set_use_vsync(bool);
	bool is_using_vsync() const;

	virtual int get_window_width() = 0;
	virtual int get_window_height() = 0;
	virtual Error initialize() = 0;
	Error setup();

	virtual void setup_debug_callback();
	virtual void destroy_debug_callback();
	bool check_validation_layer_support();

	InstanceVK();
	~InstanceVK();
};
