#if defined(VULKAN_ENABLED)

#include "vk_instance.h"
#include "vulkan/vulkan.hpp"
#include <cstring>
#include <vector>

using std::vector;

VkInstance *VkInstance::singleton = nullptr;

VkInstance *VkInstance::get_singleton() {
	return singleton;
}

vk::Instance &VkInstance::vk() {
	return instance;
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

void VkInstance::setup_debug_callback() {
	// called from initialize()

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

VkInstance::VkInstance() {
	if (enable_validation_layers) {
		extensions.push_back("VK_EXT_debug_report");
	}

	ERR_FAIL_COND(singleton);
	singleton = this;
}

VkInstance::~VkInstance() {
	instance.destroyDebugReportCallbackEXT(debug_callback);
	instance.destroy();

	if (singleton == this) singleton = nullptr;
}

#endif
