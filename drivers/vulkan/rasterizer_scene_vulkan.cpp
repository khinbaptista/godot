#include "rasterizer_scene_vulkan.h"

#include "os/os.h"
#include "project_settings.h"
#include "rasterizer_canvas_vulkan.h"
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

/* SHADOW ATLAS API */

RID RasterizerSceneVK::shadow_atlas_create() {
	ShadowAtlas *shadow_atlas = memnew(ShadowAtlas);
	/*shadow_atlas->fbo = 0;
	shadow_atlas->depth = 0;
	shadow_atlas->size = 0;
	shadow_atlas->smallest_subdiv = 0;

	for (int i = 0; i < 4; i++) {
		shadow_atlas->size_order[i] = i;
	}*/

	return shadow_atlas_owner.make_rid(shadow_atlas);
}

void RasterizerSceneVK::shadow_atlas_set_size(RID p_atlas, int p_size) {}

void RasterizerSceneVK::shadow_atlas_set_quadrant_subdivision(
		RID p_atlas, int p_quadrant, int p_subdivision) {
}

bool RasterizerSceneVK::shadow_atlas_update_light(
		RID p_atlas, RID p_light_intance, float p_coverage, uint64_t p_light_version) {
}

int RasterizerSceneVK::get_directional_light_shadow_size(RID p_light_intance) {}

void RasterizerSceneVK::set_directional_shadow_count(int p_count) {}

/* ENVIRONMENT API */

RID RasterizerSceneVK::environment_create() {
	Environment *env = memnew(Environment);
	return environment_owner.make_rid(env);
}

void RasterizerSceneVK::environment_set_background(RID p_env, VS::EnvironmentBG p_bg) {}

void RasterizerSceneVK::environment_set_sky(RID p_env, RID p_sky) {}

void RasterizerSceneVK::environment_set_sky_scale(RID p_env, float p_scale) {}

void RasterizerSceneVK::environment_set_bg_color(RID p_env, const Color &p_color) {}

void RasterizerSceneVK::environment_set_bg_energy(RID p_env, float p_energy) {}

void RasterizerSceneVK::environment_set_canvas_max_layer(RID p_env, int p_max_layer) {}

void RasterizerSceneVK::environment_set_ambient_light(
		RID p_env, const Color &p_color, float p_energy, float p_sky_contribution) {
}

void RasterizerSceneVK::environment_set_dof_blur_near(
		RID p_env, bool p_enable, float p_distance, float p_transition,
		float p_far_amount, VS::EnvironmentDOFBlurQuality p_quality) {
}

void RasterizerSceneVK::environment_set_dof_blur_far(
		RID p_env, bool p_enable, float p_distance, float p_transition,
		float p_far_amount, VS::EnvironmentDOFBlurQuality p_quality) {
}

void RasterizerSceneVK::environment_set_glow(
		RID p_env, bool p_enable, int p_level_flags, float p_intensity, float p_strength,
		float p_bloom_threshold, VS::EnvironmentGlowBlendMode p_blend_mode,
		float p_hdr_bleed_threshold, float p_hdr_bleed_scale, bool p_bicubic_upscale) {
}

void RasterizerSceneVK::environment_set_fog(
		RID p_env, bool p_enable, float p_begin, float p_end, RID p_gradient_texture) {
}

void RasterizerSceneVK::environment_set_ssr(
		RID p_env, bool p_enable, int p_max_steps, float p_fade_int,
		float p_fade_out, float p_depth_tolerance, bool p_roughness) {
}

void RasterizerSceneVK::environment_set_ssao(
		RID p_env, bool p_enable, float p_radius, float p_intensity, float p_radius2,
		float p_intensity2, float p_bias, float p_light_affect, const Color &p_color, bool p_blur) {
}

void RasterizerSceneVK::environment_set_tonemap(
		RID p_env, VS::EnvironmentToneMapper p_tone_mapper, float p_exposure,
		float p_white, bool p_auto_exposure, float p_min_luminance,
		float p_max_luminance, float p_auto_exp_speed, float p_auto_exp_scale) {
}

void RasterizerSceneVK::environment_set_adjustment(
		RID p_env, bool p_enable, float p_brightness, float p_contrast,
		float p_saturation, RID p_ramp) {
}

void RasterizerSceneVK::environment_set_fog(
		RID p_env, bool p_enable, const Color &p_color, const Color &p_sun_color, float p_sun_amount) {
}

void RasterizerSceneVK::environment_set_fog_depth(
		RID p_env, bool p_enable, float p_depth_begin,
		float p_depth_curve, bool p_transmit, float p_transmit_curve) {
}

void RasterizerSceneVK::environment_set_fog_height(
		RID p_env, bool p_enable, float p_min_height, float p_max_height, float p_height_curve) {
}

bool RasterizerSceneVK::is_environment(RID p_env) {}

