#include "vk_helper.h"

#include "error_macros.h"
#include "instance_vk.h"
#include "ustring.h"

using std::vector;

vk::Format vk_FindSupportedFormat(
		const vector<vk::Format> &candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features) {

	vk::PhysicalDevice physical_device;
	physical_device = InstanceVK::get_singleton()->get_physical_device();

	for (vk::Format format : candidates) {
		vk::FormatProperties props;
		props = physical_device.getFormatProperties(format);

		if (
				tiling == vk::ImageTiling::eLinear &&
				props.optimalTilingFeatures & features) {
			return format;
		} else if (
				tiling == vk::ImageTiling::eOptimal &&
				props.optimalTilingFeatures & features) {
			return format;
		}
	}

	ERR_EXPLAIN("Failed to find supported depth format");
	ERR_FAIL_V(vk::Format::eUndefined);
}

vk::Format vk_FindDepthFormat() {
	return vk_FindSupportedFormat(
			{ vk::Format::eD32Sfloat,
					vk::Format::eD32SfloatS8Uint,
					vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

uint32_t vk_FindMemoryType(
		uint32_t type_filter,
		vk::MemoryPropertyFlags properties) {

	vk::PhysicalDevice physical_device;
	physical_device = InstanceVK::get_singleton()->get_physical_device();

	vk::PhysicalDeviceMemoryProperties memprops;
	memprops = physical_device.getMemoryProperties();

	for (uint32_t i = 0; i < memprops.memoryTypeCount; i++) {
		if (
				(type_filter & (1 << i)) &&
				memprops.memoryTypes[i].propertyFlags == properties)
			return i;
	}

	ERR_EXPLAIN("Failed to find suitable memory type");
	ERR_FAIL_V(0);
}

vk::Image vk_CreateImage(
		uint32_t width, uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::DeviceMemory &memory,
		vk::DeviceSize offset) {

	vk::Device device = InstanceVK::get_singleton()->get_device();

	vk::ImageCreateInfo image_info = {};
	image_info.imageType = vk::ImageType::e2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = vk::ImageLayout::eUndefined;
	image_info.usage = usage;
	image_info.samples = vk::SampleCountFlagBits::e1;
	image_info.sharingMode = vk::SharingMode::eExclusive;

	vk::Image image = device.createImage(image_info);

	vk::MemoryRequirements memreq;
	memreq = device.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo alloc_info = {};
	alloc_info.allocationSize = memreq.size;
	alloc_info.memoryTypeIndex = vk_FindMemoryType(memreq.memoryTypeBits, properties);

	memory = device.allocateMemory(alloc_info);
	device.bindImageMemory(image, memory, offset);

	return image;
}

vk::ImageView vk_CreateImageView(
		vk::Image image,
		vk::Format format,
		vk::ImageAspectFlags aspect_flags,
		vk::ImageViewType view_type,
		uint32_t levelCount,
		uint32_t layerCount,
		uint32_t baseMipLevel,
		uint32_t baseArrayLayer) {

	vk::Device device = InstanceVK::get_singleton()->get_device();

	vk::ImageViewCreateInfo view_info = {};
	view_info.image = image;
	view_info.viewType = view_type;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = baseMipLevel;
	view_info.subresourceRange.levelCount = levelCount;
	view_info.subresourceRange.baseArrayLayer = baseArrayLayer;
	view_info.subresourceRange.layerCount = layerCount;

	return device.createImageView(view_info);
}
