#if defined(VULKAN_ENABLED)

#include "vk_instance.h"
#include <string.h>
#include <cstring>
#include <set>
#include <vector>
#include <vulkan/vulkan.hpp>

using std::vector;

VkInstance *VkInstance::singleton = nullptr;

VkInstance *VkInstance::get_singleton() {
	return singleton;
}

vk::Instance &VkInstance::vk() {
	return instance;
}

vk::Surface &VkInstance::get_surface() {
	return surface;
}

vk::PhysicalDevice &VkInstance::get_physical_device() {
	return physical_device;
}

vk::Device &VkInstance::get_device() {
	return device;
}

vk::Queue &VkInstance::get_queue_graphics() {
	return graphics_queue;
}

vk::Queue &VkInstance::get_queue_present() {
	return present_queue;
}

bool VkInstance::check_validation_layer_support() {
	vector<vk::LayerProperties> available_layers;
	available_layers = vk::enumerateInstanceLayerProperties();

	for (const char *layer_name : validation_layers) {
		bool layer_found = false;

		for (const auto &layer_properties : available_layers) {
			if (strcmp(layer_name, layer_properties.layerName) == 0) {
				layer_found = true;
				break;
			}
		}

		if (!layer_found) {
			return false;
		}
	}

	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL _vk_debug_print(
		VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj, size_t location, int32_t code,
		const char *layerPrefix, const char *msg, void *userData) {

	String output = String() +
					"VULKAN ERROR: Flags: " + vk::to_string(flags) +
					"\tObject type: " + vk::to_string(objType) +
					"\tCode: " + itos(code) +
					"\tMessage: \"" + msg + "\"";

	ERR_PRINTS(output);
}

void VkInstance::setup_debug_callback() { // called from initialize()
	if (!enable_validation_layers) return;

	vk::DebugReportCallbackCreateInfoEXT debug_info = {};
	debug_info.flags = vk::DebugReportFlagBitsEXT::eError |
					   vk::DebugReportFlagBitsEXT::eWarning |
					   vk::DebugReportFlagBitsEXT::ePerformanceWarning;
	debug_info.pfnCallback = _vk_debug_print;

	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	ERR_FAIL_COND(vkCreateDebugReportCallbackEXT == nullptr);

	vkCreateDebugReportCallbackEXT(instance, &debug_info, nullptr, vk_debug_callback);
}

VkQueueFamilyIndices VkInstance::find_queue_families(vk::PhysicalDevice device) {
	VkQueueFamilyIndices indices;
	vector<vk::QueueFamilyProperties> families = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto &family : families) {
		if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphics = i;
		}

		VkBool32 present_support = device.getSurfaceSupportKHR(i, surface);
		if (family.queueCount > 0 && present_support) {
			indices.present = i;
		}

		if (indices.is_complete()) break;

		i++;
	}

	return indices;
}

bool VkInstance::is_device_suitable(vk::PhysicalDevice device) {
	vk::PhysicalDeviceProperties properties = device.getProperties();
	vk::PhysicalDeviceFeatures features = device.getFeatures();
	auto queues = find_queue_families(device);

	return (properties.deviceType == physical_device_type) && queues.is_complete();
	// expand as desired
}

void VkInstance::pick_physical_device() {
	vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

	for (const auto &device : devices) {
		if (is_device_suitable(device)) {
			physical_device = device;
			break; // select the first suitable device
		}
	}

	if (!physical_device) {
		ERR_EXPLAIN("Couldn't find a suitable physical device");
	}
}

void VkInstance::create_logical_device() {
	auto indices = find_queue_families(physical_device);

	vector<vk::DeviceQueueCreateInfo> queue_infos;
	std::set<int> family_indices = { indices.graphics, indices.present };

	for (int family : family_indices) {
		vk::DeviceQueueCreateInfo queue_info = {};
		queue_info.queueFamilyIndex = family;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = nullptr;

		queue_infos.push_back(queue_info);
	}

	vk::DeviceCreateInfo device_info = {};
	device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
	device_info.pQueueCreateInfos = queue_infos.data();
	device_info.pEnabledFeatures = &physical_device_features;
	device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_info.ppEnabledExtensionNames = device_extensions.data();
	device_info.enabledLayerCount = enable_validation_layers ? static_cast<uint32_t>(validation_layers.size()) : 0;
	device_info.ppEnabledLayerNames = validation_layers.data();

	device = physical_device.createDevice(device_info);

	if (!device) {
		ERR_EXPLAIN("Failed to create logical device");
		return;
	}

	graphics_queue = device.getQueue(indices.graphics);
	present_queue = device.getQueue(indices.present);
}

VkInstance::VkInstance() {
	if (enable_validation_layers) {
		instance_extensions.push_back("VK_EXT_debug_report");
	}

	ERR_FAIL_COND(singleton);
	singleton = this;
}

VkInstance::~VkInstance() {
	device.destroy();
	physical_device.destroy();

	instance.destroyDebugReportCallbackEXT(debug_callback);
	instance.destroy();

	if (singleton == this) singleton = nullptr;
}

#endif
