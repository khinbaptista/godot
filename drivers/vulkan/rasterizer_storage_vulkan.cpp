#include "rasterizer_storage_vulkan.h"

RID RasterizerStorageVK::texture_create() {}
void RasterizerStorageVK::texture_allocate(RID p_texture, int p_width, int p_height, Image::Format p_format, uint32_t p_flags = VS::TEXTURE_FLAGS_DEFAULT) {}
void RasterizerStorageVK::texture_set_data(RID p_texture, const Ref<Image> &p_image, VS::CubeMapSide p_cube_side = VS::CUBEMAP_LEFT) {}
Ref<Image> RasterizerStorageVK::texture_get_data(RID p_texture, VS::CubeMapSide p_cube_side = VS::CUBEMAP_LEFT) {}
void RasterizerStorageVK::texture_set_flags(RID p_texture, uint32_t p_flags) {}
uint32_t RasterizerStorageVK::texture_get_flags(RID p_texture) {}
Image::Format RasterizerStorageVK::texture_get_format(RID p_texture) {}
uint32_t RasterizerStorageVK::texture_get_texid(RID p_texture) {}
uint32_t RasterizerStorageVK::texture_get_width(RID p_texture) {}
uint32_t RasterizerStorageVK::texture_get_height(RID p_texture) {}
void RasterizerStorageVK::texture_set_size_override(RID p_texture, int p_width, int p_height) {}

void RasterizerStorageVK::texture_set_path(RID p_texture, const String &p_path) {}
String RasterizerStorageVK::texture_get_path(RID p_texture) {}

void RasterizerStorageVK::texture_set_shrink_all_x2_on_set_data(bool p_enable) {}

void RasterizerStorageVK::texture_debug_usage(List<VS::TextureInfo> *r_info) {}

RID RasterizerStorageVK::texture_create_radiance_cubemap(RID p_source, int p_resolution = -1) {}

void RasterizerStorageVK::texture_set_detect_3d_callback(RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata) {}
void RasterizerStorageVK::texture_set_detect_srgb_callback(RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata) {}
void RasterizerStorageVK::texture_set_detect_normal_callback(RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata) {}

void RasterizerStorageVK::textures_keep_original(bool p_enable) {}

/* SKY API */

RID RasterizerStorageVK::sky_create() {}
void RasterizerStorageVK::sky_set_texture(RID p_sky, RID p_cube_map, int p_radiance_size) {}

/* SHADER API */

RID RasterizerStorageVK::shader_create() {}

void RasterizerStorageVK::shader_set_code(RID p_shader, const String &p_code) {}
String RasterizerStorageVK::shader_get_code(RID p_shader) {}
void RasterizerStorageVK::shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) {}

void RasterizerStorageVK::shader_set_default_texture_param(RID p_shader, const StringName &p_name, RID p_texture) {}
RID RasterizerStorageVK::shader_get_default_texture_param(RID p_shader, const StringName &p_name) {}

/* COMMON MATERIAL API */

RID RasterizerStorageVK::material_create() {}

void RasterizerStorageVK::material_set_render_priority(RID p_material, int priority) {}
void RasterizerStorageVK::material_set_shader(RID p_shader_material, RID p_shader) {}
RID RasterizerStorageVK::material_get_shader(RID p_shader_material) {}

void RasterizerStorageVK::material_set_param(RID p_material, const StringName &p_param, const Variant &p_value) {}
Variant RasterizerStorageVK::material_get_param(RID p_material, const StringName &p_param) {}

void RasterizerStorageVK::material_set_line_width(RID p_material, float p_width) {}

void RasterizerStorageVK::material_set_next_pass(RID p_material, RID p_next_material) {}

bool RasterizerStorageVK::material_is_animated(RID p_material) {}
bool RasterizerStorageVK::material_casts_shadows(RID p_material) {}

void RasterizerStorageVK::material_add_instance_owner(RID p_material, RasterizerScene::InstanceBase *p_instance) {}
void RasterizerStorageVK::material_remove_instance_owner(RID p_material, RasterizerScene::InstanceBase *p_instance) {}

/* MESH API */

RID RasterizerStorageVK::mesh_create() {}

