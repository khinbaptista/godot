#pragma once

#include "platform_config.h"
#include <vulkan/vulkan.hpp>
#include <string>

#include "camera_matrix.h"
#include "hash_map.h"
#include "map.h"
#include "variant.h"

#ifndef GLSLANGVALIDATOR
#define GLSLANGVALIDATOR "glslangValidator"
#endif

/*
	This class is more a graphics pipeline than a shader
*/

class ShaderVK {
private:
	virtual String get_shader_name() const = 0;

protected:
	struct RasterizerOptions {
		vk::PolygonMode polygonMode;
		vk::CullModeFlags cullMode;
		vk::FrontFace frontFace;
		float lineWidth;
		bool depthClampEnable;
		bool rasterizerDiscardEnable;

		RasterizerOptions() {
			polygonMode = vk::PolygonMode::eFill;
			cullMode = vk::CullModeFlagBits::eBack;
			frontFace = vk::FrontFace::eClockwise;
			lineWidth = 1.0f;
			depthClampEnable = false;
			rasterizerDiscardEnable = false;
		}
	};
	RasterizerOptions raster_opt;

	struct DepthStencilOptions {
		bool depthTestEnable;
		bool depthWriteEnable; // useful for transparent materials
		vk::CompareOp depthCompareOp;
		bool depthBoundsTestEnable;
		float minDepthBounds;
		float maxDepthBounds;

		bool stencilTestEnable;
		vk::StencilOpState front;
		vk::StencilOpState back;

		DepthStencilOptions() {
			depthTestEnable = true;
			depthWriteEnable = true;
			depthCompareOp = vk::CompareOp::eLess;
			depthBoundsTestEnable = false;
			minDepthBounds = 0.0f;
			maxDepthBounds = 1.0f;

			stencilTestEnable = false;
			front = {};
			back = {};
		}
	};
	DepthStencilOptions depth_stencil_opt;

	struct ColorBlendAttachmentOptions {
		bool blendEnable;
		vk::ColorComponentFlags colorWriteMask;

		vk::BlendFactor srcColorBlendFactor;
		vk::BlendFactor dstColorBlendFactor;
		vk::BlendOp colorBlendOp;

		vk::BlendFactor srcAlphaBlendFactor;
		vk::BlendFactor dstAlphaBlendFactor;
		vk::BlendOp alphaBlendOp;

		ColorBlendAttachmentOptions() {
			blendEnable = false;
			colorWriteMask =
				vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA;
			srcColorBlendFactor = vk::BlendFactor::eOne;
			dstColorBlendFactor = vk::BlendFactor::eZero;
			colorBlendOp = vk::BlendOp::eAdd;
			srcAlphaBlendFactor = vk::BlendFactor::eOne;
			dstAlphaBlendFactor = vk::BlendFactor::eZero;
			alphaBlendOp = vk::BlendOp::eAdd;
		}
	};
	ColorBlendAttachmentOptions blend_attachment_opt;

	struct ColorBlendOptions {
		bool logicOpEnable;
		vk::LogicOp logicOp;
		float blendCostants[4];

		ColorBlendOptions() {
			logicOpEnable = false;
			logicOp = vk::LogicOp::eCopy;
			blendCostants[0] = 0.0f;
			blendCostants[1] = 0.0f;
			blendCostants[2] = 0.0f;
			blendCostants[3] = 0.0f;
		}
	};
	ColorBlendOptions blend_opt;

	vk::PipelineLayout pipeline_layout;

	static void CompileGLSL(const std::string& filename);
	static vk::ShaderModule CreateModule(const std::vector<char>& code);

	void CreatePipelineLayout();

public:
	ShaderVK();

	void Setup();
	void Compile();
}
