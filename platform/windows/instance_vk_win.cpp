#include "instance_vk_win.h"

#include <vulkan/vulkan.hpp>

typedef HGLRC(APIENTRY *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int *);

Error InstanceVK_Win::initialize() {
	instance_extensions.push_back("VK_KHR_win32_surface");

	create_instance();
	ERR_EXPLAIN("Could not create vulkan instance");
	ERR_FAIL_COND_V(!instance, ERR_UNCONFIGURED);

	// Create window

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
		1,
		PFD_DRAW_TO_WINDOW | // Format Must Support Window
				PFD_SUPPORT_OPENGL | // Format Must Support OpenGL
				PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0, // Color Bits Ignored
		0, // No Alpha Buffer
		0, // Shift Bit Ignored
		0, // No Accumulation Buffer
		0, 0, 0, 0, // Accumulation Bits Ignored
		24, // 24Bit Z-Buffer (Depth Buffer)
		0, // No Stencil Buffer
		0, // No Auxiliary Buffer
		PFD_MAIN_PLANE, // Main Drawing Layer
		0, // Reserved
		0, 0, 0 // Layer Masks Ignored
	};

	hDC = GetDC(hWnd);
	if (!hDC) {
		MessageBox(NULL, "Can't Create A Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE; // Return FALSE
	}

	pixel_format = ChoosePixelFormat(hDC, &pfd);
	if (!pixel_format) // Did Windows Find A Matching Pixel Format?
	{
		MessageBox(NULL, "Can't Find A Suitable pixel_format.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE; // Return FALSE
	}

	BOOL ret = SetPixelFormat(hDC, pixel_format, &pfd);
	if (!ret) // Are We Able To Set The Pixel Format?
	{
		MessageBox(NULL, "Can't Set The pixel_format.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return ERR_CANT_CREATE; // Return FALSE
	}


	{ // Create surface
		vk::Win32SurfaceCreateInfo surface_info;
		surface_info.hinstance = GetModuleHandle(NULL);
		surface_info.hwnd = hWnd;

		surface = instance.createWin32SurfaceKHR(surface_info);
		ERR_EXPLAIN("Could not create vulkan surface");
		ERR_FAIL_COND_V(!surface, ERR_UNCONFIGURED);
	}

	return setup(); // initialize OS-independent stuff
}

int InstanceVK_Win::get_window_width() {
	return OS::get_singleton()->get_video_mode().width;
}

int InstanceVK_Win::get_window_height() {
	return OS::get_singleton()->get_video_mode().height;
}

InstanceVK_Win::InstanceVK_Win(HWND hwnd) {
	hWnd = hwnd;
}

InstanceVK_Win::~InstanceVK_Win() {}