void RasterizerStorageVK::mesh_add_surface(RID p_mesh, uint32_t p_format, VS::PrimitiveType p_primitive, const PoolVector<uint8_t> &p_array, int p_vertex_count, const PoolVector<uint8_t> &p_index_array, int p_index_count, const Rect3 &p_aabb, const Vector<PoolVector<uint8_t> > &p_blend_shapes = Vector<PoolVector<uint8_t> >(), const Vector<Rect3> &p_bone_aabbs = Vector<Rect3>()) {}

void RasterizerStorageVK::mesh_set_blend_shape_count(RID p_mesh, int p_amount) {}
int RasterizerStorageVK::mesh_get_blend_shape_count(RID p_mesh) {}

void RasterizerStorageVK::mesh_set_blend_shape_mode(RID p_mesh, VS::BlendShapeMode p_mode) {}
VS::BlendShapeMode RasterizerStorageVK::mesh_get_blend_shape_mode(RID p_mesh) {}

void RasterizerStorageVK::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material) {}
RID RasterizerStorageVK::mesh_surface_get_material(RID p_mesh, int p_surface) {}

int RasterizerStorageVK::mesh_surface_get_array_len(RID p_mesh, int p_surface) {}
int RasterizerStorageVK::mesh_surface_get_array_index_len(RID p_mesh, int p_surface) {}

PoolVector<uint8_t> RasterizerStorageVK::mesh_surface_get_array(RID p_mesh, int p_surface) {}
PoolVector<uint8_t> RasterizerStorageVK::mesh_surface_get_index_array(RID p_mesh, int p_surface) {}

uint32_t RasterizerStorageVK::mesh_surface_get_format(RID p_mesh, int p_surface) {}
VS::PrimitiveType RasterizerStorageVK::mesh_surface_get_primitive_type(RID p_mesh, int p_surface) {}

Rect3 RasterizerStorageVK::mesh_surface_get_aabb(RID p_mesh, int p_surface) {}
Vector<PoolVector<uint8_t> > RasterizerStorageVK::mesh_surface_get_blend_shapes(RID p_mesh, int p_surface) {}
Vector<Rect3> RasterizerStorageVK::mesh_surface_get_skeleton_aabb(RID p_mesh, int p_surface) {}

void RasterizerStorageVK::mesh_remove_surface(RID p_mesh, int p_index) {}
int RasterizerStorageVK::mesh_get_surface_count(RID p_mesh) {}

void RasterizerStorageVK::mesh_set_custom_aabb(RID p_mesh, const Rect3 &p_aabb) {}
Rect3 RasterizerStorageVK::mesh_get_custom_aabb(RID p_mesh) {}

Rect3 RasterizerStorageVK::mesh_get_aabb(RID p_mesh, RID p_skeleton) {}
void RasterizerStorageVK::mesh_clear(RID p_mesh) {}

/* MULTIMESH API */

RID RasterizerStorageVK::multimesh_create() {}

void RasterizerStorageVK::multimesh_allocate(RID p_multimesh, int p_instances, VS::MultimeshTransformFormat p_transform_format, VS::MultimeshColorFormat p_color_format) {}
int RasterizerStorageVK::multimesh_get_instance_count(RID p_multimesh) {}

void RasterizerStorageVK::multimesh_set_mesh(RID p_multimesh, RID p_mesh) {}
void RasterizerStorageVK::multimesh_instance_set_transform(RID p_multimesh, int p_index, const Transform &p_transform) {}
void RasterizerStorageVK::multimesh_instance_set_transform_2d(RID p_multimesh, int p_index, const Transform2D &p_transform) {}
void RasterizerStorageVK::multimesh_instance_set_color(RID p_multimesh, int p_index, const Color &p_color) {}

RID RasterizerStorageVK::multimesh_get_mesh(RID p_multimesh) {}

Transform RasterizerStorageVK::multimesh_instance_get_transform(RID p_multimesh, int p_index) {}
Transform2D RasterizerStorageVK::multimesh_instance_get_transform_2d(RID p_multimesh, int p_index) {}
Color RasterizerStorageVK::multimesh_instance_get_color(RID p_multimesh, int p_index) {}

void RasterizerStorageVK::multimesh_set_visible_instances(RID p_multimesh, int p_visible) {}
int RasterizerStorageVK::multimesh_get_visible_instances(RID p_multimesh) {}

Rect3 RasterizerStorageVK::multimesh_get_aabb(RID p_multimesh) {}

/* IMMEDIATE API */

