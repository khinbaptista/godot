#pragma once

#ifdef VULKAN_ENABLED

#include "drivers/vulkan/instance_vk.h"
#include "error_list.h"
#include "os/os.h"

#include <windows.h>

typedef bool(APIENTRY *PFNWGLSWAPINTERVALEXTPROC)(int interval);

class InstanceVK_Win : public InstanceVK {
	HDC hDC;
	unsigned int pixel_format;
	HWND hWnd;

public:
	virtual Error initialize();

	virtual int get_window_width();
	virtual int get_window_height();

	InstanceVK_Win(HWND hwnd);
	~InstanceVK_Win();
};

#endif //VULKAN_ENABLED
