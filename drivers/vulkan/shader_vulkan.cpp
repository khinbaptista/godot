// ##################################################

#include "shader_vulkan.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "instance_vk.h"

// ##################################################

#define VERT_GLSL_FILE "_shader.vert"
#define FRAG_GLSL_FILE "_shader.frag"

#define VERT_SPIRV_FILE "vert.spv"
#define FRAG_SPIRV_FILE "frag.spv"

// ##################################################

using std::string;
using std::vector;

// ##################################################

static void write_file(const string &filename, const char *data);
static vector<char> read_file(const string &filename);
static void delete_spirv();

// ##################################################

void ShaderVK::Setup() {
	// change anything in shader code

	Compile();

	if (blend_attachment_opt.empty())
		blend_attachment_opt.push_back(ColorBlendAttachmentOptions());

	CreatePipelineStages();

	// create attachments, subpasses and renderpasses
	// ...

	delete_spirv();
}

void ShaderVK::Compile() {
	string vertex_file = VERT_GLSL_FILE;
	string fragment_file = FRAG_GLSL_FILE;

	write_file(vertex_file, vertex_code);
	write_file(fragment_file, fragment_code);

	CompileGLSL(vertex_file);
	CompileGLSL(fragment_file);

	remove(vertex_file);
	remove(fragment_file);
}

// ##################################################

vk::ShaderModule ShaderVK::CreateModule(const vector<char> &code) {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	vk::ShaderModuleCreateInfo module_info = {};
	module_info.codeSize = code.size();
	module_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

	vk::ShaderModule module;
	module = device.createShaderModule(module_info);

	return module;
}

void ShaderVK::CreatePipelineStages() {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	// vertex stage
	vector<char> vert_code = read_file("vert.spv");
	vertex_module = CreateModule(vert_code);
	ERR_FAIL_COND(!vertex_module);

	vk::PipelineShaderStageCreateInfo vert_stage_info;
	vert_stage_info.stage = vk::ShaderStateFlagBits::eVertex;
	vert_stage_info.module = vertex_module;
	vert_stage_info.pName = "main"; // entry point

	// fragment stage
	vector<char> frag_code = read_file("frag.spv");
	fragment_module = CreateModule(frag_code);
	ERR_FAIL_COND(!fragment_module);

	vk::PipelineShaderStageCreateInfo frag_stage_info;
	frag_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
	frag_stage_info.module = fragment_module;
	frag_stage_info.pName = "main"; // entry point

	// shader stages
	pipeline_stages = { vert_stage_info, frag_stage_info };

	/*
	// vertex input
	vk::PipelineVertexInputStateCreateInfo vertex_input_info;
	vertex_input_info.vertexBindingDescriptionCount = 0;

	// input assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
	input_assembly_info.topology = vk::PrimitiveTopology::eTriangleList;
	input_assembly_info.primitiveRestartEnable = false;

	// viewport D:
	vk::Extent extent = InstanceVK::get_singleton()->get_swapchain_extent();
	vk::Viewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	vk::PipelineViewportStateCreateInfo viewport_info;
	viewport_info.viewportCount = 1;
	viewport_info.pViewports = &viewport;
	viewport_info.scissorCount = 1;
	viewport_info.pScissors = &scissor;

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

	// cleanup
	device.destroyShaderModule(vert_module);
	device.destroyShaderModule(frag_module);
	*/
}

void ShaderVK::CreateRenderPass() {
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
	if (depth_test) {
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
	renderpass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderpass_info.pAttachments = attachments.data();
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpass;
	//renderpass_info.dependencyCount = 1;
	//renderpass_info.pDependencies = &dependency;

	render_pass = device.createRenderPass(renderpass_info);
}

// ##################################################

ShaderVK::ShaderVK() {
	depth_test = true;
}

ShaderVK::~ShaderVK() {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipeline_layout);
	device.destroyRenderPass(render_pass);
}

// ##################################################

void ShaderVK::CompileGLSL(const string &filename) {
	// Rely on std::system because glslang API is poorly documented (aka crap)
	string command = string(GLSLANGVALIDATOR) + " -V " + filename;
	std::system(command.c_str());
}

static void write_file(const string &filename, const char *data) {
	ofstream file;
	file.open(filename, std::ios::out);

	ERR_FAIL_COND(!file.is_open());

	file << data;
	file.close();
}

static vector<char> read_file(const string &filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	ERR_EXPLAIN("Failed to open file" + filename.c_str());
	ERR_FAIL_COND_V(!file.is_open(), {});

	size_t size = (size_t)file.tellg();
	vector<char> buffer(size);

	file.seekg(0);
	file.read(buffer.data(), size);

	file.close();
	return buffer;
}

static void delete_spirv() {
	remove(VERT_SPIRV_FILE);
	remove(FRAG_SPIRV_FILE);
}
