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
};
