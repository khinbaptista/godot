#include "rasterizer_storage_vulkan.h"

void RasterizerStorageGLES3::_material_make_dirty(Material *p_material) const {

	if (p_material->dirty_list.in_list())
		return;

	_material_dirty_list.add(&p_material->dirty_list);
}

RID RasterizerStorageVK::material_create() {
	Material *material = memnew(Material);
	return material_owner.make_rid(material);
}

void RasterizerStorageVK::_material_setup(Material *material) {
	// vertex input
	vk::PipelineVertexInputStateCreateInfo vertex_input_info;
	vertex_input_info.vertexBindingDescriptionCount = 0;

	// input assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.topology = topology;
	input_assembly_info.primitiveRestartEnable = false;

	// viewport
	/*vk::Extent extent = InstanceVK::get_singleton()->get_swapchain_extent();
	vk::Viewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = extent;*/

	vk::PipelineViewportStateCreateInfo viewport_info;
	//viewport_info.viewportCount = 1;
	//viewport_info.pViewports = &viewport;
	//viewport_info.scissorCount = 1;
	//viewport_info.pScissors = &scissor;
	viewport_info = frame.current_rt->viewport_info;
	viewport_ref = &frame.current_rt->viewport;

	// rasterizer
	vk::PipelineRasterizationStateCreateInfo raster_info;
	raster_info.depthClampEnable = raster_opt.depthClampEnable;
	raster_info.rasterizerDiscardEnable = raster_opt.rasterizerDiscardEnable;
	raster_info.polygonMode = raster_opt.polygonMode;
	raster_info.lineWidth = raster_opt.lineWidth;
	raster_info.cullMode = raster_opt.cullMode;
	raster_info.frontFace = raster_opt.frontFace;

	// multisampling
	vk::PipelineMultisampleStateCreateInfo multisample_info;
	multisample_info.sampleShadingEnable = false;
	multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
	// commented lines are optional attributes since we won't use it (for now?)
	//multisample_info.minSampleShading = 1.0f;
	//multisample_info.pSampleMask = nullptr;
	//multisample_info.alphaToCoverageEnable = false;
	//multisample_info.alphaToOneEnable = false;

	// depth and stencil testing
	vk::PipelineDepthStencilStateCreateInfo depth_info;
	depth_info.depthTestEnable = depth_stencil_opt.depthTestEnable;
	depth_info.depthWriteEnable = depth_stencil_opt.depthWriteEnable;
	depth_info.depthCompareOp = depth_stencil_opt.depthCompareOp;
	depth_info.depthBoundsTestEnable = depth_stencil_opt.depthBoundsTestEnable;
	depth_info.minDepthBounds = depth_stencil_opt.minDepthBounds;
	depth_info.maxDepthBounds = depth_stencil_opt.maxDepthBounds;
	depth_info.stencilTestEnable = depth_stencil_opt.stencilTestEnable;
	dpeth_info.front = depth_stencil_opt.front;
	depth_info.back = depth_stencil_opt.back;

	// color blending
	vector<vk::PipelineColorBlendAttachmentState> blend_attachments;
	for (ColorBlendAttachmentOptions opt : blend_attachment_opt) {
		vk::PipelineColorBlendAttachmentState blend_attachment;
		blend_attachment.colorWriteMask = opt.colorWriteMask;
		blend_attachment.blendEnable = opt.blendEnable;
		blend_attachment.srcColorBlendFactor = opt.srcColorBlendFactor;
		blend_attachment.dstColorBlendFactor = opt.dstColorBlendFactor;
		blend_attachment.colorBlendOp = opt.colorBlendOp;
		blend_attachment.srcAlphaBlendFactor = opt.srcAlphaBlendFactor;
		blend_attachment.dstAlphaBlendFactor = opt.dstAlphaBlendFactor;
		blend_attachment.alphaBlendOp = opt.alphaBlendOp;

		blend_attachments.push_back(blend_attachment);
	}

	vk::PipelineColorBlendStateCreateInfo blend_info;
	blend_info.logicOpEnable = blend_opt.logicOpEnable;
	blend_info.logicOp = blend_opt.logicOp;
	blend_info.attachmentCount = blend_attachments.size();
	blend_info.pAttachments = blend_attachments.data();
	blend_info.blendCostants[0] = blend_opt.blendCostants[0];
	blend_info.blendCostants[1] = blend_opt.blendCostants[1];
	blend_info.blendCostants[2] = blend_opt.blendCostants[2];
	blend_info.blendCostants[3] = blend_opt.blendCostants[3];

	// dynamic state (can we take more advantage of this?)
	std::array<vk::DynamicState, 2> dynamic_states[] = {
		vk::DynamicState::eViewport, vk::DynamicState::eLineWidth
	};

	vk::PipelineDynamicStateCreateInfo dynamic_info;
	dynamic_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_info.pDynamicStates = dynamic_states.data();

	// pipeline layout
	vk::PipelineLayoutCreateInfo layout_info;
	layout_info.setLayoutCount = 0;
	layout_info.pSetLayouts = nullptr;
	layout_info.pushConstantRangeCount = 0;
	layout_info.pPushConstantRanges = nullptr;

	pipeline_layout = device.createPipelineLayout(layout_info);

	// pipeline
	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = &shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &raster_info;
	pipeline_info.pMultisampleState = &multisample_info;
	pipeline_info.pDepthStencilState = &depth_info;
	pipeline_info.pColorBlendState = &blend_info;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = layout_info;
	pipeline_info.renderPass = InstanceVK::get_singleton()->get_render_pass();
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = nullptr;
	pipeline_info.basePipelineIndex = -1;

	pipeline = device.createGraphicsPipeline(nullptr, pipeline_info);
	ERR_EXPLAIN("Failed to create graphcis pipeline object");
	ERR_FAIL_COND(!pipeline);
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

void material_set_rasterizer_opt(RID rid, Material::RasterizerOptions opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->raster_opt = opt;
	_material_make_dirty(material);
}

void material_set_attachment_opt(
		RID rid,
		std::vector<Material::ColorBlendAttachmentOptions> opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->attachment_opt = opt;
	_material_make_dirty(material);
}

void material_set_depth_opt(RID rid, Material::DepthStencilOptions opt) {
	Material *material = material_owner.get(rid);
	ERR_FAIL_COND(!material);

	material->depth_stencil_opt = opt;
	_material_make_dirty(material);
}

void material_set_blend_opt(RID rid, Material::ColorBlendOptions opt) {
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
	ERR_FAIL_COND_V(!material);

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
