#include "rasterizer_vulkan.h"

#include "os/os.h"
#include "project_settings.h"

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

	storage->initialize();
	canvas->initialize();
	scene->initialize();
}

void RasterizerVK::begin_frame() {
	uint64_t tick = OS::get_singleton()->get_ticks_usec();

	double delta = double(tick - prev_ticks) / 1000000.0;
	delta *= Engine::get_singleton()->get_time_scale();

	time_total += delta;

	if (delta == 0) {
		//to avoid hiccups
		delta = 0.001;
	}

	prev_ticks = tick;

	double time_roll_over = GLOBAL_GET("rendering/limits/time/time_rollover_secs");
	if (time_total > time_roll_over)
		time_total = 0; //roll over every day (should be customz

	storage->frame.time[0] = time_total;
	storage->frame.time[1] = Math::fmod(time_total, 3600);
	storage->frame.time[2] = Math::fmod(time_total, 900);
	storage->frame.time[3] = Math::fmod(time_total, 60);
	storage->frame.count++;
	storage->frame.delta = delta;

	storage->frame.prev_tick = tick;

	storage->update_dirty_resources();

	storage->info.render_final = storage->info.render;
	storage->info.render.reset();

	scene->iteration();
}

void RasterizerVK::set_current_render_target(RID p_render_target) {
	// TODO after storage is created
}

void RasterizerVK::restore_render_target() {
	// TODO after storage is created
}

void RasterizerVK::clear_render_target(const Color &p_color) {
	ERR_FAIL_COND(!storage->frame.current_rt);

	storage->frame.clear_request = true;
	storage->frame.clear_request_color = p_color;
}

void RasterizerVK::set_boot_image(Ref<Image> &p_image, const Color &p_color, bool p_scale) {
	// TODO after storage and canvas are created
}

void RasterizerVK::blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen) {
	// TODO after storage and canvas are created
}

void RasterizerVK::end_frame() {
	OS::get_singleton()->swap_buffers(); // necessary?
}

void RasterizerVK::finalize() {
	storage->finalize();
	canvas->finalize();
	// why not scene->finalize()? (from rasterizer_gles3.cpp:399)
}

Rasterizer RasterizerVK::_create_current() {
	return memnew(RasterizerVK);
}

void RasterizerVK::make_current() {
	_create_func = _create_current;
}

void RasterizerVK::register_config() {
	// Copy-Pasted from GLES3 renderer
	GLOBAL_DEF("rendering/quality/filters/use_nearest_mipmap_filter", false);
	GLOBAL_DEF("rendering/quality/filters/anisotropic_filter_level", 4.0);
	GLOBAL_DEF("rendering/limits/time/time_rollover_secs", 3600);
}

RasterizerVK::RasterizerVK() {
	storage = memnew(RasterizerStorageVK);
	canvas = memnew(RasterizerCanvasVK);
	scene = memnew(RasterizerSceneVK);
}

RasterizerVK::~RasterizerVK() {
	memdelete(storage);
	memdelete(canvas);
	// again (before in finalize()), no scene in GLES3 renderer, must be managed elsewhere
}
