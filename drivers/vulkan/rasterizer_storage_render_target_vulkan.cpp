#include "rasterizer_storage_vulkan.h"

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
	//t->gl_format_cache = 0;
	//t->gl_internal_format_cache = 0;
	//t->gl_type_cache = 0;
	t->data_size = 0;
	t->compressed = false;
	t->srgb = false;
	t->total_data_size = 0;
	t->ignore_mipmaps = false;
	t->mipmaps = 1;
	t->active = true;
	//t->tex_id = 0;
	t->render_target = rt;

	rt->texture = texture_owner.make_rid(t);

	return render_target_owner.make_rid(rt);
}

void RasterizerStorageVK::render_target_set_size(RID p_render_target, int p_width, int p_height) {
	RenderTarget *rt = render_target_owner.get(p_render_target);
	ERR_FAIL_COND(!rt);

	rt->width = p_width;
	rt->height = p_height;
}

RID RasterizerStorageVK::render_target_get_texture(RID p_render_target) const {
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
