#include "rasterizer_storage_vulkan.h"

RID RasterizerStorageVK::texture_create() {
	Texture *texture = memnew(Texture);
	ERR_FAIL_COND_V(!texture, RID());
	//glGenTextures(1, &texture->tex_id);
	texture->active = false;
	texture->total_data_size = 0;

	return texture_owner.make_rid(texture);
}

void RasterizerStorageVK::texture_allocate(
		RID p_texture, int p_width, int p_height, Image::Format p_format, uint32_t p_flags) {
}

void RasterizerStorageVK::texture_set_data(
		RID p_texture, const Ref<Image> &p_image, VS::CubeMapSide p_cube_side) {
}

Ref<Image> RasterizerStorageVK::texture_get_data(RID p_texture, VS::CubeMapSide p_cube_side) const {
	ERR_EXPLAIN("Sorry, It's not posible to obtain images back in not implemented");
	return Ref<Image>();
}

void RasterizerStorageVK::texture_set_flags(RID p_texture, uint32_t p_flags) {}

uint32_t RasterizerStorageVK::texture_get_flags(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture, 0);
	return texture->flags;
}

Image::Format RasterizerStorageVK::texture_get_format(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture, Image::FORMAT_L8);

	return texture->format;
}

uint32_t RasterizerStorageVK::texture_get_texid(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture, 0);
	return 0;
	//return texture->tex_id;
}

uint32_t RasterizerStorageVK::texture_get_width(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture, 0);
	return texture->width;
}

uint32_t RasterizerStorageVK::texture_get_height(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture, 0);
	return texture->height;
}

void RasterizerStorageVK::texture_set_size_override(RID p_texture, int p_width, int p_height) {}

void RasterizerStorageVK::texture_set_path(RID p_texture, const String &p_path) {}

String RasterizerStorageVK::texture_get_path(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);

	ERR_FAIL_COND_V(!texture, String());
	return texture->path;
}

void RasterizerStorageVK::texture_set_shrink_all_x2_on_set_data(bool p_enable) {}

void RasterizerStorageVK::texture_debug_usage(List<VS::TextureInfo> *r_info) {}

RID RasterizerStorageVK::texture_create_radiance_cubemap(RID p_source, int p_resolution) const {
	Texture *texture = texture_owner.get(p_source);
	ERR_FAIL_COND_V(!texture, RID());
	ERR_FAIL_COND_V(!(texture->flags & VS::TEXTURE_FLAG_CUBEMAP), RID());

	bool use_float = config.hdr_supported;

	if (p_resolution < 0) {
		p_resolution = texture->width;
	}

	/*glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(texture->target, texture->tex_id);*/

	if (config.srgb_decode_supported && texture->srgb && !texture->using_srgb) {

		//glTexParameteri(texture->target, _TEXTURE_SRGB_DECODE_EXT, _DECODE_EXT);
		texture->using_srgb = true;
#ifdef TOOLS_ENABLED
		if (!(texture->flags & VS::TEXTURE_FLAG_CONVERT_TO_LINEAR)) {
			texture->flags |= VS::TEXTURE_FLAG_CONVERT_TO_LINEAR;
			//notify that texture must be set to linear beforehand, so it works in other platforms when exported
		}
#endif
	}

	/*glActiveTexture(GL_TEXTURE1);
	GLuint new_cubemap;
	glGenTextures(1, &new_cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, new_cubemap);

	GLuint tmp_fb;

	glGenFramebuffers(1, &tmp_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, tmp_fb);*/

	int size = p_resolution;

	int lod = 0;

	//shaders.cubemap_filter.bind();

	int mipmaps = 6;

	int mm_level = mipmaps;

	/*GLenum internal_format = use_float ? GL_RGBA16F : GL_RGB10_A2;
	GLenum format = GL_RGBA;
	GLenum type = use_float ? GL_HALF_FLOAT : GL_UNSIGNED_INT_2_10_10_10_REV;

	while (mm_level) {

		for (int i = 0; i < 6; i++) {
			glTexImage2D(_cube_side_enum[i], lod, internal_format, size, size, 0, format, type, NULL);
		}

		lod++;
		mm_level--;

		if (size > 1)
			size >>= 1;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, lod - 1);

	lod = 0;
	mm_level = mipmaps;

	size = p_resolution;

	shaders.cubemap_filter.set_conditional(CubemapFilterShaderGLES3::USE_DUAL_PARABOLOID, false);

	while (mm_level) {

		for (int i = 0; i < 6; i++) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _cube_side_enum[i], new_cubemap, lod);

			glViewport(0, 0, size, size);
			glBindVertexArray(resources.quadie_array);

			shaders.cubemap_filter.set_uniform(CubemapFilterShaderGLES3::FACE_ID, i);
			shaders.cubemap_filter.set_uniform(CubemapFilterShaderGLES3::ROUGHNESS, lod / float(mipmaps - 1));

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glBindVertexArray(0);
			#ifdef DEBUG_ENABLED
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			ERR_CONTINUE(status != GL_FRAMEBUFFER_COMPLETE);
			#endif
			}

		if (size > 1)
			size >>= 1;
		lod++;
		mm_level--;
	}

	//restore ranges
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, lod - 1);

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, RasterizerStorageGLES3::system_fbo);
	glDeleteFramebuffers(1, &tmp_fb);*/

	Texture *ctex = memnew(Texture);

	ctex->flags = VS::TEXTURE_FLAG_CUBEMAP | VS::TEXTURE_FLAG_MIPMAPS | VS::TEXTURE_FLAG_FILTER;
	ctex->width = p_resolution;
	ctex->height = p_resolution;
	ctex->alloc_width = p_resolution;
	ctex->alloc_height = p_resolution;
	ctex->format = use_float ? Image::FORMAT_RGBAH : Image::FORMAT_RGBA8;
	//ctex->target = GL_TEXTURE_CUBE_MAP;
	//ctex->gl_format_cache = format;
	//ctex->gl_internal_format_cache = internal_format;
	//ctex->gl_type_cache = type;
	ctex->data_size = 0;
	ctex->compressed = false;
	ctex->srgb = false;
	ctex->total_data_size = 0;
	ctex->ignore_mipmaps = false;
	ctex->mipmaps = mipmaps;
	ctex->active = true;
	//ctex->tex_id = new_cubemap;
	ctex->stored_cube_sides = (1 << 6) - 1;
	ctex->render_target = NULL;

	return texture_owner.make_rid(ctex);
}

void RasterizerStorageVK::texture_set_detect_3d_callback(
		RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata) {
}

void RasterizerStorageVK::texture_set_detect_srgb_callback(
		RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata) {
}

