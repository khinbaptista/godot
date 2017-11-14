#include "rasterizer_storage_vulkan.h"

#include "instance_vk.h"
#include "vk_helper.h"

using std::vector;

void RasterizerStorageVK::_material_make_dirty(RasterizerStorageVK::Material *p_material) const {

	if (p_material->dirty_list.in_list())
		return;

	_material_dirty_list.add(&p_material->dirty_list);
}

RID RasterizerStorageVK::material_create() {
	Material *material = memnew(Material);
	material->topology = vk::PrimitiveTopology::eTriangleList;

	return material_owner.make_rid(material);
}

void RasterizerStorageVK::_update_material(RasterizerStorageVK::Material *material) {
	if (material->pipeline) {
		vk::Device device = InstanceVK::get_singleton()->get_device();
		device.destroyPipeline(material->pipeline);
	}

	_material_setup(material);
}

void RasterizerStorageVK::_material_setup(RasterizerStorageVK::Material *material) {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	// vertex input
	vk::PipelineVertexInputStateCreateInfo vertex_input_info;
	vertex_input_info.vertexBindingDescriptionCount = 0;

	// input assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.topology = material->topology;
	input_assembly_info.primitiveRestartEnable = false;

	// rasterizer (create with pipeline)
	vk::PipelineRasterizationStateCreateInfo raster_info;
	{
		Material::RasterizerOptions opt = material->raster_opt;

		raster_info.depthClampEnable = opt.depthClampEnable;
		raster_info.rasterizerDiscardEnable = opt.rasterizerDiscardEnable;
		raster_info.polygonMode = opt.polygonMode;
		raster_info.lineWidth = opt.lineWidth;
		raster_info.cullMode = opt.cullMode;
		raster_info.frontFace = opt.frontFace;
	}

	vk::AttachmentReference *depth_reference = nullptr;

	vk::AttachmentReference depth_ref;
	if (frame.current_rt && frame.current_rt->depth_stencil_opt.depthTestEnable) {
		depth_ref.attachment = 1;
		depth_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		depth_reference = &depth_ref;
	}

	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;
	subpass.pDepthStencilAttachment = depth_reference;
}

RasterizerStorageVK::Material::RasterizerOptions
RasterizerStorageVK::material_get_rasterizer_opt(RID rid) const {
	const Material *material = material_owner.get(rid);
	ERR_FAIL_COND_V(!material, {});

	return material->raster_opt;
}

RasterizerStorageVK::Material::DepthStencilOptions
RasterizerStorageVK::material_get_depth_opt(RID rid) const {
	const Material *material = material_owner.get(rid);
	ERR_FAIL_COND_V(!material, {});

	return material->depth_stencil_opt;
}

std::vector<RasterizerStorageVK::Material::ColorBlendAttachmentOptions>
RasterizerStorageVK::material_get_attachment_opt(RID rid) const {
	const Material *material = material_owner.get(rid);
	ERR_FAIL_COND_V(!material, {});

	return material->attachment_opt;
}

RasterizerStorageVK::Material::ColorBlendOptions
RasterizerStorageVK::material_get_blend_opt(RID rid) const {
	const Material *material = material_owner.get(rid);
	ERR_FAIL_COND_V(!material, {});

	return material->blend_opt;
}

void RasterizerStorageVK::material_set_rasterizer_opt(RID rid, Material::RasterizerOptions opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->raster_opt = opt;
	_material_make_dirty(material);
}

void RasterizerStorageVK::material_set_attachment_opt(
		RID rid,
		std::vector<Material::ColorBlendAttachmentOptions> opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->attachment_opt = opt;
	_material_make_dirty(material);
}

void RasterizerStorageVK::material_set_depth_opt(RID rid, Material::DepthStencilOptions opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->depth_stencil_opt = opt;
	_material_make_dirty(material);
}

void RasterizerStorageVK::material_set_blend_opt(RID rid, RasterizerStorageVK::Material::ColorBlendOptions opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->blend_opt = opt;
	_material_make_dirty(material);
}

void RasterizerStorageVK::material_set_render_priority(RID p_material, int priority) {}

void RasterizerStorageVK::material_set_shader(RID p_shader_material, RID p_shader) {
	Material *material = material_owner.get(p_shader_material);
	ERR_FAIL_COND(!material);

	Shader *shader = shader_owner.get(p_shader);
	ERR_FAIL_COND(!shader);

	material->shader = shader;
}

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

void RasterizerStorageVK::material_set_line_width(RID p_material, float p_width) {
	Material *material = material_owner.get(p_material);
	ERR_FAIL_COND(!material);

	material->raster_opt.lineWidth = p_width;

	_material_make_dirty(material);
}

void RasterizerStorageVK::material_set_next_pass(RID p_material, RID p_next_material) {}

bool RasterizerStorageVK::material_is_animated(RID p_material) {}

bool RasterizerStorageVK::material_casts_shadows(RID p_material) {}

void RasterizerStorageVK::material_add_instance_owner(
		RID p_material, RasterizerScene::InstanceBase *p_instance) {
}

void RasterizerStorageVK::material_remove_instance_owner(
		RID p_material, RasterizerScene::InstanceBase *p_instance) {
}
