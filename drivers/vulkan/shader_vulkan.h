#pragma once

#include "platform_config.h"
#include <vulkan/vulkan.hpp>

#include "camera_matrix.h"
#include "hash_map.h"
#include "map.h"
#include "variant.h"

class ShaderVK {
public:
	ShaderVK();

	void Setup();
}