void RasterizerStorageVK::texture_set_detect_normal_callback(
		RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata) {
}

void RasterizerStorageVK::textures_keep_original(bool p_enable) {}

/* SKY API */

RID RasterizerStorageVK::sky_create() {
	Sky *sky = memnew(Sky);
	//sky->radiance = 0;

	return sky_owner.make_rid(sky);
}

void RasterizerStorageVK::sky_set_texture(RID p_sky, RID p_cube_map, int p_radiance_size) {}

/* SHADER API */

RID RasterizerStorageVK::shader_create() {
	Shader *shader = memnew(Shader);
	shader->mode = VS::SHADER_SPATIAL;
	//shader->shader = &scene->state.scene_shader;
	RID rid = shader_owner.make_rid(shader);
	//_shader_make_dirty(shader);
	shader->self = rid;

	return rid;
}

void RasterizerStorageVK::shader_set_code(RID p_shader, const String &p_code) {}

String RasterizerStorageVK::shader_get_code(RID p_shader) const {
	const Shader *shader = shader_owner.get(p_shader);

	ERR_FAIL_COND_V(!shader, String());
	return shader->code;
}

void RasterizerStorageVK::shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const {}

void RasterizerStorageVK::shader_set_default_texture_param(
		RID p_shader, const StringName &p_name, RID p_texture) {
}

RID RasterizerStorageVK::shader_get_default_texture_param(RID p_shader, const StringName &p_name) const {
	const Shader *shader = shader_owner.get(p_shader);
	ERR_FAIL_COND_V(!shader, RID());

	const Map<StringName, RID>::Element *E = shader->default_textures.find(p_name);
	if (!E)
		return RID();
	return E->get();
}

/* COMMON MATERIAL API */

RID RasterizerStorageVK::material_create() {
	Material *material = memnew(Material);
	return material_owner.make_rid(material);
}

void RasterizerStorageVK::material_set_render_priority(RID p_material, int priority) {}

void RasterizerStorageVK::material_set_shader(RID p_shader_material, RID p_shader) {}

RID RasterizerStorageVK::material_get_shader(RID p_shader_material) const {
	const Material *material = material_owner.get(p_shader_material);
	ERR_FAIL_COND_V(!material, RID());

	if (material->shader)
		return material->shader->self;

	return RID();
}

void RasterizerStorageVK::material_set_param(
		RID p_material, const StringName &p_param, const Variant &p_value) {
}

Variant RasterizerStorageVK::material_get_param(RID p_material, const StringName &p_param) const {
	const Material *material = material_owner.get(p_material);
	ERR_FAIL_COND_V(!material, RID());

	if (material->params.has(p_param))
		return material->params[p_param];

	return Variant();
}

void RasterizerStorageVK::material_set_line_width(RID p_material, float p_width) {}

void RasterizerStorageVK::material_set_next_pass(RID p_material, RID p_next_material) {}

bool RasterizerStorageVK::material_is_animated(RID p_material) {}

bool RasterizerStorageVK::material_casts_shadows(RID p_material) {}

void RasterizerStorageVK::material_add_instance_owner(
		RID p_material, RasterizerScene::InstanceBase *p_instance) {
}

void RasterizerStorageVK::material_remove_instance_owner(
		RID p_material, RasterizerScene::InstanceBase *p_instance) {
}

/* MESH API */

RID RasterizerStorageVK::mesh_create() {
	Mesh *mesh = memnew(Mesh);
	return mesh_owner.make_rid(mesh);
}

void RasterizerStorageVK::mesh_add_surface(
		RID p_mesh, uint32_t p_format, VS::PrimitiveType p_primitive,
		const PoolVector<uint8_t> &p_array, int p_vertex_count,
		const PoolVector<uint8_t> &p_index_array, int p_index_count,
		const Rect3 &p_aabb, const Vector<PoolVector<uint8_t> > &p_blend_shapes,
		const Vector<Rect3> &p_bone_aabbs) {
}

void RasterizerStorageVK::mesh_set_blend_shape_count(RID p_mesh, int p_amount) {}

int RasterizerStorageVK::mesh_get_blend_shape_count(RID p_mesh) const {}

void RasterizerStorageVK::mesh_set_blend_shape_mode(RID p_mesh, VS::BlendShapeMode p_mode) {}

VS::BlendShapeMode RasterizerStorageVK::mesh_get_blend_shape_mode(RID p_mesh) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);
	ERR_FAIL_COND_V(!mesh, VS::BLEND_SHAPE_MODE_NORMALIZED);

	return mesh->blend_shape_mode;
}

void RasterizerStorageVK::mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material) {}

RID RasterizerStorageVK::mesh_surface_get_material(RID p_mesh, int p_surface) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);
	ERR_FAIL_COND_V(!mesh, RID());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), RID());

	return mesh->surfaces[p_surface]->material;
}

int RasterizerStorageVK::mesh_surface_get_array_len(RID p_mesh, int p_surface) const {}

int RasterizerStorageVK::mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const {}

PoolVector<uint8_t> RasterizerStorageVK::mesh_surface_get_array(RID p_mesh, int p_surface) const {
	return PoolVector<uint8_t>();
}

PoolVector<uint8_t> RasterizerStorageVK::mesh_surface_get_index_array(RID p_mesh, int p_surface) const {
	return PoolVector<uint8_t>();
}

uint32_t RasterizerStorageVK::mesh_surface_get_format(RID p_mesh, int p_surface) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);

	ERR_FAIL_COND_V(!mesh, 0);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), 0);

	return mesh->surfaces[p_surface]->format;
}

VS::PrimitiveType RasterizerStorageVK::mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);
	ERR_FAIL_COND_V(!mesh, VS::PRIMITIVE_MAX);
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), VS::PRIMITIVE_MAX);

	return mesh->surfaces[p_surface]->primitive;
}

Rect3 RasterizerStorageVK::mesh_surface_get_aabb(RID p_mesh, int p_surface) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);
	ERR_FAIL_COND_V(!mesh, Rect3());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), Rect3());

	return mesh->surfaces[p_surface]->aabb;
}

Vector<PoolVector<uint8_t> > RasterizerStorageVK::mesh_surface_get_blend_shapes(RID p_mesh, int p_surface) const {
	return Vector<PoolVector<uint8_t> >();
}

Vector<Rect3> RasterizerStorageVK::mesh_surface_get_skeleton_aabb(RID p_mesh, int p_surface) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);
	ERR_FAIL_COND_V(!mesh, Vector<Rect3>());
	ERR_FAIL_INDEX_V(p_surface, mesh->surfaces.size(), Vector<Rect3>());

	return mesh->surfaces[p_surface]->skeleton_bone_aabb;
}

