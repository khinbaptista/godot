#pragma once

#include "rasterizer_storage_vk.h"
#include "servers/visual/rasterizer.h"

class RasterizerSceneVK;

class RasterizerCanvasVK : public RasterizerCanvas {
public:
	struct CanvasItemUBO {
		float projection_matrix[16];
		float time;
		uint8_t padding[12];
	};

	RasterizerSceneVK *scene_render;

	struct Data {
		/*GLuint canvas_quad_vertices;
		GLuint canvas_quad_array;

		GLuint polygon_buffer;
		GLuint polygon_buffer_quad_arrays[4];
		GLuint polygon_buffer_pointer_array;
		GLuint polygon_index_buffer;

		GLuint particle_quad_vertices;
		GLuint particle_quad_array;*/

		uint32_t polygon_buffer_size;
	} data;

	struct State {
		CanvasItemUBO canvas_item_ubo_data;
		//GLuint canvas_item_ubo;
		bool canvas_texscreen_used;
		//CanvasShaderGLES3 canvas_shader;
		//CanvasShadowShaderGLES3 canvas_shadow_shader;

		bool using_texture_rect;
		bool using_ninepatch;

		RID current_tex;
		RID current_normal;
		RasterizerStorageVK::Texture *current_tex_ptr;

		Transform vp;

		Color canvas_item_modulate;
		Transform2D extra_matrix;
		Transform2D final_transform;
	} state;

	RasterizerStorageVK *storage;

	struct LightInternal : public RID_Data {
		struct UBOData {
			float light_matrix[16];
			float local_matrix[16];
			float shadow_matrix[16];
			float color[4];
			float shadow_color[4];
			float light_pos[2];
			float shadowpixel_size;
			float shadow_gradient;
			float light_height;
			float light_outside_alpha;
			float shadow_distance_mult;
			uint8_t padding[4];
		} ubo_data;

		//GLuint ubo;
	};

	RID_Owner<LightInternal> light_internal_owner;

	virtual RID light_internal_create();
	virtual void light_internal_update(RID p_rid, Light *p_light);
	virtual void light_internal_free(RID p_rid);

	virtual void canvas_begin();
	virtual void canvas_end();

	virtual void canvas_render_items(Item *p_item_list, int p_z, const Color &p_modulate, Light *p_light);
	virtual void canvas_debug_viewport_shadows(Light *p_lights_with_shadow);

	virtual void canvas_light_shadow_buffer_update(RID p_buffer, const Transform2D &p_light_xform, int p_light_mask, float p_near, float p_far, LightOccluderInstance *p_occluders, CameraMatrix *p_xform_cache);

	virtual void reset_canvas();

	void draw_generic_textured_rect(const Rect2 &p_rect, const Rect2 &p_src);

	void initialize();
	void finalize();

	virtual void draw_window_margins(int *black_margin, RID *black_image);

	RasterizerCanvasVK();
};
