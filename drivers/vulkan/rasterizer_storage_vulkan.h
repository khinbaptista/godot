#pragma once

#include "self_list.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual/shader_language.h"

class RasterizerCanvasVK;
class RasterizerSceneVK;

class RasterizerStorageVK : public RasterizerStorage {
public:
	RasterizerCanvasVK *canvas;
	RasterizerSceneVK *scene;

	struct Config {
		bool shrink_textures_x2;
		bool use_fast_texture_filter;
		bool use_anisotropic_filter;

		bool s3tc_supported;
		bool latc_supported;
		bool rgtc_supported;
		bool bptc_supported;
		bool etc_supported;
		bool etc2_supported;
		bool pvrtc_supported;

		bool hdr_supported;

		bool srgb_decode_supported;

		bool use_rgba_2d_shadows;

		float anisotropic_level;

		int max_texture_image_units;
		int max_texture_size;

		bool generate_wireframes;

		bool use_texture_array_environment;

		Set<String> extensions;

		bool keep_original_textures;

		bool no_depth_prepass;
		bool force_vertex_shading;
	} config;

	// SHADERS!!!!

	// APIs

	//

	struct Frame {

		RenderTarget *current_rt;

		bool clear_request;
		Color clear_request_color;
		int canvas_draw_commands;
		float time[4];
		float delta;
		uint64_t prev_tick;
		uint64_t count;

	} frame;

	void initialize();
	void finalize();

	virtual VS::InstanceType get_base_type(RID p_rid) const;
	virtual bool free(RID p_rid);

	virtual bool has_os_feature(const String &p_feature) const;

	virtual void update_dirty_resources();

	virtual void set_debug_generate_wireframes(bool p_generate);

	virtual void render_info_begin_capture();
	virtual void render_info_end_capture();
	virtual int get_captured_render_info(VS::RenderInfo p_info);

	virtual int get_render_info(VS::RenderInfo p_info);

	RasterizerStorageVK();
};