void RasterizerStorageVK::mesh_remove_surface(RID p_mesh, int p_index) {}

int RasterizerStorageVK::mesh_get_surface_count(RID p_mesh) const {}

void RasterizerStorageVK::mesh_set_custom_aabb(RID p_mesh, const Rect3 &p_aabb) {}

Rect3 RasterizerStorageVK::mesh_get_custom_aabb(RID p_mesh) const {
	const Mesh *mesh = mesh_owner.getornull(p_mesh);
	ERR_FAIL_COND_V(!mesh, Rect3());

	return mesh->custom_aabb;
}

Rect3 RasterizerStorageVK::mesh_get_aabb(RID p_mesh, RID p_skeleton) const {
	return Rect3();
}

void RasterizerStorageVK::mesh_clear(RID p_mesh) {}

/* MULTIMESH API */

RID RasterizerStorageVK::multimesh_create() {
	MultiMesh *multimesh = memnew(MultiMesh);
	return multimesh_owner.make_rid(multimesh);
}

void RasterizerStorageVK::multimesh_allocate(
		RID p_multimesh, int p_instances,
		VS::MultimeshTransformFormat p_transform_format,
		VS::MultimeshColorFormat p_color_format) {
}

int RasterizerStorageVK::multimesh_get_instance_count(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, 0);

	return multimesh->size;
}

void RasterizerStorageVK::multimesh_set_mesh(RID p_multimesh, RID p_mesh) {}

void RasterizerStorageVK::multimesh_instance_set_transform(
		RID p_multimesh, int p_index, const Transform &p_transform) {
}

void RasterizerStorageVK::multimesh_instance_set_transform_2d(
		RID p_multimesh, int p_index, const Transform2D &p_transform) {
}

void RasterizerStorageVK::multimesh_instance_set_color(
		RID p_multimesh, int p_index, const Color &p_color) {
}

RID RasterizerStorageVK::multimesh_get_mesh(RID p_multimesh) const {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, RID());

	return multimesh->mesh;
}

Transform RasterizerStorageVK::multimesh_instance_get_transform(RID p_multimesh, int p_index) const {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Transform());
	ERR_FAIL_INDEX_V(p_index, multimesh->size, Transform());
	ERR_FAIL_COND_V(multimesh->transform_format == VS::MULTIMESH_TRANSFORM_2D, Transform());

	int stride = multimesh->color_floats + multimesh->xform_floats;
	float *dataptr = &multimesh->data[stride * p_index];

	Transform xform;

	xform.basis.elements[0][0] = dataptr[0];
	xform.basis.elements[0][1] = dataptr[1];
	xform.basis.elements[0][2] = dataptr[2];
	xform.origin.x = dataptr[3];
	xform.basis.elements[1][0] = dataptr[4];
	xform.basis.elements[1][1] = dataptr[5];
	xform.basis.elements[1][2] = dataptr[6];
	xform.origin.y = dataptr[7];
	xform.basis.elements[2][0] = dataptr[8];
	xform.basis.elements[2][1] = dataptr[9];
	xform.basis.elements[2][2] = dataptr[10];
	xform.origin.z = dataptr[11];

	return xform;
}

Transform2D RasterizerStorageVK::multimesh_instance_get_transform_2d(RID p_multimesh, int p_index) const {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Transform2D());
	ERR_FAIL_INDEX_V(p_index, multimesh->size, Transform2D());
	ERR_FAIL_COND_V(multimesh->transform_format == VS::MULTIMESH_TRANSFORM_3D, Transform2D());

	int stride = multimesh->color_floats + multimesh->xform_floats;
	float *dataptr = &multimesh->data[stride * p_index];

	Transform2D xform;

	xform.elements[0][0] = dataptr[0];
	xform.elements[1][0] = dataptr[1];
	xform.elements[2][0] = dataptr[3];
	xform.elements[0][1] = dataptr[4];
	xform.elements[1][1] = dataptr[5];
	xform.elements[2][1] = dataptr[7];

	return xform;
}

Color RasterizerStorageVK::multimesh_instance_get_color(RID p_multimesh, int p_index) {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Color());
	ERR_FAIL_INDEX_V(p_index, multimesh->size, Color());
	ERR_FAIL_COND_V(multimesh->color_format == VS::MULTIMESH_COLOR_NONE, Color());

	int stride = multimesh->color_floats + multimesh->xform_floats;
	float *dataptr = &multimesh->data[stride * p_index + multimesh->xform_floats];

	if (multimesh->color_format == VS::MULTIMESH_COLOR_8BIT) {
		union {
			uint32_t colu;
			float colf;
		} cu;

		cu.colf = dataptr[0];

		return Color::hex(BSWAP32(cu.colu));

	} else if (multimesh->color_format == VS::MULTIMESH_COLOR_FLOAT) {
		Color c;
		c.r = dataptr[0];
		c.g = dataptr[1];
		c.b = dataptr[2];
		c.a = dataptr[3];

		return c;
	}

	return Color();
}

void RasterizerStorageVK::multimesh_set_visible_instances(RID p_multimesh, int p_visible) {}

int RasterizerStorageVK::multimesh_get_visible_instances(RID p_multimesh) {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, -1);

	return multimesh->visible_instances;
}

Rect3 RasterizerStorageVK::multimesh_get_aabb(RID p_multimesh) {
	MultiMesh *multimesh = multimesh_owner.getornull(p_multimesh);
	ERR_FAIL_COND_V(!multimesh, Rect3());

	//update pending AABBs
	const_cast<RasterizerStorageVK *>(this)->update_dirty_multimeshes();

	return multimesh->aabb;
}

/* IMMEDIATE API */

RID RasterizerStorageVK::immediate_create() {
	Immediate *im = memnew(Immediate);
	return immediate_owner.make_rid(im);
}

void RasterizerStorageVK::immediate_begin(
		RID p_immediate, VS::PrimitiveType p_rimitive, RID p_texture) {
}

void RasterizerStorageVK::immediate_vertex(RID p_immediate, const Vector3 &p_vertex) {}

void RasterizerStorageVK::immediate_normal(RID p_immediate, const Vector3 &p_normal) {}

void RasterizerStorageVK::immediate_tangent(RID p_immediate, const Plane &p_tangent) {}

void RasterizerStorageVK::immediate_color(RID p_immediate, const Color &p_color) {}

void RasterizerStorageVK::immediate_uv(RID p_immediate, const Vector2 &tex_uv) {}

void RasterizerStorageVK::immediate_uv2(RID p_immediate, const Vector2 &tex_uv) {}

void RasterizerStorageVK::immediate_end(RID p_immediate) {}

