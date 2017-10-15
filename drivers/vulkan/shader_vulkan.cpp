// ##################################################

#include "shader_vulkan.h"

#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <cstdio>

#include "instance_vk.h"

// ##################################################

using std::string;
using std::vector;

// ##################################################

static void write_file(const string& filename, const char* data);
static vector<char> read_file(const string &filename);
static void delete_spirv();

// ##################################################

void ShaderVK::Setup() {
	// change anything in shader code

	Compile();

	if (blend_attachment_opt.empty())
		blend_attachment_opt.push_back(ColorBlendAttachmentOptions());

	CreatePipelineLayout();

	// create attachments, subpasses and renderpasses
	// ...

	delete_spirv();
}

void ShaderVK::Compile() {
	string vertex_file = "_shader.vert";
	string fragment_file = "_shader.frag";

	write_file(vertex_file, vertex_code);
	write_file(fragment_file, fragment_code);

	CompileGLSL(vertex_file);
	CompileGLSL(fragment_file);

	remove(vertex_file);
	remove(fragment_file);
}

// ##################################################



void ShaderVK::CreatePipeline() {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	// vertex stage
	vector<char> vert_code = read_file("vert.spv");
	vk::ShaderModule vert_module = CreateModule(vert_code);

	vk::PipelineShaderStageCreateInfo vert_stage_info = {};
	vert_stage_info.stage = vk::ShaderStateFlagBits::eVertex;
	vert_stage_info.module = vert_module;
	vert_stage_info.pName = "main";

	// fragment stage
	vk::ShaderModule frag_module;
	vector<char> frag_code = read_file("frag.spv");
	frag_module = CreateModule(frag_code);

	vk::PipelineShaderStageCreateInfo frag_stage_info = {};
	frag_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
	frag_stage_info.module = frag_module;
	frag_stage_info.pNmae = "main";

	// shader stages
	vk::PipelineShaderStageCreateInfo shader_stages[] = {
		vert_stage_info, frag_stage_info
	};

	// vertex input
	vk::PipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.vertexBindingDescriptionCount = 0;

	// input assembly
	vk::PipelineInputAssemblyStateCreateInfo input_assembly_info = {};
	input_assembly_info.topology = vk::PrimitiveTopology::eTriangleList;
	input_assembly_info.primitiveRestartEnable = false;

	// viewport D:
	vk::Extent extent = InstanceVK::get_singleton()->get_swapchain_extent();

	vk::Viewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = extent;

	vk::PipelineViewportStateCreateInfo viewport_info = {};
		viewport_info.viewportCount = 1;
		viewport_info.pViewports = &viewport;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &scissor;

	// rasterizer
	vk::PipelineRasterizationStateCreateInfo raster_info = {};
		raster_info.depthClampEnable = raster_opt.depthClampEnable;
		raster_info.rasterizerDiscardEnable = raster_opt.rasterizerDiscardEnable;
		raster_info.polygonMode = raster_opt.polygonMode;
		raster_info.lineWidth = raster_opt.lineWidth;
		raster_info.cullMode = raster_opt.cullMode;
		raster_info.frontFace = raster_opt.frontFace;

	// multisampling
	vk::PipelineMultisampleStateCreateInfo multisample_info = {};
		multisample_info.sampleShadingEnable = false;
		multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
		// commented lines are optional attributes since we won't use it (for now?)
		/*multisample_info.minSampleShading = 1.0f;
		multisample_info.pSampleMask = nullptr;
		multisample_info.alphaToCoverageEnable = false;
		multisample_info.alphaToOneEnable = false;*/

	// depth and stencil testing
	vk::PipelineDepthStencilStateCreateInfo depth_info = {};
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
		vk::PipelineColorBlendAttachmentState blend_attachment = {};
		blend_attachment.colorWriteMask = opt.colorWriteMask;
		blend_attachment.blendEnable = opt.blendEnable;
		blend_attachment.srcColorBlendFactor = opt.srcColorBlendFactor;
		blend_attachment.dstColorBlendFactor = opt.dstColorBlendFactor;
		blend_attachment.colorBlendOp = opt.colorBlendOp;
		blend_attachment.srcAlphaBlendFactor = opt.srcAlphaBlendFactor;
		blend_attachment.dstAlphaBlendFactor = opt.dstAlphaBlendFactor;
		blend_attachment.alphaBlendOp = opt.alphaBlendOp;

		blend_attachments.push_back(bçend_attachment);
	}

	vk::PipelineColorBlendStateCreateInfo blend_info = {};
		blend_info.logicOpEnable = blend_opt.logicOpEnable;
		blend_info.logicOp = blend_opt.logicOp;
		blend_info.attachmentCount = blend_attachments.size();
		blend_info.pAttachments = blend_attachments.data();
		blend_info.blendCostants[0] = blend_opt[0];
		blend_info.blendCostants[1] = blend_opt[1];
		blend_info.blendCostants[2] = blend_opt[2];
		blend_info.blendCostants[3] = blend_opt[3];

	// dynamic state
	vk::DynamicState dynamic_states[] = { // can we take more advantage of this?
		vk::DynamicState::eViewport, vk::DynamicState::eLineWidth
	};

	vk::PipelineDynamicStateCreateInfo dynamic_info = {};
	dynamic_info.dynamicStateCount = 2;
	dynamic_info.pDynamicStates = dynamic_states;

	// pipeline layout
	vk::PipelineLayoutCreateInfo layout_info = {};
	layout_info.setLayoutCount = 0;
	layout_info.pSetLayouts = nullptr;
	layout_info.pushConstantRangeCount = 0;
	layout_info.pPushConstantRanges = nullptr;

	pipeline_layout = device.createPipelineLayout(layout_info);

	// pipeline
	vk::GraphicsPipelineCreateInfo pipeline_info = {};
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
}

vk::ShaderModule ShaderVK::CreateModule(const vector<char>& code) {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	vk::ShaderModuleCreateInfo module_info = {};
	module_info.codeSize = code.size();
	module_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	vk::ShaderModule module;
	module = device.createShaderModule(module_info);

	ERR_FAIL_COND_V(module == nullptr, {});

	return module;
}

// ##################################################

ShaderVK::~ShaderVK() {
	vk::Device device = InstanceVK::get_singleton()->get_device();
	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(pipeline_layout);
}

// ##################################################

void ShaderVK::CompileGLSL(const string& filename) {
	// Rely on std::system because glslang API is poorly documented (aka crap)
	string command = string(GLSLANGVALIDATOR) + " -V " + filename;
	std::system(command.c_str());
}

static void write_file(const string& filename, const char* data) {
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
	remove("vert.spv");
	remove("frag.spv");
}
