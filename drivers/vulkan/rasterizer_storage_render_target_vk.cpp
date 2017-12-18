#include "rasterizer_storage_vulkan.h"

#include "instance_vk.h"
#include "vk_helper.h"

using std::vector;

bool RasterizerStorageVK::_render_target_use_msaa(RasterizerStorageVK::RenderTarget *rt) {
	return !rt->flags[RENDER_TARGET_NO_3D] &&
		   (!rt->flags[RENDER_TARGET_NO_3D_EFFECTS] ||
				   rt->msaa != VS::VIEWPORT_MSAA_DISABLED);
}

void RasterizerStorageVK::_render_target_clear(RasterizerStorageVK::RenderTarget *rt) {
}

void RasterizerStorageVK::_render_target_allocate(RasterizerStorageVK::RenderTarget *rt) {
	if (rt->width <= 0 || rt->height <= 0)
		return;

	vk::Device device = InstanceVK::get_singleton()->get_device();

	vk::SampleCountFlagBits msaa_samples = vk::SampleCountFlagBits::e1;
	{ // multisampling (rt->multisample_info)
		if (_render_target_use_msaa(rt)) {
			static const vk::SampleCountFlagBits msaa_value[] = {
				vk::SampleCountFlagBits::e1,
				vk::SampleCountFlagBits::e2,
				vk::SampleCountFlagBits::e4,
				vk::SampleCountFlagBits::e8,
				vk::SampleCountFlagBits::e16
			};

			msaa_samples = msaa_value[rt->msaa];
			rt->multisample_info.sampleShadingEnable = true;
			rt->multisample_info.rasterizationSamples = msaa_samples;
			rt->multisample_info.minSampleShading = 1.0f;
			rt->multisample_info.pSampleMask = nullptr;
			rt->multisample_info.alphaToCoverageEnable = false;
			rt->multisample_info.alphaToOneEnable = false;
		} else {
			rt->multisample_info.sampleShadingEnable = false;
			rt->multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		}
	}

	{ // depth and stencil testing (rt->depth_info)
		RenderTarget::DepthStencilOptions opt = rt->depth_stencil_opt;

		rt->depth_info.depthTestEnable = opt.depthTestEnable;
		rt->depth_info.depthWriteEnable = opt.depthWriteEnable;
		rt->depth_info.depthCompareOp = opt.depthCompareOp;
		rt->depth_info.depthBoundsTestEnable = opt.depthBoundsTestEnable;
		rt->depth_info.minDepthBounds = opt.minDepthBounds;
		rt->depth_info.maxDepthBounds = opt.maxDepthBounds;
		rt->depth_info.stencilTestEnable = opt.stencilTestEnable;
		rt->depth_info.front = opt.front;
		rt->depth_info.back = opt.back;
	}

	{ // color blend attachments (rt->blend_attachments)
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

			rt->blend_attachments.push_back(blend_attachment);
		};
	}

	{ // color blend options (rt->blend_info)
		RenderTarget::ColorBlendOptions opt = rt->blend_opt;

		rt->blend_info.logicOpEnable = opt.logicOpEnable;
		rt->blend_info.logicOp = opt.logicOp;
		rt->blend_info.attachmentCount = rt->blend_attachments.size();
		rt->blend_info.pAttachments = rt->blend_attachments.data();
		rt->blend_info.blendConstants[0] = opt.blendConstants[0];
		rt->blend_info.blendConstants[1] = opt.blendConstants[1];
		rt->blend_info.blendConstants[2] = opt.blendConstants[2];
		rt->blend_info.blendConstants[3] = opt.blendConstants[3];
	}

	{ // create renderpass
		std::vector<vk::AttachmentDescription> color_attachments;
		std::vector<vk::AttachmentReference> color_attachment_references;

		vk::AttachmentDescription depth_attachment;
		vk::AttachmentReference depth_attachment_reference;

		for (int i = 0; i < rt->blend_attachments.size(); i++) {
			vk::AttachmentDescription color_attachment;
			color_attachment.format = InstanceVK::get_singleton()->get_swapchain_format();
			color_attachment.samples = msaa_samples;
			color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
			color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
			color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
			color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			color_attachment.initialLayout = vk::ImageLayout::eUndefined;
			color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

			vk::AttachmentReference color_ref;
			color_ref.attachment = i;
			color_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

			color_attachments.push_back(color_attachment);
			color_attachment_references.push_back(color_ref);
		}

		vector<vk::AttachmentDescription> attachments(color_attachments.begin(), color_attachments.end());

		if (rt->depth_stencil_opt.depthTestEnable) {
			depth_attachment.format = vk_FindDepthFormat();
			depth_attachment.samples = msaa_samples;
			depth_attachment.loadOp = vk::AttachmentLoadOp::eClear;
			depth_attachment.storeOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare; // not using stencil
			depth_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			depth_attachment.initialLayout = vk::ImageLayout::eUndefined;
			depth_attachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			depth_attachment_reference.attachment = color_attachments.size();
			depth_attachment_reference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

			attachments.push_back(depth_attachment);
		}

		vk::SubpassDescription subpass;
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = (uint32_t)color_attachment_references.size();
		subpass.pColorAttachments = color_attachment_references.data();
		subpass.pDepthStencilAttachment = &depth_attachment_reference;

		vk::SubpassDependency dependency;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		vk::RenderPassCreateInfo renderpass_info = {};
		renderpass_info.attachmentCount = (uint32_t)attachments.size();
		renderpass_info.pAttachments = attachments.data();
		renderpass_info.subpassCount = 1;
		renderpass_info.pSubpasses = &subpass;
		renderpass_info.dependencyCount = 1;
		renderpass_info.pDependencies = &dependency;

		rt->renderpass = device.createRenderPass(renderpass_info);
	}
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

void RasterizerStorageVK::render_target_clear_used(RID p_render_target) {
	RenderTarget *rt = render_target_owner.getornull(p_render_target);
	ERR_FAIL_COND(!rt);

	rt->used_in_frame = false;
}

void RasterizerStorageVK::render_target_set_msaa(RID p_render_target, VS::ViewportMSAA p_msaa) {
	RenderTarget *rt = render_target_owner.getornull(p_render_target);
	ERR_FAIL_COND(!rt);

	if (rt->msaa == p_msaa)
		return;

	_render_target_clear(rt);
	rt->msaa = p_msaa;
	_render_target_allocate(rt);
}