void RasterizerStorageVK::immediate_clear(RID p_immediate) {}

void RasterizerStorageVK::immediate_set_material(RID p_immediate, RID p_material) {}

RID RasterizerStorageVK::immediate_get_material(RID p_immediate) {
	const Immediate *im = immediate_owner.get(p_immediate);
	ERR_FAIL_COND_V(!im, RID());

	return im->material;
}

Rect3 RasterizerStorageVK::immediate_get_aabb(RID p_immediate) {
	Immediate *im = immediate_owner.get(p_immediate);
	ERR_FAIL_COND_V(!im, Rect3());

	return im->aabb;
}

/* SKELETON API */

RID RasterizerStorageVK::skeleton_create() {
	Skeleton *skeleton = memnew(Skeleton);
	//glGenTextures(1, &skeleton->texture);

	return skeleton_owner.make_rid(skeleton);
}

void RasterizerStorageVK::skeleton_allocate(RID p_skeleton, int p_bones, bool p_2d_skeleton) {}

int RasterizerStorageVK::skeleton_get_bone_count(RID p_skeleton) {}

void RasterizerStorageVK::skeleton_bone_set_transform(
		RID p_skeleton, int p_bone, const Transform &p_transform) {
}

Transform RasterizerStorageVK::skeleton_bone_get_transform(RID p_skeleton, int p_bone) {
	Skeleton *skeleton = skeleton_owner.getornull(p_skeleton);

	ERR_FAIL_COND_V(!skeleton, Transform());
	ERR_FAIL_INDEX_V(p_bone, skeleton->size, Transform());
	ERR_FAIL_COND_V(skeleton->use_2d, Transform());

	const float *texture = skeleton->skel_texture.ptr();

	Transform ret;

	int base_ofs = ((p_bone / 256) * 256) * 3 * 4 + (p_bone % 256) * 4;

	ret.basis[0].x = texture[base_ofs + 0];
	ret.basis[0].y = texture[base_ofs + 1];
	ret.basis[0].z = texture[base_ofs + 2];
	ret.origin.x = texture[base_ofs + 3];
	base_ofs += 256 * 4;
	ret.basis[1].x = texture[base_ofs + 0];
	ret.basis[1].y = texture[base_ofs + 1];
	ret.basis[1].z = texture[base_ofs + 2];
	ret.origin.y = texture[base_ofs + 3];
	base_ofs += 256 * 4;
	ret.basis[2].x = texture[base_ofs + 0];
	ret.basis[2].y = texture[base_ofs + 1];
	ret.basis[2].z = texture[base_ofs + 2];
	ret.origin.z = texture[base_ofs + 3];

	return ret;
}

void RasterizerStorageVK::skeleton_bone_set_transform_2d(
		RID p_skeleton, int p_bone, const Transform2D &p_transform) {
}

Transform2D RasterizerStorageVK::skeleton_bone_get_transform_2d(RID p_skeleton, int p_bone) {
	Skeleton *skeleton = skeleton_owner.getornull(p_skeleton);

	ERR_FAIL_COND_V(!skeleton, Transform2D());
	ERR_FAIL_INDEX_V(p_bone, skeleton->size, Transform2D());
	ERR_FAIL_COND_V(!skeleton->use_2d, Transform2D());

	const float *texture = skeleton->skel_texture.ptr();

	Transform2D ret;

	int base_ofs = ((p_bone / 256) * 256) * 2 * 4 + (p_bone % 256) * 4;

	ret[0][0] = texture[base_ofs + 0];
	ret[1][0] = texture[base_ofs + 1];
	ret[2][0] = texture[base_ofs + 3];
	base_ofs += 256 * 4;
	ret[0][1] = texture[base_ofs + 0];
	ret[1][1] = texture[base_ofs + 1];
	ret[2][1] = texture[base_ofs + 3];

	return ret;
}

/* Light API */

RID RasterizerStorageVK::light_create(VS::LightType p_type) {
	Light *light = memnew(Light);
	light->type = p_type;

	light->param[VS::LIGHT_PARAM_ENERGY] = 1.0;
	light->param[VS::LIGHT_PARAM_SPECULAR] = 0.5;
	light->param[VS::LIGHT_PARAM_RANGE] = 1.0;
	light->param[VS::LIGHT_PARAM_SPOT_ANGLE] = 45;
	light->param[VS::LIGHT_PARAM_CONTACT_SHADOW_SIZE] = 45;
	light->param[VS::LIGHT_PARAM_SHADOW_MAX_DISTANCE] = 0;
	light->param[VS::LIGHT_PARAM_SHADOW_SPLIT_1_OFFSET] = 0.1;
	light->param[VS::LIGHT_PARAM_SHADOW_SPLIT_2_OFFSET] = 0.3;
	light->param[VS::LIGHT_PARAM_SHADOW_SPLIT_3_OFFSET] = 0.6;
	light->param[VS::LIGHT_PARAM_SHADOW_NORMAL_BIAS] = 0.1;
	light->param[VS::LIGHT_PARAM_SHADOW_BIAS_SPLIT_SCALE] = 0.1;

	light->color = Color(1, 1, 1, 1);
	light->shadow = false;
	light->negative = false;
	light->cull_mask = 0xFFFFFFFF;
	light->directional_shadow_mode = VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL;
	light->omni_shadow_mode = VS::LIGHT_OMNI_SHADOW_DUAL_PARABOLOID;
	light->omni_shadow_detail = VS::LIGHT_OMNI_SHADOW_DETAIL_VERTICAL;
	light->directional_blend_splits = false;
	light->directional_range_mode = VS::LIGHT_DIRECTIONAL_SHADOW_DEPTH_RANGE_STABLE;
	light->reverse_cull = false;
	light->version = 0;

	return light_owner.make_rid(light);
}

void RasterizerStorageVK::light_set_color(RID p_light, const Color &p_color) {}

void RasterizerStorageVK::light_set_param(RID p_light, VS::LightParam p_param, float p_value) {}

void RasterizerStorageVK::light_set_shadow(RID p_light, bool p_enabled) {}

void RasterizerStorageVK::light_set_shadow_color(RID p_light, const Color &p_color) {}

void RasterizerStorageVK::light_set_projector(RID p_light, RID p_texture) {}

void RasterizerStorageVK::light_set_negative(RID p_light, bool p_enable) {}

void RasterizerStorageVK::light_set_cull_mask(RID p_light, uint32_t p_mask) {}

void RasterizerStorageVK::light_set_reverse_cull_face_mode(RID p_light, bool p_enabled) {}

void RasterizerStorageVK::light_omni_set_shadow_mode(
		RID p_light, VS::LightOmniShadowMode p_mode) {
}

