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

	// viewport (is dynamic state)
	vk::PipelineViewportStateCreateInfo viewport_info;
	viewport_info.viewportCount = 0;
	viewport_info.pViewports = nullptr;
	viewport_info.scissorCount = 0;
	viewport_info.pScissors = nullptr;

	// rasterizer
	vk::PipelineRasterizationStateCreateInfo raster_info;
	raster_info.depthClampEnable = material->raster_opt.depthClampEnable;
	raster_info.rasterizerDiscardEnable = material->raster_opt.rasterizerDiscardEnable;
	raster_info.polygonMode = material->raster_opt.polygonMode;
	raster_info.lineWidth = material->raster_opt.lineWidth;
	raster_info.cullMode = material->raster_opt.cullMode;
	raster_info.frontFace = material->raster_opt.frontFace;

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
	depth_info.depthTestEnable = material->depth_stencil_opt.depthTestEnable;
	depth_info.depthWriteEnable = material->depth_stencil_opt.depthWriteEnable;
	depth_info.depthCompareOp = material->depth_stencil_opt.depthCompareOp;
	depth_info.depthBoundsTestEnable = material->depth_stencil_opt.depthBoundsTestEnable;
	depth_info.minDepthBounds = material->depth_stencil_opt.minDepthBounds;
	depth_info.maxDepthBounds = material->depth_stencil_opt.maxDepthBounds;
	depth_info.stencilTestEnable = material->depth_stencil_opt.stencilTestEnable;
	depth_info.front = material->depth_stencil_opt.front;
	depth_info.back = material->depth_stencil_opt.back;

	// color blending
	vector<vk::PipelineColorBlendAttachmentState> blend_attachments;
	for (Material::ColorBlendAttachmentOptions opt : material->attachment_opt) {
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
	};

	vk::PipelineColorBlendStateCreateInfo blend_info;
	blend_info.logicOpEnable = material->blend_opt.logicOpEnable;
	blend_info.logicOp = material->blend_opt.logicOp;
	blend_info.attachmentCount = blend_attachments.size();
	blend_info.pAttachments = blend_attachments.data();
	blend_info.blendConstants[0] = material->blend_opt.blendConstants[0];
	blend_info.blendConstants[1] = material->blend_opt.blendConstants[1];
	blend_info.blendConstants[2] = material->blend_opt.blendConstants[2];
	blend_info.blendConstants[3] = material->blend_opt.blendConstants[3];

	{ // create renderpass
		Material::DepthStencilOptions opt = material->depth_stencil_opt;

		vector<vk::AttachmentDescription> attachments;

		vk::AttachmentDescription color_attachment = {};
		color_attachment.format = InstanceVK::get_singleton()->get_swapchain_format();
		color_attachment.samples = vk::SampleCountFlagBits::e1; // not using multisampling
		color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
		color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
		color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
		color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		color_attachment.initialLayout = vk::ImageLayout::eUndefined;
		color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		attachments.push_back(color_attachment);

		vk::AttachmentReference color_ref = {};
		color_ref.attachment = 0;
		color_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentDescription depth_attachment;
		vk::AttachmentReference *depth_reference = nullptr;
		if (opt.depthTestEnable) {
			depth_attachment.format = vk_FindDepthFormat();
			depth_attachment.samples = vk::SampleCountFlagBits::e1; // not using multisampling
			depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
			depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
			depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
			depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			attachments.push_back(depth_attachment);

			vk::AttachmentReference depth_ref = {};
			depth_ref.attachment = 1;
			depth_ref.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			depth_reference = &depth_ref;
		}

		vk::SubpassDescription subpass = {};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_ref;
		subpass.pDepthStencilAttachment = depth_reference;

		vk::RenderPassCreateInfo renderpass_info = {};
		renderpass_info.attachmentCount = (uint32_t)attachments.size();
		renderpass_info.pAttachments = attachments.data();
		renderpass_info.subpassCount = 1;
		renderpass_info.pSubpasses = &subpass;
		//renderpass_info.dependencyCount = 1;
		//renderpass_info.pDependencies = &dependency;

		material->render_pass = device.createRenderPass(renderpass_info);
	}

	// dynamic state
	std::vector<vk::DynamicState> dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eLineWidth
	};

	vk::PipelineDynamicStateCreateInfo dynamic_info;
	dynamic_info.dynamicStateCount = (uint32_t)dynamic_states.size();
	dynamic_info.pDynamicStates = dynamic_states.data();

	// pipeline layout
	vk::PipelineLayoutCreateInfo layout_info;
	layout_info.setLayoutCount = 0;
	layout_info.pSetLayouts = nullptr;
	layout_info.pushConstantRangeCount = 0;
	layout_info.pPushConstantRanges = nullptr;

	material->pipeline_layout = device.createPipelineLayout(layout_info);

	// pipeline
	vk::GraphicsPipelineCreateInfo pipeline_info;
	pipeline_info.stageCount = material->shader->shader->get_stages().size();
	pipeline_info.pStages = material->shader->shader->get_stages().data();
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly_info;
	pipeline_info.pViewportState = &viewport_info;
	pipeline_info.pRasterizationState = &raster_info;
	pipeline_info.pMultisampleState = &multisample_info;
	pipeline_info.pDepthStencilState = &depth_info;
	pipeline_info.pColorBlendState = &blend_info;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = material->pipeline_layout;
	pipeline_info.renderPass = material->render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = nullptr;
	pipeline_info.basePipelineIndex = -1;

	material->pipeline = device.createGraphicsPipeline(nullptr, pipeline_info);
	ERR_EXPLAIN("Failed to create graphcis pipeline object");
	ERR_FAIL_COND(!material->pipeline);
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
