#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.h"

vk::Format vk_FindSupportedFormat(
		const std::vector<vk::Format> &candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features);

vk::Format vk_FindDepthFormat();

uint32_t vk_FindMemoryType(
		uint32_t type_bits,
		vk::MemoryPropertyFlags properties);

vk::Image vk_CreateImage(
		VmaAllocation &allocation, // out
		vk::ImageCreateInfo,
		VmaAllocationCreateInfo);

vk::Image vk_CreateImage(
		VmaAllocation &allocation, // out
		uint32_t width,
		uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		VmaMemoryUsage memory_usage,
		VmaAllocationCreateFlags allocation_flags = (VmaAllocationCreateFlagBits)0,
		uint32_t depth = 1,
		uint32_t mipLevels = 1,
		uint32_t arrayLayers = 1,
		vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1,
		vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
		vk::SharingMode sharingMode = vk::SharingMode::eExclusive);

vk::ImageView vk_CreateImageView(
		vk::Image,
		vk::Format,
		vk::ImageAspectFlags = vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType = vk::ImageViewType::e2D,
		uint32_t levelCount = 1,
		uint32_t layerCount = 1,
		uint32_t baseMipLevel = 0,
		uint32_t baseArrayLayer = 0);
