#pragma once

#include "servers/visual/rasterizer.h"
#include "servers/visual/shader_language.h"

class RasterizerCanvasVK;
class RasterizerSceneVK;

class RasterizerStorageVK : public RasterizerStorage {
public:
	RasterizerCanvasVK *canvas;
	RasterizerSceneVK *scene;

	struct Config {
		bool hdr_supported; // example from GLES3 renderer
	} config;
};
