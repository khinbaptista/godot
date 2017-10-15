//#ifdef VULKAN_ENABLED

#include "instance_vk.h"

#include "ustring.h"
#include "vk_helper.h"
#include <array>
#include <cstring>
#include <set>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

using std::set;
using std::string;
using std::vector;

InstanceVK *InstanceVK::singleton = nullptr;

VkQueueFamilyIndices::VkQueueFamilyIndices() {}

VkQueueFamilyIndices::VkQueueFamilyIndices(vk::PhysicalDevice device) {
	vector<vk::QueueFamilyProperties> families = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto &family : families) {
		if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics) {
			graphics = i;
		}

		vk::SurfaceKHR surface = InstanceVK::get_singleton()->get_surface();
		VkBool32 present_support = device.getSurfaceSupportKHR(i, surface);
		if (family.queueCount > 0 && present_support) {
			present = i;
		}

		if (is_complete()) break;

		i++;
	}
}

SwapchainSupportDetails::SwapchainSupportDetails(){};

SwapchainSupportDetails::SwapchainSupportDetails(vk::PhysicalDevice device) {
	vk::SurfaceKHR surface = InstanceVK::get_singleton()->get_surface();

	capabilities = device.getSurfaceCapabilitiesKHR(surface);
	formats = device.getSurfaceFormatsKHR(surface);
	present_modes = device.getSurfacePresentModesKHR(surface);
}

bool InstanceVK::check_validation_layer_support() {
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
					"VULKAN ERROR: Flags: " +
					vk::to_string(vk::DebugReportFlagBitsEXT(flags)).c_str() +

					"\tObject type: " +
					vk::to_string(vk::DebugReportObjectTypeEXT(objType)).c_str() +

					"\tCode: " + itos(code) +
					"\tMessage: \"" + msg + "\"";

	ERR_PRINTS(output);
}

void InstanceVK::setup_debug_callback() {
	if (!enable_validation_layers) return;

	vk::DebugReportCallbackCreateInfoEXT debug_info = {};
	debug_info.flags = vk::DebugReportFlagBitsEXT::eError |
					   vk::DebugReportFlagBitsEXT::eWarning |
					   vk::DebugReportFlagBitsEXT::ePerformanceWarning;
	debug_info.pfnCallback = _vk_debug_print;

	auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

	ERR_EXPLAIN("Failed to find procedure \"vkCreateDebugReportCallbackEXT\"");
	ERR_FAIL_COND(vkCreateDebugReportCallbackEXT == nullptr);

	vkCreateDebugReportCallbackEXT(
			instance, (VkDebugReportCallbackCreateInfoEXT *)&debug_info,
			nullptr, (VkDebugReportCallbackEXT *)&debug_callback);
}

void InstanceVK::destroy_debug_callback() {
	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	ERR_EXPLAIN("Failed to find procedure \"vkDestroyDebugReportCallbackEXT\"");
	ERR_FAIL_COND(vkDestroyDebugReportCallbackEXT == nullptr);

	vkDestroyDebugReportCallbackEXT(instance, (VkDebugReportCallbackEXT)debug_callback, nullptr);
}

void InstanceVK::create_instance() {
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
	instance_info.enabledExtensionCount = instance_extensions.size();
	instance_info.ppEnabledExtensionNames = instance_extensions.data();

	if (enable_validation_layers)
		if (!check_validation_layer_support()) {
			ERR_PRINTS("Vulkan validation layers enabled but not supported");
		} else {
			instance_info.enabledLayerCount = validation_layers.size();
			instance_info.ppEnabledLayerNames = validation_layers.data();
		}

	instance = vk::createInstance(instance_info);
	if (!instance) return;

	setup_debug_callback();
}

bool InstanceVK::check_device_extensions(vk::PhysicalDevice device) {
	vector<vk::ExtensionProperties> available_extensions;
	available_extensions = device.enumerateDeviceExtensionProperties();

	set<string> required_extensions(device_extensions.begin(), device_extensions.end());

	for (const auto &extension : available_extensions) {
		required_extensions.erase(extension.extensionName);
	}

	return required_extensions.empty();
}