RID RasterizerStorageVK::immediate_create() {}
void RasterizerStorageVK::immediate_begin(RID p_immediate, VS::PrimitiveType p_rimitive, RID p_texture = RID()) {}
void RasterizerStorageVK::immediate_vertex(RID p_immediate, const Vector3 &p_vertex) {}
void RasterizerStorageVK::immediate_normal(RID p_immediate, const Vector3 &p_normal) {}
void RasterizerStorageVK::immediate_tangent(RID p_immediate, const Plane &p_tangent) {}
void RasterizerStorageVK::immediate_color(RID p_immediate, const Color &p_color) {}
void RasterizerStorageVK::immediate_uv(RID p_immediate, const Vector2 &tex_uv) {}
void RasterizerStorageVK::immediate_uv2(RID p_immediate, const Vector2 &tex_uv) {}
void RasterizerStorageVK::immediate_end(RID p_immediate) {}
void RasterizerStorageVK::immediate_clear(RID p_immediate) {}
void RasterizerStorageVK::immediate_set_material(RID p_immediate, RID p_material) {}
RID RasterizerStorageVK::immediate_get_material(RID p_immediate) {}
Rect3 RasterizerStorageVK::immediate_get_aabb(RID p_immediate) {}

/* SKELETON API */

RID RasterizerStorageVK::skeleton_create() {}
void RasterizerStorageVK::skeleton_allocate(RID p_skeleton, int p_bones, bool p_2d_skeleton = false) {}
int RasterizerStorageVK::skeleton_get_bone_count(RID p_skeleton) {}
void RasterizerStorageVK::skeleton_bone_set_transform(RID p_skeleton, int p_bone, const Transform &p_transform) {}
Transform RasterizerStorageVK::skeleton_bone_get_transform(RID p_skeleton, int p_bone) {}
void RasterizerStorageVK::skeleton_bone_set_transform_2d(RID p_skeleton, int p_bone, const Transform2D &p_transform) {}
Transform2D RasterizerStorageVK::skeleton_bone_get_transform_2d(RID p_skeleton, int p_bone) {}

/* Light API */

RID RasterizerStorageVK::light_create(VS::LightType p_type) {}

void RasterizerStorageVK::light_set_color(RID p_light, const Color &p_color) {}
void RasterizerStorageVK::light_set_param(RID p_light, VS::LightParam p_param, float p_value) {}
void RasterizerStorageVK::light_set_shadow(RID p_light, bool p_enabled) {}
void RasterizerStorageVK::light_set_shadow_color(RID p_light, const Color &p_color) {}
void RasterizerStorageVK::light_set_projector(RID p_light, RID p_texture) {}
void RasterizerStorageVK::light_set_negative(RID p_light, bool p_enable) {}
void RasterizerStorageVK::light_set_cull_mask(RID p_light, uint32_t p_mask) {}
void RasterizerStorageVK::light_set_reverse_cull_face_mode(RID p_light, bool p_enabled) {}

void RasterizerStorageVK::light_omni_set_shadow_mode(RID p_light, VS::LightOmniShadowMode p_mode) {}
void RasterizerStorageVK::light_omni_set_shadow_detail(RID p_light, VS::LightOmniShadowDetail p_detail) {}

void RasterizerStorageVK::light_directional_set_shadow_mode(RID p_light, VS::LightDirectionalShadowMode p_mode) {}
void RasterizerStorageVK::light_directional_set_blend_splits(RID p_light, bool p_enable) {}
bool RasterizerStorageVK::light_directional_get_blend_splits(RID p_light) {}
void RasterizerStorageVK::light_directional_set_shadow_depth_range_mode(RID p_light, VS::LightDirectionalShadowDepthRangeMode p_range_mode) {}
VS::LightDirectionalShadowDepthRangeMode RasterizerStorageVK::light_directional_get_shadow_depth_range_mode(RID p_light) {}

VS::LightDirectionalShadowMode RasterizerStorageVK::light_directional_get_shadow_mode(RID p_light) {}
VS::LightOmniShadowMode RasterizerStorageVK::light_omni_get_shadow_mode(RID p_light) {}

bool RasterizerStorageVK::light_has_shadow(RID p_light) {}

