// ##################################################

#include "shader_vulkan.h"

#include <array>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "instance_vk.h"

// ##################################################

#define VERT_GLSL_FILENAME "_shader.vert"
#define FRAG_GLSL_FILENAME "_shader.frag"

#define VERT_SPIRV_FILENAME "vert.spv"
#define FRAG_SPIRV_FILENAME "frag.spv"

// ##################################################

using std::array;
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
	string vertex_file = VERT_GLSL_FILENAME;
	string fragment_file = FRAG_GLSL_FILENAME;

	write_file(vertex_file, vertex_code);
	write_file(fragment_file, fragment_code);

	CompileGLSL(vertex_file);
	CompileGLSL(fragment_file);

	remove(vertex_file);
	remove(fragment_file);
}

void ShaderVK::DestroyModules() {
	vk::Device device = InstanceVK::get_singleton()->get_device();

	if (vertex_module) {
		device.destroyShaderModule(vertex_module);
		vertex_module = {};
	}

	if (fragment_module) {
		device.destroyShaderModule(fragment_module);
		fragment_module = {};
	}
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
	vector<char> vert_code = read_file(VERT_SPIRV_FILENAME);
	vertex_module = CreateModule(vert_code);
	ERR_FAIL_COND(!vertex_module);

	vk::PipelineShaderStageCreateInfo vert_stage_info;
	vert_stage_info.stage = vk::ShaderStateFlagBits::eVertex;
	vert_stage_info.module = vertex_module;
	vert_stage_info.pName = "main"; // entry point

	// fragment stage
	vector<char> frag_code = read_file(FRAG_SPIRV_FILENAME);
	fragment_module = CreateModule(frag_code);
	ERR_FAIL_COND(!fragment_module);

	vk::PipelineShaderStageCreateInfo frag_stage_info;
	frag_stage_info.stage = vk::ShaderStageFlagBits::eFragment;
	frag_stage_info.module = fragment_module;
	frag_stage_info.pName = "main"; // entry point

	// shader stages
	pipeline_stages = { vert_stage_info, frag_stage_info };
}

/*void ShaderVK::CreateRenderPass() {
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
*/

array<vk::PipelineShaderStageCreateInfo, 2> ShaderVK::get_stages() {
	return pipeline_stages;
}

// ##################################################

ShaderVK::ShaderVK() {
	depth_test = true;
}

ShaderVK::~ShaderVK() {
	//vk::Device device = InstanceVK::get_singleton()->get_device();
	//device.destroyPipeline(pipeline);
	//device.destroyPipelineLayout(pipeline_layout);
	//device.destroyRenderPass(render_pass);

	DestroyModules()
}

// ##################################################

void ShaderVK::CompileGLSL(const string &filename) {
	// Rely on std::system because glslang API is poorly documented
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