bool InstanceVK::is_device_suitable(vk::PhysicalDevice device) {
	//vk::PhysicalDeviceProperties properties = device.getProperties();
	//vk::PhysicalDeviceFeatures features = device.getFeatures();

	VkQueueFamilyIndices queues(device);
	bool extensions_supported = check_device_extensions(device);

	bool swapchain_adequate = false;
	if (extensions_supported) {
		SwapchainSupportDetails swapchain_support(device);
		swapchain_adequate = !swapchain_support.formats.empty() &&
							 !swapchain_support.present_modes.empty();
	}

	return queues.is_complete() &&
		   extensions_supported &&
		   swapchain_adequate;
}

void InstanceVK::pick_physical_device() {
	vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

	for (const auto &device : devices) {
		if (is_device_suitable(device)) {
			physical_device = device;
			break; // select the first suitable device
		}
	}
}

void InstanceVK::create_logical_device() {
	VkQueueFamilyIndices indices(physical_device);

	vector<vk::DeviceQueueCreateInfo> queue_infos;
	std::set<int> family_indices = { indices.graphics, indices.present };

	for (int family : family_indices) {
		float priority = 1.0f;
		vk::DeviceQueueCreateInfo queue_info = {};
		queue_info.queueFamilyIndex = family;
		queue_info.queueCount = 1;
		queue_info.pQueuePriorities = &priority;

		queue_infos.push_back(queue_info);
	}

	vk::DeviceCreateInfo device_info = {};
	device_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
	device_info.pQueueCreateInfos = queue_infos.data();
	device_info.pEnabledFeatures = nullptr;
	device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	device_info.ppEnabledExtensionNames = device_extensions.data();
	device_info.enabledLayerCount = enable_validation_layers ? static_cast<uint32_t>(validation_layers.size()) : 0;
	device_info.ppEnabledLayerNames = validation_layers.data();

	device = physical_device.createDevice(device_info);

	if (!device) return;

	graphics_queue = device.getQueue(indices.graphics, 0);
	present_queue = device.getQueue(indices.present, 0);
}

void InstanceVK::create_swapchain() {
	SwapchainSupportDetails swapchain_support(physical_device);
	vk::SurfaceCapabilitiesKHR capabilities = swapchain_support.capabilities;

	vk::SurfaceFormatKHR surface_format;
	{ // choose swapchain surface format

		vector<vk::SurfaceFormatKHR> available_formats = swapchain_support.formats;

		if (available_formats.size() == 1 && available_formats[0].format == vk::Format::eUndefined) {
			surface_format = { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
		} else {
			for (const auto &available_format : available_formats) {
				if (available_format.format == vk::Format::eB8G8R8A8Unorm &&
						available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
					surface_format = available_format;
					break;
				}
			}

			if (surface_format.format == vk::Format::eUndefined) {
				surface_format = available_formats[0];
			}
		}

		swapchain_image_format = surface_format.format;
	}

	vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
	{ // choose swapchain present mode

		for (const auto &available_mode : swapchain_support.present_modes) {
			if (available_mode == preferred_present_mode) {
				present_mode = available_mode;
				break;
			}
		}
	}

	swapchain_extent = capabilities.currentExtent;
	{ // choose swapchain extent

		if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
			swapchain_extent = vk::Extent2D(get_window_width(), get_window_height());

			swapchain_extent.width = std::max(capabilities.minImageExtent.width,
					std::min(capabilities.maxImageExtent.width, swapchain_extent.width));
			swapchain_extent.height = std::max(capabilities.minImageExtent.height,
					std::min(capabilities.maxImageExtent.height, swapchain_extent.height));
		}
	}

	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1; // if allowed, +1 for triple buffering
	{ // get image count

		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}
	}

	VkQueueFamilyIndices indices(physical_device);
	uint32_t queue_indices[] = { (uint32_t)indices.graphics, (uint32_t)indices.present };

	vk::SwapchainCreateInfoKHR swapchain_info = {};
	{ // initialize swapchain_info object
		swapchain_info.surface = surface;
		swapchain_info.minImageCount = image_count;
		swapchain_info.imageFormat = surface_format.format;
		swapchain_info.imageColorSpace = surface_format.colorSpace;
		swapchain_info.imageExtent = swapchain_extent;
		swapchain_info.imageArrayLayers = 1;
		swapchain_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		if (indices.graphics != indices.present) {
			swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
			swapchain_info.queueFamilyIndexCount = 2;
			swapchain_info.pQueueFamilyIndices = queue_indices;
		} else {
			swapchain_info.imageSharingMode = vk::SharingMode::eExclusive;
			swapchain_info.queueFamilyIndexCount = 0; // optional
			swapchain_info.pQueueFamilyIndices = nullptr; // optional
		}
		swapchain_info.preTransform = swapchain_support.capabilities.currentTransform;

		// could blend with other windows in the window system (but doesn't)
		swapchain_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchain_info.presentMode = present_mode;

		// don't care about pixels obscured by other windows
		swapchain_info.clipped = VK_TRUE;

		// swapchain must be recreated if the window is resized
		swapchain_info.oldSwapchain = nullptr;
	}

	swapchain = device.createSwapchainKHR(swapchain_info);
	if (!swapchain) return;

	swapchain_images = device.getSwapchainImagesKHR(swapchain);

	swapchain_imageviews.resize(swapchain_images.size());
	for (size_t i = 0; i < swapchain_images.size(); i++) {
		swapchain_imageviews[i] = vk_CreateImageView(
				swapchain_images[i],
				swapchain_image_format,
				vk::ImageAspectFlagBits::eColor)
	}
}

