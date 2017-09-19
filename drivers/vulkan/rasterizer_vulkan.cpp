#include "rasterizer_vulkan.h"

#include "os/os.h"
#include "project_settings.h"
#include "vk_instance.h"
#include <algorithm>
#include <vulkan/vulkan.hpp>

void RasterizerVK::initialize() {
	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("Using Vulkan video driver");
	}

	SwapchainSupportDetails swapchain_support(VkInstance::get_singleton()->get_physical_device());

	vk::SurfaceFormatKHR surface_format;
	{ // choose swapchain surface format
		vector<vk::SurfaceFormatKHR> available_formats = swapchain_support.formats;

		if (available_formats.size() == 1 && available_formats[0] == vk::Format::eUndefined) {
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
	}

	vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
	{ // choose swapchain present mode
		vector<vk::PresentModeKHR> available_modes = swapchain_support.present_modes;

		for (const auto &available_mode : available_modes) {
			if (available_mode == vk::PresentModeKHR::eMailbox) {
				present_mode = available_mode;
				break;
			} else if (available_mode == vk::PresentModeKHR::eImmediate) {
				present_mode = available_mode;
				break;
			}
		}
	}

	vk::Extent2D extent = swapchain_support.capabilities.currentExtent;
	{ // choose swapchain extent
		vk::SurfaceCapabilitiesKHR capabilities = swapchain_support.capabilities;
		if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
			extent = {
				VkInstance::get_singleton()->get_window_width(),
				VkInstance::get_singleton()->get_window_height()
			};

			extent.width = std::max(capabilities.minImageExtent.width,
					std::min(capabilities.maxImageExtent.width, extent.width));
			extent.height = std::max(capabilities.minImageExtent.height,
					std::min(capabilities.maxImageExtent.height, extent.height));
		}
	}

	uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
	{ // get max image count
		vk::SurfaceCapabilitiesKHR capabilities = swapchain_support.capabilities;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}
	}

	{
		VkQueueFamilyIndices indices(VkInstance::get_singleton()->get_physical_device());
		uint32_t queue_indices[] = { (uint32_t)indices.graphics, (uint32_t)indices.present };

		vk::SwapchainCreateInfoKHR swapchain_info = {};
		swapchain_info.surface = VkInstance::get_singleton()->get_surface();
		swapchain_info.minImageCount = image_count;
		swapchain_info.imageFormat = surface_format.format;
		swapchain_info.imageColorSpace = surface_format.colorSpace;
		swapchain_info.imageExtent = extent;
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
		swapchain_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque; // can blend with other windows in the window system
		swapchain_info.presentMode = present_mode;
		swapchain_info.clipped = VK_TRUE; // don't care about pixels obscured by other windows
		swapchain_info.oldSwapchain = nullptr; // swapchain must be recreated if the window is resized
	}

	storage->initialize();
	canvas->initialize();
	scene->initialize();
}

void RasterizerVK::begin_frame() {
	uint64_t tick = OS::get_singleton()->get_ticks_usec();

	double delta = double(tick - prev_ticks) / 1000000.0;
	delta *= Engine::get_singleton()->get_time_scale();

	time_total += delta;

	if (delta == 0) {
		//to avoid hiccups
		delta = 0.001;
	}

	prev_ticks = tick;

	double time_roll_over = GLOBAL_GET("rendering/limits/time/time_rollover_secs");
	if (time_total > time_roll_over)
		time_total = 0; //roll over every day (should be customizable)

	storage->frame.time[0] = time_total;
	storage->frame.time[1] = Math::fmod(time_total, 3600);
	storage->frame.time[2] = Math::fmod(time_total, 900);
	storage->frame.time[3] = Math::fmod(time_total, 60);
	storage->frame.count++;
	storage->frame.delta = delta;

	storage->frame.prev_tick = tick;

	storage->update_dirty_resources();

	storage->info.render_final = storage->info.render;
	storage->info.render.reset();

	scene->iteration();
}

void RasterizerVK::set_current_render_target(RID p_render_target) {
	// TODO after storage is created
}

void RasterizerVK::restore_render_target() {
	// TODO after storage is created
}

void RasterizerVK::clear_render_target(const Color &p_color) {
	ERR_FAIL_COND(!storage->frame.current_rt);

	storage->frame.clear_request = true;
	storage->frame.clear_request_color = p_color;
}

void RasterizerVK::set_boot_image(Ref<Image> &p_image, const Color &p_color, bool p_scale) {
	// TODO after storage and canvas are created
}

void RasterizerVK::blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen) {
	// TODO after storage and canvas are created
}

void RasterizerVK::end_frame() {
	OS::get_singleton()->swap_buffers(); // necessary?
}

void RasterizerVK::finalize() {
	storage->finalize();
	canvas->finalize();
	// why not scene->finalize()? (from rasterizer_gles3.cpp:399)
}

Rasterizer RasterizerVK::_create_current() {
	return memnew(RasterizerVK);
}

void RasterizerVK::make_current() {
	_create_func = _create_current;
}

void RasterizerVK::register_config() {
	// Copy-Pasted from GLES3 renderer
	GLOBAL_DEF("rendering/quality/filters/use_nearest_mipmap_filter", false);
	GLOBAL_DEF("rendering/quality/filters/anisotropic_filter_level", 4.0);
	GLOBAL_DEF("rendering/limits/time/time_rollover_secs", 3600);
}

RasterizerStorage *RaterizerVK::get_storage() {
	return storage;
}

RasterizerCanvas *RasterizerVK::get_canvas() {
	return canvas;
}

RasterizerScene *RasterizerVK::get_scene() {
	return scene;
}

RasterizerVK::RasterizerVK() {
	storage = memnew(RasterizerStorageVK);
	canvas = memnew(RasterizerCanvasVK);
	scene = memnew(RasterizerSceneVK);
}

RasterizerVK::~RasterizerVK() {
	memdelete(storage);
	memdelete(canvas);
	// again (before in finalize()), no scene in GLES3 renderer, must be managed elsewhere

	VkInstance::get_singleton()->get_device().destroySwapchainKHR(swapchain);
}
