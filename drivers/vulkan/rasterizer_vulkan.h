#pragma once

#include "rasterizer_canvas_vulkan.h"
#include "rasterizer_scene_vulkan.h"
#include "rasterizer_storage_vulkan.h"
#include "servers/visual/rasterizer.h"

class RasterizerVK : public Rasterizer {
private:
	static Rasterizer *_create_current();

	RasterizerStorageVK *storage;
	RasterizerCanvasVK *canvas;
	RasterizerSceneVK *scene;

	uint64_t prev_ticks;
	double time_total;

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
	virtual void blit_render_target_to_screen(
			RID p_render_target,
			const Rect2 &p_screen_rect,
			int p_screen = 0);
	virtual void end_frame();
	virtual void finalize();

	static void make_current();

	static void register_config();
	RasterizerVK();
	~RasterizerVK();
};