VS::LightType RasterizerStorageVK::light_get_type(RID p_light) {}
Rect3 RasterizerStorageVK::light_get_aabb(RID p_light) {}
float RasterizerStorageVK::light_get_param(RID p_light, VS::LightParam p_param) {}
Color RasterizerStorageVK::light_get_color(RID p_light) {}
uint64_t RasterizerStorageVK::light_get_version(RID p_light) {}

/* PROBE API */

RID RasterizerStorageVK::reflection_probe_create() {}

void RasterizerStorageVK::reflection_probe_set_update_mode(RID p_probe, VS::ReflectionProbeUpdateMode p_mode) {}
void RasterizerStorageVK::reflection_probe_set_intensity(RID p_probe, float p_intensity) {}
void RasterizerStorageVK::reflection_probe_set_interior_ambient(RID p_probe, const Color &p_ambient) {}
void RasterizerStorageVK::reflection_probe_set_interior_ambient_energy(RID p_probe, float p_energy) {}
void RasterizerStorageVK::reflection_probe_set_interior_ambient_probe_contribution(RID p_probe, float p_contrib) {}
void RasterizerStorageVK::reflection_probe_set_max_distance(RID p_probe, float p_distance) {}
void RasterizerStorageVK::reflection_probe_set_extents(RID p_probe, const Vector3 &p_extents) {}
void RasterizerStorageVK::reflection_probe_set_origin_offset(RID p_probe, const Vector3 &p_offset) {}
void RasterizerStorageVK::reflection_probe_set_as_interior(RID p_probe, bool p_enable) {}
void RasterizerStorageVK::reflection_probe_set_enable_box_projection(RID p_probe, bool p_enable) {}
void RasterizerStorageVK::reflection_probe_set_enable_shadows(RID p_probe, bool p_enable) {}
void RasterizerStorageVK::reflection_probe_set_cull_mask(RID p_probe, uint32_t p_layers) {}

Rect3 RasterizerStorageVK::reflection_probe_get_aabb(RID p_probe) {}
VS::ReflectionProbeUpdateMode RasterizerStorageVK::reflection_probe_get_update_mode(RID p_probe) {}
uint32_t RasterizerStorageVK::reflection_probe_get_cull_mask(RID p_probe) {}
Vector3 RasterizerStorageVK::reflection_probe_get_extents(RID p_probe) {}
Vector3 RasterizerStorageVK::reflection_probe_get_origin_offset(RID p_probe) {}
float RasterizerStorageVK::reflection_probe_get_origin_max_distance(RID p_probe) {}
bool RasterizerStorageVK::reflection_probe_renders_shadows(RID p_probe) {}

void RasterizerStorageVK::instance_add_skeleton(RID p_skeleton, RasterizerScene::InstanceBase *p_instance) {}
void RasterizerStorageVK::instance_remove_skeleton(RID p_skeleton, RasterizerScene::InstanceBase *p_instance) {}

void RasterizerStorageVK::instance_add_dependency(RID p_base, RasterizerScene::InstanceBase *p_instance) {}
void RasterizerStorageVK::instance_remove_dependency(RID p_base, RasterizerScene::InstanceBase *p_instance) {}

/* GI PROBE API */

RID RasterizerStorageVK::gi_probe_create() {}