void RasterizerStorageVK::light_omni_set_shadow_detail(
		RID p_light, VS::LightOmniShadowDetail p_detail) {
}

void RasterizerStorageVK::light_directional_set_shadow_mode(
		RID p_light, VS::LightDirectionalShadowMode p_mode) {
}

void RasterizerStorageVK::light_directional_set_blend_splits(RID p_light, bool p_enable) {}

bool RasterizerStorageVK::light_directional_get_blend_splits(RID p_light) {}

void RasterizerStorageVK::light_directional_set_shadow_depth_range_mode(
		RID p_light, VS::LightDirectionalShadowDepthRangeMode p_range_mode) {
}

VS::LightDirectionalShadowDepthRangeMode
RasterizerStorageVK::light_directional_get_shadow_depth_range_mode(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, VS::LIGHT_DIRECTIONAL_SHADOW_DEPTH_RANGE_STABLE);

	return light->directional_range_mode;
}

VS::LightDirectionalShadowMode RasterizerStorageVK::light_directional_get_shadow_mode(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, VS::LIGHT_DIRECTIONAL_SHADOW_ORTHOGONAL);

	return light->directional_shadow_mode;
}

VS::LightOmniShadowMode RasterizerStorageVK::light_omni_get_shadow_mode(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, VS::LIGHT_OMNI_SHADOW_CUBE);

	return light->omni_shadow_mode;
}

bool RasterizerStorageVK::light_has_shadow(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, VS::LIGHT_DIRECTIONAL);

	return light->shadow;
}

VS::LightType RasterizerStorageVK::light_get_type(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, VS::LIGHT_DIRECTIONAL);

	return light->type;
}

Rect3 RasterizerStorageVK::light_get_aabb(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, Rect3());

	switch (light->type) {

		case VS::LIGHT_SPOT: {

			float len = light->param[VS::LIGHT_PARAM_RANGE];
			float size = Math::tan(Math::deg2rad(light->param[VS::LIGHT_PARAM_SPOT_ANGLE])) * len;
			return Rect3(Vector3(-size, -size, -len), Vector3(size * 2, size * 2, len));
		} break;
		case VS::LIGHT_OMNI: {

			float r = light->param[VS::LIGHT_PARAM_RANGE];
			return Rect3(-Vector3(r, r, r), Vector3(r, r, r) * 2);
		} break;
		case VS::LIGHT_DIRECTIONAL: {

			return Rect3();
		} break;
		default: {}
	}

	ERR_FAIL_V(Rect3());
	return Rect3();
}

float RasterizerStorageVK::light_get_param(RID p_light, VS::LightParam p_param) {}

Color RasterizerStorageVK::light_get_color(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, Color());

	return light->color;
}

uint64_t RasterizerStorageVK::light_get_version(RID p_light) {
	const Light *light = light_owner.getornull(p_light);
	ERR_FAIL_COND_V(!light, 0);

	return light->version;
}

/* PROBE API */

RID RasterizerStorageVK::reflection_probe_create() {
	ReflectionProbe *reflection_probe = memnew(ReflectionProbe);

	reflection_probe->intensity = 1.0;
	reflection_probe->interior_ambient = Color();
	reflection_probe->interior_ambient_energy = 1.0;
	reflection_probe->max_distance = 0;
	reflection_probe->extents = Vector3(1, 1, 1);
	reflection_probe->origin_offset = Vector3(0, 0, 0);
	reflection_probe->interior = false;
	reflection_probe->box_projection = false;
	reflection_probe->enable_shadows = false;
	reflection_probe->cull_mask = (1 << 20) - 1;
	reflection_probe->update_mode = VS::REFLECTION_PROBE_UPDATE_ONCE;

	return reflection_probe_owner.make_rid(reflection_probe);
}

void RasterizerStorageVK::reflection_probe_set_update_mode(
		RID p_probe, VS::ReflectionProbeUpdateMode p_mode) {
}

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

Rect3 RasterizerStorageVK::reflection_probe_get_aabb(RID p_probe) {
	const ReflectionProbe *reflection_probe = reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!reflection_probe, Rect3());

	Rect3 aabb;
	aabb.position = -reflection_probe->extents;
	aabb.size = reflection_probe->extents * 2.0;

	return aabb;
}

VS::ReflectionProbeUpdateMode RasterizerStorageVK::reflection_probe_get_update_mode(RID p_probe) {
	const ReflectionProbe *reflection_probe = reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!reflection_probe, VS::REFLECTION_PROBE_UPDATE_ALWAYS);

	return reflection_probe->update_mode;
}

uint32_t RasterizerStorageVK::reflection_probe_get_cull_mask(RID p_probe) {
	const ReflectionProbe *reflection_probe = reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!reflection_probe, 0);

	return reflection_probe->cull_mask;
}

Vector3 RasterizerStorageVK::reflection_probe_get_extents(RID p_probe) {
	const ReflectionProbe *reflection_probe = reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!reflection_probe, Vector3());

	return reflection_probe->extents;
}

Vector3 RasterizerStorageVK::reflection_probe_get_origin_offset(RID p_probe) {
	const ReflectionProbe *reflection_probe = reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!reflection_probe, Vector3());

	return reflection_probe->origin_offset;
}

float RasterizerStorageVK::reflection_probe_get_origin_max_distance(RID p_probe) {}

bool RasterizerStorageVK::reflection_probe_renders_shadows(RID p_probe) {
	const ReflectionProbe *reflection_probe = reflection_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!reflection_probe, false);

	return reflection_probe->enable_shadows;
}

/* GI PROBE API */

RID RasterizerStorageVK::gi_probe_create() {
	GIProbe *gip = memnew(GIProbe);
	gip->bounds = Rect3(Vector3(), Vector3(1, 1, 1));
	gip->dynamic_range = 1.0;
	gip->energy = 1.0;
	gip->propagation = 1.0;
	gip->bias = 0.4;
	gip->normal_bias = 0.4;
	gip->interior = false;
	gip->compress = false;
	gip->version = 1;
	gip->cell_size = 1.0;

	return gi_probe_owner.make_rid(gip);
}

void RasterizerStorageVK::gi_probe_set_bounds(RID p_probe, const Rect3 &p_bounds) {}

Rect3 RasterizerStorageVK::gi_probe_get_bounds(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, Rect3());

	return gip->bounds;
}

void RasterizerStorageVK::gi_probe_set_cell_size(RID p_probe, float p_range) {}

float RasterizerStorageVK::gi_probe_get_cell_size(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->cell_size;
}

void RasterizerStorageVK::gi_probe_set_to_cell_xform(RID p_probe, const Transform &p_xform) {}

