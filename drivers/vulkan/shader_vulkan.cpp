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

static vector<char> read_file(const string &filename);
static void delete_spirv();

// ##################################################

void ShaderVK::CreatePipelineLayout() {
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
	vk::PipelineShaderStageCreateInfo stages[] = {
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
	vk::PipelineColorBlendAttachmentState blend_attachment = {};
		blend_attachment.colorWriteMask = blend_attachment_opt.colorWriteMask;
		blend_attachment.blendEnable = blend_attachment_opt.blendEnable;
		blend_attachment.srcColorBlendFactor = blend_attachment_opt.srcColorBlendFactor;
		blend_attachment.dstColorBlendFactor = blend_attachment_opt.dstColorBlendFactor;
		blend_attachment.colorBlendOp = blend_attachment_opt.colorBlendOp;
		blend_attachment.srcAlphaBlendFactor = blend_attachment_opt.srcAlphaBlendFactor;
		blend_attachment.dstAlphaBlendFactor = blend_attachment_opt.dstAlphaBlendFactor;
		blend_attachment.alphaBlendOp = blend_attachment_opt.alphaBlendOp;

	vk::PipelineColorBlendStateCreateInfo blend_info = {};
		blend_info.logicOpEnable = blend_opt.logicOpEnable;
		blend_info.logicOp = blend_opt.logicOp;
		blend_info.attachmentCount = 1;
		blend_info.pAttachments = &blend_attachment;
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

void ShaderVK::CompileGLSL(const string& filename) {
	// Rely on std::system because glslang API is poorly documented (aka crap)
	string command = string(GLSLANGVALIDATOR) + " -V " + filename;
	std::system(command.c_str());
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