VS::EnvironmentBG RasterizerSceneVK::environment_get_background(RID p_env) {
	const Environment *env = environment_owner.getornull(p_env);
	ERR_FAIL_COND_V(!env, VS::ENV_BG_MAX);

	return env->bg_mode;
}

int RasterizerSceneVK::environment_get_canvas_max_layer(RID p_env) {}

//

RID RasterizerSceneVK::light_instance_create(RID p_light) {
	LightInstance *light_instance = memnew(LightInstance);

	light_instance->last_pass = 0;
	light_instance->last_scene_pass = 0;
	light_instance->last_scene_shadow_pass = 0;

	light_instance->light = p_light;
	light_instance->light_ptr = storage->light_owner.getornull(p_light);

	ERR_FAIL_COND_V(!light_instance->light_ptr, RID());

	light_instance->self = light_instance_owner.make_rid(light_instance);

	return light_instance->self;
}

void RasterizerSceneVK::light_instance_set_transform(RID p_light_instance, const Transform &p_transform) {}

void RasterizerSceneVK::light_instance_set_shadow_transform(
		RID p_light_instance, const CameraMatrix &p_projection, const Transform &p_transform,
		float p_far, float p_split, int p_pass, float p_bias_scale) {
}

void RasterizerSceneVK::light_instance_mark_visible(RID p_light_instance) {}

RID RasterizerSceneVK::reflection_atlas_create() {
	ReflectionAtlas *reflection_atlas = memnew(ReflectionAtlas);
	reflection_atlas->subdiv = 0;
	//reflection_atlas->color = 0;
	reflection_atlas->size = 0;
	/*for (int i = 0; i < 6; i++) {
		reflection_atlas->fbo[i] = 0;
	}*/

	return reflection_atlas_owner.make_rid(reflection_atlas);
}

void RasterizerSceneVK::reflection_atlas_set_size(RID p_ref_atlas, int p_size) {}

void RasterizerSceneVK::reflection_atlas_set_subdivision(RID p_ref_atlas, int p_subdiv) {}

RID RasterizerSceneVK::reflection_probe_instance_create(RID p_probe) {
	RasterizerStorageVK::ReflectionProbe *probe = storage->reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!probe, RID());

	ReflectionProbeInstance *rpi = memnew(ReflectionProbeInstance);

	/*rpi->probe_ptr = probe;
	rpi->self = reflection_probe_instance_owner.make_rid(rpi);
	rpi->probe = p_probe;
	rpi->reflection_atlas_index = -1;
	rpi->render_step = -1;
	rpi->last_pass = 0;*/

	return rpi->self;
}

void RasterizerSceneVK::reflection_probe_instance_set_transform(RID p_instance, const Transform &p_transform) {}

void RasterizerSceneVK::reflection_probe_release_atlas_index(RID p_instance) {}

bool RasterizerSceneVK::reflection_probe_instance_needs_redraw(RID p_instance) {}

bool RasterizerSceneVK::reflection_probe_instance_has_reflection(RID p_instance) {}

bool RasterizerSceneVK::reflection_probe_instance_begin_render(RID p_instance, RID p_reflection_atlas) {}

bool RasterizerSceneVK::reflection_probe_instance_postprocess_step(RID p_instance) {}

RID RasterizerSceneVK::gi_probe_instance_create() {
	GIProbeInstance *gipi = memnew(GIProbeInstance);

	return gi_probe_instance_owner.make_rid(gipi);
}

void RasterizerSceneVK::gi_probe_instance_set_light_data(RID p_probe, RID p_base, RID p_data) {}

void RasterizerSceneVK::gi_probe_instance_set_transform_to_data(RID p_probe, const Transform &p_xform) {}

void RasterizerSceneVK::gi_probe_instance_set_bounds(RID p_probe, const Vector3 &p_bounds) {}

void RasterizerSceneVK::render_scene(
		const Transform &p_cam_transform, const CameraMatrix &p_cam_projection,
		bool p_cam_ortogonal, InstanceBase **p_cull_result, int p_cull_count,
		RID *p_light_cull_result, int p_light_cull_count, RID *p_reflection_probe_cull_result,
		int p_reflection_probe_cull_count, RID p_environment, RID p_shadow_atlas,
		RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass) {
}

void RasterizerSceneVK::render_shadow(
		RID p_light, RID p_shadow_atlas, int p_pass, InstanceBase **p_cull_result, int p_cull_count) {
}

void RasterizerSceneVK::set_scene_pass(uint64_t p_pass) {}

void RasterizerSceneVK::set_debug_draw_mode(VS::ViewportDebugDraw p_debug_draw) {}

bool RasterizerSceneVK::free(RID p_rid) {}

void RasterizerSceneVK::iteration() {}

void RasterizerSceneVK::initialize() {}

void RasterizerSceneVK::finalize() {}

RasterizerSceneVK::RasterizerSceneVK() {}