Transform RasterizerStorageVK::gi_probe_get_to_cell_xform(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, Transform());

	return gip->to_cell;
}

void RasterizerStorageVK::gi_probe_set_dynamic_data(RID p_probe, const PoolVector<int> &p_data) {}

PoolVector<int> RasterizerStorageVK::gi_probe_get_dynamic_data(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, PoolVector<int>());

	return gip->dynamic_data;
}

void RasterizerStorageVK::gi_probe_set_dynamic_range(RID p_probe, int p_range) {}

int RasterizerStorageVK::gi_probe_get_dynamic_range(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->dynamic_range;
}

void RasterizerStorageVK::gi_probe_set_energy(RID p_probe, float p_range) {}

float RasterizerStorageVK::gi_probe_get_energy(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->energy;
}

void RasterizerStorageVK::gi_probe_set_bias(RID p_probe, float p_range) {}

float RasterizerStorageVK::gi_probe_get_bias(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->bias;
}

void RasterizerStorageVK::gi_probe_set_normal_bias(RID p_probe, float p_range) {}

float RasterizerStorageVK::gi_probe_get_normal_bias(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->normal_bias;
}

void RasterizerStorageVK::gi_probe_set_propagation(RID p_probe, float p_range) {}

float RasterizerStorageVK::gi_probe_get_propagation(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->propagation;
}

void RasterizerStorageVK::gi_probe_set_interior(RID p_probe, bool p_enable) {}

bool RasterizerStorageVK::gi_probe_is_interior(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, false);

	return gip->interior;
}

void RasterizerStorageVK::gi_probe_set_compress(RID p_probe, bool p_enable) {}

bool RasterizerStorageVK::gi_probe_is_compressed(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, false);

	return gip->compress;
}

uint32_t RasterizerStorageVK::gi_probe_get_version(RID p_probe) {
	const GIProbe *gip = gi_probe_owner.getornull(p_probe);
	ERR_FAIL_COND_V(!gip, 0);

	return gip->version;
}

RasterizerStorage::GIProbeCompression RasterizerStorageVK::gi_probe_get_dynamic_data_get_preferred_compression() {
	if (config.s3tc_supported) {
		return GI_PROBE_S3TC;
	} else {
		return GI_PROBE_UNCOMPRESSED;
	}
}

RID RasterizerStorageVK::gi_probe_dynamic_data_create(
		int p_width, int p_height, int p_depth, GIProbeCompression p_compression) {
	GIProbeData *gipd = memnew(GIProbeData);

	gipd->width = p_width;
	gipd->height = p_height;
	gipd->depth = p_depth;
	gipd->compression = p_compression;

	/*glActiveTexture(GL_TEXTURE0);
			glGenTextures(1, &gipd->tex_id);
			glBindTexture(GL_TEXTURE_3D, gipd->tex_id);

			int level = 0;
			int min_size = 1;

			if (gipd->compression == GI_PROBE_S3TC) {
				min_size = 4;
			}

			while (true) {

				if (gipd->compression == GI_PROBE_S3TC) {
					int size = p_width * p_height * p_depth;
					glCompressedTexImage3D(GL_TEXTURE_3D, level, _EXT_COMPRESSED_RGBA_S3TC_DXT5_EXT, p_width, p_height, p_depth, 0, size, NULL);
				} else {
					glTexImage3D(GL_TEXTURE_3D, level, GL_RGBA8, p_width, p_height, p_depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				}

				if (p_width <= min_size || p_height <= min_size || p_depth <= min_size)
					break;
				p_width >>= 1;
				p_height >>= 1;
				p_depth >>= 1;
				level++;
			}

			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, level);*/

	gipd->levels = level + 1;

	return gi_probe_data_owner.make_rid(gipd);
}

void RasterizerStorageVK::gi_probe_dynamic_data_update(
		RID p_gi_probe_data, int p_depth_slice, int p_slice_count, int p_mipmap, const void *p_data) {
}

/* PARTICLES */

RID RasterizerStorageVK::particles_create() {
	Particles *particles = memnew(Particles);
	return particles_owner.make_rid(particles);
}

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

Rect3 RasterizerStorageVK::particles_get_current_aabb(RID p_particles) {
	return Rect3();
}

Rect3 RasterizerStorageVK::particles_get_aabb(RID p_particles) {
	const Particles *particles = particles_owner.getornull(p_particles);
	ERR_FAIL_COND_V(!particles, Rect3());

	return particles->custom_aabb;
}

void RasterizerStorageVK::particles_set_emission_transform(RID p_particles, const Transform &p_transform) {}

int RasterizerStorageVK::particles_get_draw_passes(RID p_particles) {
	const Particles *particles = particles_owner.getornull(p_particles);
	ERR_FAIL_COND_V(!particles, 0);

	return particles->draw_passes.size();
}

RID RasterizerStorageVK::particles_get_draw_pass_mesh(RID p_particles, int p_pass) {
	const Particles *particles = particles_owner.getornull(p_particles);
	ERR_FAIL_COND_V(!particles, RID());
	ERR_FAIL_INDEX_V(p_pass, particles->draw_passes.size(), RID());

	return particles->draw_passes[p_pass];
}

/* INSTANCE */

void RasterizerStorageVK::instance_add_skeleton(
		RID p_skeleton, RasterizerScene::InstanceBase *p_instance) {
}

void RasterizerStorageVK::instance_remove_skeleton(
		RID p_skeleton, RasterizerScene::InstanceBase *p_instance) {
}

void RasterizerStorageVK::instance_add_dependency(
		RID p_base, RasterizerScene::InstanceBase *p_instance) {
}

void RasterizerStorageVK::instance_remove_dependency(
		RID p_base, RasterizerScene::InstanceBase *p_instance) {
}

/* RENDER TARGET */

RID RasterizerStorageVK::render_target_create() {
	RenderTarget *rt = memnew(RenderTarget);

	Texture *t = memnew(Texture);
	t->flags = 0;
	t->width = 0;
	t->height = 0;
	t->alloc_height = 0;
	t->alloc_width = 0;
	t->format = Image::FORMAT_R8;
	//t->target = GL_TEXTURE_2D;
	t->gl_format_cache = 0;
	t->gl_internal_format_cache = 0;
	t->gl_type_cache = 0;
	t->data_size = 0;
	t->compressed = false;
	t->srgb = false;
	t->total_data_size = 0;
	t->ignore_mipmaps = false;
	t->mipmaps = 1;
	t->active = true;
	t->tex_id = 0;
	t->render_target = rt;

	rt->texture = texture_owner.make_rid(t);

	return render_target_owner.make_rid(rt);
}

