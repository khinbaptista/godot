#include "rasterizer_storage_vk.h"

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