static vk::Format FindDepthFormat() {
	return FindSupportedFormat(
		{
			vk::Format::eD32Sfloat,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint
		},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
}

/*inline bool has_stencil_component(vk::Format format) {
	return	format == vk::Format::eD32SfloatS8Uint ||
		format == vk::Format::eD24UnormS8Uint;
}*/

void InstanceVK::create_depth_resources() {
	vk::Format depth_format = FindDepthFormat();

	depth_image = vk_CreateImage(
			swapchain_extent.width,
			swapchain_extent.height,
			depth_format,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			depth_memory)

			depth_imageview = vk_CreateImageView(
					depth_image, depth_format, vk::ImageAspectFlagBits::eDepth);
}

void InstanceVK::create_render_pass() {
	vk::AttachmentDescription color_attachment = {};
	color_attachment.format = swapchain_image_format;
	color_attachment.samples = vk::SampleCountFlagBits::e1; // not using multisampling
	color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
	color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
	color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
	color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	color_attachment.initialLayout = vk::ImageLayout::eUndefined;
	color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentDescription depth_attachment = {};
	depth_attachment.format = FindDepthFormat();
	depth_attachment.samples = vk::SampleCountFlagBits::e1; // not using multisampling
	depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
	depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
	depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
	depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::AttachmentReference color_ref = {};
	color_ref.attachment = 0;
	color_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference depth_ref = {};
	depth_ref.attachment = 1;
	depth_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;
	subpass.pDepthStencilAttachment = &depth_ref;

	std::array<vk::AttachmentDescription, 2> attachments = {
		color_attachment, depth_attachment
	};

	vk::RenderPassCreateInfo renderpass_info = {};
	renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderpass_info.pAttachments = attachments.data();
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpass;
	//renderpass_info.dependencyCount = 1;
	//renderpass_info.pDependencies = &dependency;

	render_pass = device.createRenderPass(renderpass_info);
}

InstanceVK *InstanceVK::get_singleton() {
	return singleton;
}

vk::Instance InstanceVK::vk() {
	return instance;
}

vk::SurfaceKHR InstanceVK::get_surface() {
	return surface;
}

vk::PhysicalDevice InstanceVK::get_physical_device() {
	return physical_device;
}

vk::Device InstanceVK::get_device() {
	return device;
}

vk::Queue InstanceVK::get_queue_graphics() {
	return graphics_queue;
}

vk::Queue InstanceVK::get_queue_present() {
	return present_queue;
}

vk::SwapchainKHR InstanceVK::get_swapchain() {
	return swapchain;
}

vk::Extent2D InstanceVK::get_swapchain_extent() {
	return swapchain_extent;
}

vk::RenderPass InstanceVK::get_render_pass() {
	return render_pass;
}

InstanceVK::InstanceVK() {
	if (enable_validation_layers) {
		instance_extensions.push_back("VK_EXT_debug_report");
	}

	ERR_FAIL_COND(singleton);
	singleton = this;
}

InstanceVK::~InstanceVK() {
	device.destroyPipelineLayout(pipeline_layout);
	device.destroyRenderPass(render_pass);

	for (size_t i = 0; i < swapchain_imageviews.size(); i++) {
		device.destroyImageView(swapchain_imageviews[i]);
	}

	device.destroySwapchainKHR(swapchain);
	device.destroy();

	destroy_debug_callback();
	instance.destroy();

	if (singleton == this) singleton = nullptr;
}

//#endif