void RasterizerStorageVK::render_target_set_size(RID p_render_target, int p_width, int p_height) {}

RID RasterizerStorageVK::render_target_get_texture(RID p_render_target) {
	RenderTarget *rt = render_target_owner.getornull(p_render_target);
	ERR_FAIL_COND_V(!rt, RID());

	return rt->texture;
}

void RasterizerStorageVK::render_target_set_flag(RID p_render_target, RenderTargetFlags p_flag, bool p_value) {}

bool RasterizerStorageVK::render_target_was_used(RID p_render_target) {
	RenderTarget *rt = render_target_owner.getornull(p_render_target);
	ERR_FAIL_COND_V(!rt, false);

	return rt->used_in_frame;
}

void RasterizerStorageVK::render_target_clear_used(RID p_render_target) {}

void RasterizerStorageVK::render_target_set_msaa(RID p_render_target, VS::ViewportMSAA p_msaa) {}

/* CANVAS SHADOW */

RID RasterizerStorageVK::canvas_light_shadow_buffer_create(int p_width) {
	return RID();

	/*CanvasLightShadow *cls = memnew(CanvasLightShadow);
	if (p_width > config.max_texture_size)
		p_width = config.max_texture_size;

	cls->size = p_width;
	cls->height = 16;

	glActiveTexture(GL_TEXTURE0);

	glGenFramebuffers(1, &cls->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, cls->fbo);

	glGenRenderbuffers(1, &cls->depth);
	glBindRenderbuffer(GL_RENDERBUFFER, cls->depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cls->size, cls->height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, cls->depth);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenTextures(1, &cls->distance);
	glBindTexture(GL_TEXTURE_2D, cls->distance);
	if (config.use_rgba_2d_shadows) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cls->size, cls->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, cls->size, cls->height, 0, GL_RED, GL_FLOAT, NULL);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cls->distance, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//printf("errnum: %x\n",status);
	glBindFramebuffer(GL_FRAMEBUFFER, RasterizerStorageGLES3::system_fbo);

	ERR_FAIL_COND_V(status != GL_FRAMEBUFFER_COMPLETE, RID());

	return canvas_light_shadow_owner.make_rid(cls);*/
}

/* LIGHT SHADOW MAPPING */

RID RasterizerStorageVK::canvas_light_occluder_create() {
	CanvasOccluder *co = memnew(CanvasOccluder);
	co->index_id = 0;
	co->vertex_id = 0;
	co->len = 0;
	glGenVertexArrays(1, &co->array_id);

	return canvas_occluder_owner.make_rid(co);
}

void RasterizerStorageVK::canvas_light_occluder_set_polylines(RID p_occluder, const PoolVector<Vector2> &p_lines) {}

/* x */

void RasterizerStorageVK::set_debug_generate_wireframes(bool p_generate) {}

void RasterizerStorageVK::render_info_begin_capture() {}

void RasterizerStorageVK::render_info_end_capture() {}

int RasterizerStorageVK::get_captured_render_info(VS::RenderInfo p_info) {
	switch (p_info) {
		case VS::INFO_OBJECTS_IN_FRAME: {

			return info.snap.object_count;
		} break;
		case VS::INFO_VERTICES_IN_FRAME: {

			return info.snap.vertices_count;
		} break;
		case VS::INFO_MATERIAL_CHANGES_IN_FRAME: {
			return info.snap.material_switch_count;
		} break;
		case VS::INFO_SHADER_CHANGES_IN_FRAME: {
			return info.snap.shader_rebind_count;
		} break;
		case VS::INFO_SURFACE_CHANGES_IN_FRAME: {
			return info.snap.surface_switch_count;
		} break;
		case VS::INFO_DRAW_CALLS_IN_FRAME: {
			return info.snap.draw_call_count;
		} break;
		default: {
			return get_render_info(p_info);
		}
	}
}

int RasterizerStorageVK::get_render_info(VS::RenderInfo p_info) {
	switch (p_info) {
		case VS::INFO_OBJECTS_IN_FRAME:
			return info.render_final.object_count;
		case VS::INFO_VERTICES_IN_FRAME:
			return info.render_final.vertices_count;
		case VS::INFO_MATERIAL_CHANGES_IN_FRAME:
			return info.render_final.material_switch_count;
		case VS::INFO_SHADER_CHANGES_IN_FRAME:
			return info.render_final.shader_rebind_count;
		case VS::INFO_SURFACE_CHANGES_IN_FRAME:
			return info.render_final.surface_switch_count;
		case VS::INFO_DRAW_CALLS_IN_FRAME:
			return info.render_final.draw_call_count;
		case VS::INFO_USAGE_VIDEO_MEM_TOTAL:
			return 0; //no idea
		case VS::INFO_VIDEO_MEM_USED:
			return info.vertex_mem + info.texture_mem;
		case VS::INFO_TEXTURE_MEM_USED:
			return info.texture_mem;
		case VS::INFO_VERTEX_MEM_USED:
			return info.vertex_mem;
		default:
			return 0; //no idea either
	}
}

VS::InstanceType RasterizerStorageVK::get_base_type(RID p_rid) {
	if (mesh_owner.owns(p_rid)) {
		return VS::INSTANCE_MESH;
	}

	if (multimesh_owner.owns(p_rid)) {
		return VS::INSTANCE_MULTIMESH;
	}

	if (immediate_owner.owns(p_rid)) {
		return VS::INSTANCE_IMMEDIATE;
	}

	if (particles_owner.owns(p_rid)) {
		return VS::INSTANCE_PARTICLES;
	}

	if (light_owner.owns(p_rid)) {
		return VS::INSTANCE_LIGHT;
	}

	if (reflection_probe_owner.owns(p_rid)) {
		return VS::INSTANCE_REFLECTION_PROBE;
	}

	if (gi_probe_owner.owns(p_rid)) {
		return VS::INSTANCE_GI_PROBE;
	}

	return VS::INSTANCE_NONE;
}

