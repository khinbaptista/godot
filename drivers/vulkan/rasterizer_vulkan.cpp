#include "rasterizer_vulkan.h"

#include "os/os.h"
#include "project_settings.h"
#include "vulkan/vulkan.hpp"
#include <vector>

using std::vector;

RasterizerStorage *RaterizerVK::get_storage() {
	return storage;
}

RasterizerCanvas *RasterizerVK::get_canvas() {
	return canvas;
}

RasterizerScene *RasterizerVK::get_scene() {
	return scene;
}

void RasterizerVK::initialize() {
	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("Using Vulkan video driver");
	}
}

void RasterizerVK::finalize() {
}
