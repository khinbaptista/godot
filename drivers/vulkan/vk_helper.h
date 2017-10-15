#pragma once

#include <vulkan/vulkan.hpp>

vk::Format vk_FindSupportedFormat(
		const vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features
);

uint32_t vk_FindMemoryType(
	uint32_t type_bits,
	vk::MemoryPropertyFlags properties
);

vk::Image vk_CreateImage(
	uint32_t width, uint32_t height,
	vk::Format format,
	vk::ImageTiling tiling,
	vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags properties,

	vk::DeviceMemory& memory
);

vk::ImageView vk_CreateImageView(
	vk::Image,
	vk::Format,
	vk::ImageAspectFlags
);