bool RasterizerStorageVK::free(RID p_rid) {
	if (render_target_owner.owns(p_rid)) {
		RenderTarget *rt = render_target_owner.getornull(p_rid);
		_render_target_clear(rt);
		Texture *t = texture_owner.get(rt->texture);
		texture_owner.free(rt->texture);
		memdelete(t);
		render_target_owner.free(p_rid);
		memdelete(rt);

	} else if (texture_owner.owns(p_rid)) {
		// delete the texture
		Texture *texture = texture_owner.get(p_rid);
		ERR_FAIL_COND_V(texture->render_target, true); //can't free the render target texture, dude
		info.texture_mem -= texture->total_data_size;
		texture_owner.free(p_rid);
		memdelete(texture);
	} else if (sky_owner.owns(p_rid)) {
		// delete the sky
		Sky *sky = sky_owner.get(p_rid);
		sky_set_texture(p_rid, RID(), 256);
		sky_owner.free(p_rid);
		memdelete(sky);

	} else if (shader_owner.owns(p_rid)) {

		// delete the texture
		Shader *shader = shader_owner.get(p_rid);

		if (shader->shader)
			shader->shader->free_custom_shader(shader->custom_code_id);

		if (shader->dirty_list.in_list())
			_shader_dirty_list.remove(&shader->dirty_list);

		while (shader->materials.first()) {

			Material *mat = shader->materials.first()->self();

			mat->shader = NULL;
			_material_make_dirty(mat);

			shader->materials.remove(shader->materials.first());
		}

		//material_shader.free_custom_shader(shader->custom_code_id);
		shader_owner.free(p_rid);
		memdelete(shader);

	} else if (material_owner.owns(p_rid)) {

		// delete the texture
		Material *material = material_owner.get(p_rid);

		if (material->shader) {
			material->shader->materials.remove(&material->list);
		}

		if (material->ubo_id) {
			//glDeleteBuffers(1, &material->ubo_id);
		}

		//remove from owners
		for (Map<Geometry *, int>::Element *E = material->geometry_owners.front(); E; E = E->next()) {

			Geometry *g = E->key();
			g->material = RID();
		}
		for (Map<RasterizerScene::InstanceBase *, int>::Element *E = material->instance_owners.front(); E; E = E->next()) {
			RasterizerScene::InstanceBase *ins = E->key();
			if (ins->material_override == p_rid) {
				ins->material_override = RID();
			}

			for (int i = 0; i < ins->materials.size(); i++) {
				if (ins->materials[i] == p_rid) {
					ins->materials[i] = RID();
				}
			}
		}

		material_owner.free(p_rid);
		memdelete(material);

	} else if (skeleton_owner.owns(p_rid)) {

		// delete the texture
		Skeleton *skeleton = skeleton_owner.get(p_rid);
		if (skeleton->update_list.in_list()) {
			skeleton_update_list.remove(&skeleton->update_list);
		}

		for (Set<RasterizerScene::InstanceBase *>::Element *E = skeleton->instances.front(); E; E = E->next()) {
			E->get()->skeleton = RID();
		}

		skeleton_allocate(p_rid, 0, false);

		//glDeleteTextures(1, &skeleton->texture);
		skeleton_owner.free(p_rid);
		memdelete(skeleton);

	} else if (mesh_owner.owns(p_rid)) {

		// delete the texture
		Mesh *mesh = mesh_owner.get(p_rid);
		mesh->instance_remove_deps();
		mesh_clear(p_rid);

		while (mesh->multimeshes.first()) {
			MultiMesh *multimesh = mesh->multimeshes.first()->self();
			multimesh->mesh = RID();
			multimesh->dirty_aabb = true;
			mesh->multimeshes.remove(mesh->multimeshes.first());

			if (!multimesh->update_list.in_list()) {
				multimesh_update_list.add(&multimesh->update_list);
			}
		}

		mesh_owner.free(p_rid);
		memdelete(mesh);

	} else if (multimesh_owner.owns(p_rid)) {

		// delete the texture
		MultiMesh *multimesh = multimesh_owner.get(p_rid);
		multimesh->instance_remove_deps();

		if (multimesh->mesh.is_valid()) {
			Mesh *mesh = mesh_owner.getornull(multimesh->mesh);
			if (mesh) {
				mesh->multimeshes.remove(&multimesh->mesh_list);
			}
		}

		multimesh_allocate(p_rid, 0, VS::MULTIMESH_TRANSFORM_2D, VS::MULTIMESH_COLOR_NONE); //frees multimesh
		update_dirty_multimeshes();

		multimesh_owner.free(p_rid);
		memdelete(multimesh);
	} else if (immediate_owner.owns(p_rid)) {

		Immediate *immediate = immediate_owner.get(p_rid);
		immediate->instance_remove_deps();

		immediate_owner.free(p_rid);
		memdelete(immediate);
	} else if (light_owner.owns(p_rid)) {

		// delete the texture
		Light *light = light_owner.get(p_rid);
		light->instance_remove_deps();

		light_owner.free(p_rid);
		memdelete(light);

	} else if (reflection_probe_owner.owns(p_rid)) {

		// delete the texture
		ReflectionProbe *reflection_probe = reflection_probe_owner.get(p_rid);
		reflection_probe->instance_remove_deps();

		reflection_probe_owner.free(p_rid);
		memdelete(reflection_probe);

	} else if (gi_probe_owner.owns(p_rid)) {

		// delete the texture
		GIProbe *gi_probe = gi_probe_owner.get(p_rid);

		gi_probe_owner.free(p_rid);
		memdelete(gi_probe);
	} else if (gi_probe_data_owner.owns(p_rid)) {

		// delete the texture
		GIProbeData *gi_probe_data = gi_probe_data_owner.get(p_rid);

		//glDeleteTextures(1, &gi_probe_data->tex_id);
		gi_probe_owner.free(p_rid);
		memdelete(gi_probe_data);

	} else if (canvas_occluder_owner.owns(p_rid)) {

		CanvasOccluder *co = canvas_occluder_owner.get(p_rid);
		/*if (co->index_id)
			glDeleteBuffers(1, &co->index_id);
		if (co->vertex_id)
			glDeleteBuffers(1, &co->vertex_id);

		glDeleteVertexArrays(1, &co->array_id);*/

		canvas_occluder_owner.free(p_rid);
		memdelete(co);

	} else if (canvas_light_shadow_owner.owns(p_rid)) {

		CanvasLightShadow *cls = canvas_light_shadow_owner.get(p_rid);
		/*glDeleteFramebuffers(1, &cls->fbo);
		glDeleteRenderbuffers(1, &cls->depth);
		glDeleteTextures(1, &cls->distance);*/
		canvas_light_shadow_owner.free(p_rid);
		memdelete(cls);
	} else {
		return false;
	}

	return true;
}

void RasterizerStorageVK::initialize() {}

void RasterizerStorageVK::finalize() {}

bool RasterizerStorageVK::has_os_feature(const String &p_feature) {
	if (p_feature == "s3tc")
		return config.s3tc_supported;

	if (p_feature == "etc")
		return config.etc_supported;

	if (p_feature == "etc2")
		return config.etc2_supported;

	if (p_feature == "pvrtc")
		return config.pvrtc_supported;

	return false;
}

void RasterizerStorageVK::update_dirty_resources() {}

RasterizerStorageVK::RasterizerStorageVK() {}
