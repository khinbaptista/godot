#pragma once

#include "platform_config.h"
#include <array>
#include <string>
#include <vulkan/vulkan.hpp>

#include "camera_matrix.h"
#include "hash_map.h"
#include "map.h"
#include "variant.h"

#ifndef GLSLANGVALIDATOR
#define GLSLANGVALIDATOR "glslangValidator"
#endif

/*
	This class is more a graphics pipeline / material than a shader
*/

class ShaderVK {
private:
	virtual String get_shader_name() const = 0;

	const char *vertex_code;
	const char *fragment_code;

protected:
	vk::ShaderModule vertex_module;
	vk::ShaderModule fragment_module;
	std::array<vk::PipelineShaderStageCreateInfo, 2> pipeline_stages;

	static void CompileGLSL(const std::string &filename);
	static vk::ShaderModule CreateModule(const std::vector<char> &code);

	void CreatePipelineStages();

public:
	ShaderVK();
	~ShaderVK();

	void Setup();
	void Compile();
	void DestroyModules();

	std::array<vk::PipelineShaderStageCreateInfo, 2> get_stages();
}
