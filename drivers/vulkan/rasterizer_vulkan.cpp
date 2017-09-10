#include "rasterizer_vulkan.h"

#include <cstring>
#include <vector>

#include "vulkan/vulkan.hpp"

#include "os/os.h"
#include "project_settings.h"

using std::vector;

RasterizerStorage *RaterizerVK::get_storage() {
	return storage;
}

RasterizerCanvas *RasterizerVK::get_canvas() {
	return canvas;
}

RasterizerScene *RasterizerVK::get_scene() {
	return scene;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL _vk_debug_print(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData) {
	String output = String() +
					"VULKAN ERROR: Flags: " + vk::to_string(flags) +
					"\tObject type: " + vk::to_string(objType) +
					"\tCode: " + itos(code) +
					"\tMessage: \"" + msg + "\"";

	ERR_PRINTS(output);
}

bool CheckVulkanValidationLayerSupport() {
	vector<vk::LayerProperties> available_layers;
	available_layers = vk::enumerateInstanceLayerProperties();

	bool layers_found = false;
	for (const char *layer_name : vk_instance_layers) {
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

void CheckVulkanInstanceExtensionsSupport() {
	vector<vk::ExtensionProperties> extensions;
	extentions = vk::enumerateInstanceExtensionProperties();

	print_line("Available Vulkan instance extensions:");
	for (const auto &extension : extensions) {
		print_line("\t" + extension.extendionName);
	}
}

void RasterizerVK::SetupDebugCallback() {
	if (!enable_validation_layers) return;

	vk::DebugReportCallbackCreateInfoEXT debug_info = {};
	debug_info.flags = vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError;
	debug_info.pfnCallback = _vk_debug_print;

	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	ERR_FAIL_COND(vkCreateDebugReportCallbackEXT == nullptr);

	vkCreateDebugReportCallbackEXT(instance, &debug_info, nullptr, vk_debug_callback);
}

void RasterizerVK::initialize() {
	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("Using Vulkan video driver");
	}

	// Initialize vulkan instance
	vk::ApplicationInfo app_info = {};
	app_info.pApplicationName = "Godot Engine Editor";
	app_info.applicationVersion = VK_MAKE_VERSION(3, 0, 0);
	app_info.pEngineName = "Godot Engine";
	app_info.engineVersion = VK_MAKE_VERSION(3, 0, 0);
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	vk::InstanceCreateInfo instance_info = {};
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = nullptr;
	instance_info.enabledExtensionCount = vk_instance_extensions.count();
	instance_info.ppEnabledExtensionNames = vk_instance_extensions.data();

	if (enable_validation_layers)
		if (!CheckVulkanValidationLayerSupport()) {
			ERR_PRINTS("Vulkan validation layers enabled but not supported");
		} else {
			instance_info.enabledLayerCount = vk_instance_layers.count();
			instance_info.ppEnabledLayerNames = vk_instance_layers.data();
		}

	//CheckVulkanInstanceExtensionsSupport();

	vk_instance = vk::createInstance(&instance_info);
}

void RasterizerVK::finalize() {
	vk_instance.destroyDebugReportCallbackEXT(vk_debug_callback);
	vk_instance.destroy();
}
