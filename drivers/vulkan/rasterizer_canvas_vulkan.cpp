#include "rasterizer_canvas_vulkan.h"
#include "os/os.h"
#include "project_settings.h"
#include "rasterizer_scene_vulkan.h"
#include "servers/visual/visual_server_raster.h"

static _FORCE_INLINE_ void store_transform2d(const Transform2D &p_mtx, float *p_array) {

	p_array[0] = p_mtx.elements[0][0];
	p_array[1] = p_mtx.elements[0][1];
	p_array[2] = 0;
	p_array[3] = 0;
	p_array[4] = p_mtx.elements[1][0];
	p_array[5] = p_mtx.elements[1][1];
	p_array[6] = 0;
	p_array[7] = 0;
	p_array[8] = 0;
	p_array[9] = 0;
	p_array[10] = 1;
	p_array[11] = 0;
	p_array[12] = p_mtx.elements[2][0];
	p_array[13] = p_mtx.elements[2][1];
	p_array[14] = 0;
	p_array[15] = 1;
}

static _FORCE_INLINE_ void store_transform(const Transform &p_mtx, float *p_array) {
	p_array[0] = p_mtx.basis.elements[0][0];
	p_array[1] = p_mtx.basis.elements[1][0];
	p_array[2] = p_mtx.basis.elements[2][0];
	p_array[3] = 0;
	p_array[4] = p_mtx.basis.elements[0][1];
	p_array[5] = p_mtx.basis.elements[1][1];
	p_array[6] = p_mtx.basis.elements[2][1];
	p_array[7] = 0;
	p_array[8] = p_mtx.basis.elements[0][2];
	p_array[9] = p_mtx.basis.elements[1][2];
	p_array[10] = p_mtx.basis.elements[2][2];
	p_array[11] = 0;
	p_array[12] = p_mtx.origin.x;
	p_array[13] = p_mtx.origin.y;
	p_array[14] = p_mtx.origin.z;
	p_array[15] = 1;
}

static _FORCE_INLINE_ void store_camera(const CameraMatrix &p_mtx, float *p_array) {

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {

			p_array[i * 4 + j] = p_mtx.matrix[i][j];
		}
	}
}

RID RasterizerCanvasVK::light_internal_create() {
	LightInternal *li = memnew(LightInternal);

	// gl stuff

	return light_internal_owner.make_rid(li);
}

void RasterizerCanvasVK::light_internal_update(RID p_rid, Light *p_light) {}

void RasterizerCanvasVK::light_internal_free(RID p_rid) {}

void RasterizerCanvasVK::canvas_begin() {}

void RasterizerCanvasVK::canvas_end() {}

void RasterizerCanvasVK::canvas_render_items(Item *p_item_list, int p_z, const Color &p_modulate, Light *p_light) {}

void RasterizerCanvasVK::canvas_debug_viewport_shadows(Light *p_lights_with_shadow) {}

void RasterizerCanvasVK::canvas_light_shadow_buffer_update(
		RID p_buffer, const Transform2D &p_light_xform, int p_light_mask,
		float p_near, float p_far, LightOccluderInstance *p_occluders, CameraMatrix *p_xform_cache) {}

void RasterizerCanvasVK::reset_canvas() {}

void RasterizerCanvasVK::draw_generic_textured_rect(const Rect2 &p_rect, const Rect2 &p_src) {}

void RasterizerCanvasVK::draw_window_margins(int *black_margin, RID *black_image) {}

void RasterizerCanvasVK::initialize() {}

void RasterizerCanvasVK::finalize() {}

void RasterizerCanvasVK::RasterizerCanvasVK() {}
