#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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
		VmaAllocation &allocation, // out
		vk::ImageCreateInfo image_info,
		VmaAllocationCreateInfo alloc_info) {

	vk::Image image;
	VmaAllocator allocator = InstanceVK::get_singleton()->get_allocator();

	auto result = vmaCreateImage(&allocator, &image_info, &alloc_info, &image, &allocation, nullptr);

	ERR_EXPLAIN("Failed to create vulkan image");
	ERR_FAIL_COND_V(result != VK_SUCCESS, vk::Image());

	return image;
}

vk::Image vk_CreateImage(
		VmaAllocation &allocation,
		uint32_t width,
		uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		VmaMemoryUsage memory_usage,
		VmaAllocationCreateFlags allocation_flags,
		uint32_t depth,
		uint32_t mipLevels,
		uint32_t arrayLayers,
		vk::SampleCountFlagBits samples,
		vk::ImageLayout initialLayout,
		vk::SharingMode sharingMode) {

	vk::ImageCreateInfo image_info;
	image_info.imageType = vk::ImageType::e2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = depth;
	image_info.mipLevels = mipLevels;
	image_info.arrayLayers = arrayLayers;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = initialLayout;
	image_info.usage = usage;
	image_info.samples = samples;
	image_info.sharingMode = sharingMode;

	VmaAllocationCreateInfo alloc_info;
	alloc_info.usage = memory_usage;
	alloc_info.flags = allocation_flags;

	return vk_CreateImage(allocation, image_info, alloc_info);
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

	vk::ImageView imageview = device.createImageView(view_info);

	ERR_EXPLAIN("Couldn't create vulkan image view");
	ERR_FAIL_COND_V(!imageview, vk::ImageView());

	return imageview;
}
