
#ifndef RASTERIZERVK_H
#define RASTERIZERVK_H

#include <vector>

#include "vulkan/vulkan.hpp"

#include "rasterizer_canvas_vulkan.h"
#include "rasterizer_scene_vulkan.h"
#include "rasterizer_storage_vulkan.h"
#include "servers/visual/rasterizer.h"

class RasterizerVK : public Rasterizer {

	static Rasterizer *_create_current();

	RasterizerStorageVK *storage;
	RasterizerCanvasVK *canvas;
	RasterizerSceneVK *scene;

	uint64_t prev_ticks;
	double time_total;

	/* VULKAN STUFF */

#ifdef NDEBUG
	const bool enable_validation_layers = false;
#else
	const bool enable_validation_layers = true;
#endif

	vk::Instance vk_instance;
	vk::DebugReportCallbackEXT vk_debug_callback;

	const std::vector<const char *> vk_instance_layers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	std::vector<const char *> vk_instance_extensions = {
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
		// THIS IS NOT PLATFORM-INDEPENDENT == I'll have to create something like the gl_context class for the vulkan instance
	};

	void SetupDebugCallback();

public:
	virtual RasterizerStorage *get_storage();
	virtual RasterizerCanvas *get_canvas();
	virtual RasterizerScene *get_scene();

	virtual void set_boot_image(
			const Ref<Image> &p_image,
			const Color &p_color,
			bool p_scale);

	virtual void initialize();
	virtual void begin_frame();
	virtual void set_current_render_target(RID p_render_target);
	virtual void restore_render_target();
	virtual void clear_render_target(const Color &p_color);
	virtual void blit_render_target_to_screen(RID p_render_target,
			const Rect2 &p_screen_rect,
			int p_screen = 0);
	virtual void end_frame();
	virtual void finalize();

	static void make_current();

	static void register_config();

	RasterizerVK();
	~RasterizerVK();
};

#endif //RASTERIZERVK_H