void RasterizerStorageVK::gi_probe_set_bounds(RID p_probe, const Rect3 &p_bounds) {}
Rect3 RasterizerStorageVK::gi_probe_get_bounds(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_cell_size(RID p_probe, float p_range) {}
float RasterizerStorageVK::gi_probe_get_cell_size(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_to_cell_xform(RID p_probe, const Transform &p_xform) {}
Transform RasterizerStorageVK::gi_probe_get_to_cell_xform(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_dynamic_data(RID p_probe, const PoolVector<int> &p_data) {}
PoolVector<int> RasterizerStorageVK::gi_probe_get_dynamic_data(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_dynamic_range(RID p_probe, int p_range) {}
int RasterizerStorageVK::gi_probe_get_dynamic_range(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_energy(RID p_probe, float p_range) {}
float RasterizerStorageVK::gi_probe_get_energy(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_bias(RID p_probe, float p_range) {}
float RasterizerStorageVK::gi_probe_get_bias(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_normal_bias(RID p_probe, float p_range) {}
float RasterizerStorageVK::gi_probe_get_normal_bias(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_propagation(RID p_probe, float p_range) {}
float RasterizerStorageVK::gi_probe_get_propagation(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_interior(RID p_probe, bool p_enable) {}
bool RasterizerStorageVK::gi_probe_is_interior(RID p_probe) {}

void RasterizerStorageVK::gi_probe_set_compress(RID p_probe, bool p_enable) {}
bool RasterizerStorageVK::gi_probe_is_compressed(RID p_probe) {}

uint32_t RasterizerStorageVK::gi_probe_get_version(RID p_probe) {}

GIProbeCompression RasterizerStorageVK::gi_probe_get_dynamic_data_get_preferred_compression() {}
RID RasterizerStorageVK::gi_probe_dynamic_data_create(int p_width, int p_height, int p_depth, GIProbeCompression p_compression) {}
void RasterizerStorageVK::gi_probe_dynamic_data_update(RID p_gi_probe_data, int p_depth_slice, int p_slice_count, int p_mipmap, const void *p_data) {}

/* PARTICLES */

RID RasterizerStorageVK::particles_create() {}

void RasterizerStorageVK::particles_set_emitting(RID p_particles, bool p_emitting) {}
void RasterizerStorageVK::particles_set_amount(RID p_particles, int p_amount) {}
void RasterizerStorageVK::particles_set_lifetime(RID p_particles, float p_lifetime) {}
void RasterizerStorageVK::particles_set_one_shot(RID p_particles, bool p_one_shot) {}
void RasterizerStorageVK::particles_set_pre_process_time(RID p_particles, float p_time) {}
void RasterizerStorageVK::particles_set_explosiveness_ratio(RID p_particles, float p_ratio) {}
void RasterizerStorageVK::particles_set_randomness_ratio(RID p_particles, float p_ratio) {}
void RasterizerStorageVK::particles_set_custom_aabb(RID p_particles, const Rect3 &p_aabb) {}
void RasterizerStorageVK::particles_set_speed_scale(RID p_particles, float p_scale) {}
void RasterizerStorageVK::particles_set_use_local_coordinates(RID p_particles, bool p_enable) {}
void RasterizerStorageVK::particles_set_process_material(RID p_particles, RID p_material) {}
void RasterizerStorageVK::particles_set_fixed_fps(RID p_particles, int p_fps) {}
void RasterizerStorageVK::particles_set_fractional_delta(RID p_particles, bool p_enable) {}
void RasterizerStorageVK::particles_restart(RID p_particles) {}

void RasterizerStorageVK::particles_set_draw_order(RID p_particles, VS::ParticlesDrawOrder p_order) {}

void RasterizerStorageVK::particles_set_draw_passes(RID p_particles, int p_count) {}
void RasterizerStorageVK::particles_set_draw_pass_mesh(RID p_particles, int p_pass, RID p_mesh) {}

void RasterizerStorageVK::particles_request_process(RID p_particles) {}
Rect3 RasterizerStorageVK::particles_get_current_aabb(RID p_particles) {}
Rect3 RasterizerStorageVK::particles_get_aabb(RID p_particles) {}

void RasterizerStorageVK::particles_set_emission_transform(RID p_particles, const Transform &p_transform) {}

int RasterizerStorageVK::particles_get_draw_passes(RID p_particles) {}
RID RasterizerStorageVK::particles_get_draw_pass_mesh(RID p_particles, int p_pass) {}

/* RENDER TARGET */

RID RasterizerStorageVK::render_target_create() {}
void RasterizerStorageVK::render_target_set_size(RID p_render_target, int p_width, int p_height) {}
RID RasterizerStorageVK::render_target_get_texture(RID p_render_target) {}
void RasterizerStorageVK::render_target_set_flag(RID p_render_target, RenderTargetFlags p_flag, bool p_value) {}
bool RasterizerStorageVK::render_target_was_used(RID p_render_target) {}
void RasterizerStorageVK::render_target_clear_used(RID p_render_target) {}
void RasterizerStorageVK::render_target_set_msaa(RID p_render_target, VS::ViewportMSAA p_msaa) {}

/* CANVAS SHADOW */

RID RasterizerStorageVK::canvas_light_shadow_buffer_create(int p_width) {}

/* LIGHT SHADOW MAPPING */

RID RasterizerStorageVK::canvas_light_occluder_create() {}
void RasterizerStorageVK::canvas_light_occluder_set_polylines(RID p_occluder, const PoolVector<Vector2> &p_lines) {}
