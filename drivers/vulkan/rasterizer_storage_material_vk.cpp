#include "rasterizer_storage_vk.h"

#include "instance_vk.h"
#include "vk_helper.h"

using std::array;
using std::map;
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
	ERR_FAIL_COND(material == nullptr);

	vk::Device device = InstanceVK::get_singleton()->get_device();
	for (auto it : material->rt_pipelines) {
		device.destroyPipeline(it.second);
	}
	material->rt_pipelines.clear();

	_material_setup(material);
}

void RasterizerStorageVK::_material_setup(RasterizerStorageVK::Material *material) {
	ERR_FAIL_COND(material == nullptr);

	vk::Device device = InstanceVK::get_singleton()->get_device();

	vk::PipelineLayoutCreateInfo layout_info;
	{ // @TODO: pipeline layout
		layout_info.setLayoutCount = 0;
		layout_info.pSetLayouts = nullptr;
		layout_info.pushConstantRangeCount = 0;
		layout_info.pPushConstantRanges = nullptr;

		material->pipeline_layout = device.createPipelineLayout(layout_info);
	}

	{ // fill 'raster_info' based on 'material->raster_opt'
		auto opt = material->raster_opt;
		material->raster_info.depthClampEnable = opt.depthClampEnable;
		material->raster_info.rasterizerDiscardEnable = opt.rasterizerDiscardEnable;
		material->raster_info.polygonMode = opt.polygonMode;
		material->raster_info.lineWidth = opt.lineWidth;
		material->raster_info.cullMode = opt.cullMode;
		material->raster_info.frontFace = opt.frontFace;
	}

	_material_create_pipeline(material, frame.current_rt);
}

void RasterizerStorageVK::_material_create_pipeline(RasterizerStorageVK::Material *material, RasterizerStorageVK::RenderTarget *rt) {
	ERR_FAIL_COND(rt == nullptr);
	ERR_FAIL_COND(material == nullptr);

	vk::Device device = InstanceVK::get_singleton()->get_device();

	if (material->rt_pipelines.count(rt) > 0) {
		if (material->rt_pipelines[rt])
			device.destroyPipeline(material->rt_pipelines[rt]);

		material->rt_pipelines.erase(rt);
	}

	// @TODO: vertex input from material
	vk::PipelineVertexInputStateCreateInfo vertex_input_info;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = nullptr;
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = nullptr;

	// input assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.topology = material->topology;
	input_assembly_info.primitiveRestartEnable = false;

	// get shader stages from material shader
	std::array<vk::PipelineShaderStageCreateInfo, 2>
			shader_stages = material->shader->shader->get_stages();

	// dynamic state
	std::vector<vk::DynamicState> dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eLineWidth
	};

	vk::PipelineDynamicStateCreateInfo dynamic_info;
	dynamic_info.dynamicStateCount = (uint32_t)dynamic_states.size();
	dynamic_info.pDynamicStates = dynamic_states.data();

	// viewport (is dynamic state)
	vk::PipelineViewportStateCreateInfo viewport_info;
	viewport_info.viewportCount = 0;
	viewport_info.pViewports = nullptr;
	viewport_info.scissorCount = 0;
	viewport_info.pScissors = nullptr;

	vk::GraphicsPipelineCreateInfo pipeline_info;
	{
		pipeline_info.stageCount = (uint32_t)shader_stages.size();
		pipeline_info.pStages = shader_stages.data();
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly_info;
		pipeline_info.pViewportState = &viewport_info;
		pipeline_info.pRasterizationState = &material->raster_info;
		pipeline_info.pMultisampleState = &rt->multisample_info;
		pipeline_info.pDepthStencilState = &rt->depth_info;
		pipeline_info.pColorBlendState = &rt->blend_info;
		pipeline_info.pDynamicState = &dynamic_info;
		pipeline_info.layout = material->pipeline_layout;
		pipeline_info.renderPass = rt->renderpass;
		pipeline_info.subpass = 0;
		pipeline_info.basePipelineHandle = nullptr;
		pipeline_info.basePipelineIndex = -1;
	}

	material->rt_pipelines[rt] = device.createGraphicsPipeline(nullptr, pipeline_info);
	ERR_EXPLAIN("Failed to create graphcis pipeline object");
	ERR_FAIL_COND(!material->rt_pipelines[rt]);
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
