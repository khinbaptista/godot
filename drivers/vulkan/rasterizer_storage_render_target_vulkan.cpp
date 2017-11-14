#include "rasterizer_storage_vulkan.h"

inline bool _use_msaa(RasterizerStorageVK::RenderTarget *rt) {
	return !rt->flags[RENDER_TARGET_NO_3D] &&
		   (!rt->flags[RENDER_TARGET_NO_3D_EFFECTS] ||
				   rt->msaa != VS::VIEWPORT_MSAA_DISABLED);
}

void RasterizerStorageVK::_render_target_allocate(RasterizerStorageVK::RenderTarget *rt) {
	if (rt->width <= 0 || rt->height <= 0)
		return;

	// multisampling
	vk::PipelineMultisampleStateCreateInfo multisample_info;
	multisample_info.sampleShadingEnable = false;
	multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;

	if (_use_msaa(rt)) {
		static const vk::SampleCountFlagBits msaa_value[] = {
			vk::SampleCountFlagBits::e1,
			vk::SampleCountFlagBits::e2,
			vk::SampleCountFlagBits::e4,
			vk::SampleCountFlagBits::e8,
			vk::SampleCountFlagBits::e16
		}

		vk::SampleCountFlagBits msaa = msaa_value[rt->msaa];

		multisample_info.sampleShadingEnable = true;
		multisample_info.rasterizationSamples = msaa;
		multisample_info.minSampleShading = 1.0f;
		multisample_info.pSampleMask = nullptr;
		multisample_info.alphaToCoverageEnable = false;
		multisample_info.alphaToOneEnable = false;
	}

	// depth and stencil testing
	vk::PipelineDepthStencilStateCreateInfo depth_info;
	{
		RenderTarget::DepthStencilOptions opt = rt->depth_stencil_opt;

		depth_info.depthTestEnable = opt.depthTestEnable;
		depth_info.depthWriteEnable = opt.depthWriteEnable;
		depth_info.depthCompareOp = opt.depthCompareOp;
		depth_info.depthBoundsTestEnable = opt.depthBoundsTestEnable;
		depth_info.minDepthBounds = opt.minDepthBounds;
		depth_info.maxDepthBounds = opt.maxDepthBounds;
		depth_info.stencilTestEnable = opt.stencilTestEnable;
		depth_info.front = opt.front;
		depth_info.back = opt.back;
	}

	// color blending
	vector<vk::PipelineColorBlendAttachmentState> blend_attachments;
	for (RenderTarget::ColorBlendAttachmentOptions opt : rt->attachment_opt) {
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
	{
		ColorBlendOptions opt = rt->blend_opt;

		blend_info.logicOpEnable = opt.logicOpEnable;
		blend_info.logicOp = opt.logicOp;
		blend_info.attachmentCount = blend_attachments.size();
		blend_info.pAttachments = blend_attachments.data();
		blend_info.blendConstants[0] = opt.blendConstants[0];
		blend_info.blendConstants[1] = opt.blendConstants[1];
		blend_info.blendConstants[2] = opt.blendConstants[2];
		blend_info.blendConstants[3] = opt.blendConstants[3];
	}

	{ // create renderpass (only right before creating pipeline)
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

		if (rt->depth_stencil_opt.depthTestEnable) {
			depth_attachment.format = vk_FindDepthFormat();
			depth_attachment.samples = vk::SampleCountFlagBits::e1; // not using multisampling
			depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
			depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
			depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
			depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			attachments.push_back(depth_attachment);
		}


		vk::RenderPassCreateInfo renderpass_info = {};
		renderpass_info.attachmentCount = (uint32_t)attachments.size();
		renderpass_info.pAttachments = attachments.data();
		renderpass_info.subpassCount = 1;
		renderpass_info.pSubpasses = &subpass;
		//renderpass_info.dependencyCount = 1;
		//renderpass_info.pDependencies = &dependency;

		material->render_pass = device.createRenderPass(renderpass_info);
	}

	/*// viewport (is dynamic state)
	vk::PipelineViewportStateCreateInfo viewport_info;
	viewport_info.viewportCount = 0;
	viewport_info.pViewports = nullptr;
	viewport_info.scissorCount = 0;
	viewport_info.pScissors = nullptr;*/

	// dynamic state
	std::vector<vk::DynamicState> dynamic_states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
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

	if (rt->width == p_width && rt->height == p_height)
		return;

	_render_target_clear(rt);
	rt->width = p_width;
	rt->height = p_height;
	_render_target_allocate(rt);
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
