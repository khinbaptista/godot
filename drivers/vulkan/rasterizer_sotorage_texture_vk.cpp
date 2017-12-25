#include "rasterizer_storage_vulkan.h"

#include "instance_vk.h"
#include "vk_helper.h"

Ref<Image> RasterizerStorageVK::_get_vk_image_and_format(
		const Ref<Image> &p_image,
		Image::Format p_format, uint32_t p_flags,
		vk::Format &vk_format,
		bool &r_compressed,
		bool &srgb) {

	r_compressed = false;
	vk_format = vk::Format::eUndefined;
	Ref<Image> image = p_image;
	srgb = false;

	bool need_decompress = false;

	switch (p_format) {
		case Image::FORMAT_L8: {
			vk_format = vk::Format::eR8Unorm;
		} break;
		case Image::FORMAT_LA8: {
			vk_format = vk::Format::eR8G8Unorm;
		} break;

		case Image::FORMAT_R8: {
			vk_format = vk::Format::eR8Unorm;
		} break;
		case Image::FORMAT_RG8: {
			vk_format = vk::Format::eR8G8Unorm;
		} break;
		case Image::FORMAT_RGB8: {
			srgb = (config.srgb_decode_supported || (p_flags & VS::TEXTURE_FLAG_CONVERT_TO_LINEAR));
			vk_format = srgb ? vk::Format::eR8G8B8Srgb : eR8G8B8Unorm;
			srgb = true;
		} break;
		case Image::FORMAT_RGBA8: {
			srgb = (config.srgb_decode_supported || (p_flags & VS::TEXTURE_FLAG_CONVERT_TO_LINEAR));
			vk_format = srgb ? vk::Format::eR8G8B8A8Srgb : eR8G8B8A8Unorm;
			srgb = true;
		} break;
		case Image::FORMAT_RGBA4444: {
			vk_format = vk::Format::eR4G4B4A4UnormPack16;
		} break;
		case Image::FORMAT_RGBA5551: {
			vk_format = vk::Format::eR5G5B5A1UnormPack16;
		} break;

		case Image::FORMAT_RF: {
			vk_format = vk::Format::eR32Sfloat;
		} break;
		case Image::FORMAT_RGF: {
			vk_format = vk::Format::eR32G32Sfloat;
		} break;
		case Image::FORMAT_RGBF: {
			vk_format = vk::Format::eR32G32A32Sfloat;
		} break;
		case Image::FORMAT_RGBAF: {
			vk_format = vk::Format::eR32G32A32A32Sfloat;
		} break;

		case Image::FORMAT_RH: {
			vk_format = vk::Format::eR16Sfloat;
		} break;
		case Image::FORMAT_RGH: {
			vk_format = vk::Format::eR16G16Sfloat;
		} break;
		case Image::FORMAT_RGBH: {
			vk_format = vk::Format::eR16G16B16Sfloat;
		} break;
		case Image::FORMAT_RGBAH: {
			vk_format = vk::Format::eR16G16B16A16Sfloat;
		} break;

		case Image::FORMAT_RGBE9995: {
			vk_format = vk::Format::eE5B9G9R9UfloatPack32;
		} break;

		case Image::FORMAT_DXT1: need_decompress = true; break;
		case Image::FORMAT_DXT3: need_decompress = true; break;
		case Image::FORMAT_DXT5: need_decompress = true; break;

		case Image::FORMAT_RGTC_R: need_decompress = true; break;
		case Image::FORMAT_RGTC_RG: need_decompress = true; break;
		case Image::FORMAT_BPTC_RGBA: need_decompress = true; break;
		case Image::FORMAT_BPTC_RGBF: need_decompress = true; break;
		case Image::FORMAT_BPTC_RGBFU: need_decompress = true; break;

		case Image::FORMAT_PVRTC2: need_decompress = true; break;
		case Image::FORMAT_PVRTC2A: need_decompress = true; break;
		case Image::FORMAT_PVRTC4: need_decompress = true; break;
		case Image::FORMAT_PVRTC4A: need_decompress = true; break;

		case Image::FORMAT_ETC: need_decompress = true; break;
		case Image::FORMAT_ETC2_R11: need_decompress = true; break;
		case Image::FORMAT_ETC2_R11S: need_decompress = true; break;
		case Image::FORMAT_ETC2_RG11: need_decompress = true; break;
		case Image::FORMAT_ETC2_RG11S: need_decompress = true; break;
		case Image::FORMAT_ETC2_RGB8: need_decompress = true; break;
		case Image::FORMAT_ETC2_RGBA8: need_decompress = true; break;
		case Image::FORMAT_ETC2_RGB8A1: need_decompress = true; break;

		default: ERR_FAIL_V(Ref<Image>()); break;
	}

	if (need_decompress) {
		if (!image.is_null()) {
			image = image->duplicate();
			image->decompress();
			ERR_FAIL_COND_V(image->is_compressed(), image);
			image->convert(Image::FORMAT_RGBA8);
		}

		srgb = (config.srgb_decode_supported || (p_flags & VS::TEXTURE_FLAG_CONVERT_TO_LINEAR));
		vk_format = srgb ? vk::Format::eR8G8B8A8Srgb : eR8G8B8A8Unorm;
		srgb = true;
		r_compressed = false;
	}

	return image;
}

RID RasterizerStorageVK::texture_create() {
	Texture *texture = memnew(Texture);
	ERR_FAIL_COND_V(!texture, RID());

	texture->total_data_size = 0;

	return texture_owner.make_rid(texture);
}

void RasterizerStorageVK::texture_allocate(
		RID p_texture, int p_width, int p_height, Image::Format p_format, uint32_t p_flags) {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND(!texture);

	vk::Format format;

	bool compressed;
	bool srgb;

	if (p_flags & VS::TEXTURE_FLAG_USED_FOR_STREAMING) {
		p_flags &= ~VS::TEXTURE_FLAG_MIPMAPS; // no mipies for video
	}

	texture->width = p_width;
	texture->height = p_height;
	texture->format = p_format;
	texture->flags = p_flags;
	texture->stored_cube_sides = 0;

	_get_vk_image_and_format(Ref<Image>(), texture->format, texture->flags, format, compressed, srgb);

	texture->vk_format = format;
	texture->alloc_width = texture->width;
	texture->alloc_height = texture->height;

	texture->compressed = compressed;
	texture->srgb = srgb;
	texture->data_size = 0;
	texture->mipmaps = 1;

	// TODO: parameterize tiling, usage and properties
	texture->image = vk_CreateImage(
			texture->alloc_width, texture->alloc_height,
			texture->vk_format, vk::ImageTiling::eLinear,
			vk::ImageUsageFlags usage,
			vk::MemoryPropertyFlags properties,
			texture->memory);

	// TODO: parameterize aspect flags, view type,
	// baseMipLevel, levelCount, baseArrayLayer and layerCount
	texture->imageview = vk_CreateImageView(
			texture->image,
			texture->vk_format,
			vk::ImageAspectFlags,
			vk::ImageViewType, // may be cubemap
			optional uint32_t levelCount = 1,
			optional uint32_t layerCount = 1,
			optional uint32_t baseMipLevel = 0,
			optional uint32_t baseArrayLayer = 0);
}

void RasterizerStorageVK::texture_set_data(
		RID p_texture, const Ref<Image> &p_image, VS::CubeMapSide p_cube_side) {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND(!texture);

	// TODO: implement
}

Ref<Image> RasterizerStorageVK::texture_get_data(RID p_texture, VS::CubeMapSide p_cube_side) const {
	return Ref<Image>();

	// TODO: implement
}

void RasterizerStorageVK::texture_set_flags(RID p_texture, uint32_t p_flags) {}

uint32_t RasterizerStorageVK::texture_get_flags(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND_V(!texture, 0);

	return texture->flags;
}

Image::Format RasterizerStorageVK::texture_get_format(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND_V(!texture, Image::FORMAT_L8);

	return texture->format;
}

uint32_t RasterizerStorageVK::texture_get_texid(RID p_texture) const {
	ERR_EXPLAIN("Cannot get 'tex_id' while using Vulkan renderer");
	ERR_FAIL_V(0);
}

uint32_t RasterizerStorageVK::texture_get_width(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND_V(!texture, 0);

	return texture->width;
}

uint32_t RasterizerStorageVK::texture_get_height(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND_V(!texture, 0);

	return texture->height;
}

void RasterizerStorageVK::texture_set_size_override(RID p_texture, int p_width, int p_height) {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND(!texture);
	ERR_FAIL_COND(texture->render_target);

	int max_dimension = static_cast<int>(InstanceVK::get_singleton()->get_device_limits().maxImageDimension2D);
	ERR_FAIL_COND(p_width <= 0 || p_width > max_dimension);
	ERR_FAIL_COND(p_height <= 0 || p_height > max_dimension);

	//real texture size is in alloc width and height
	texture->width = p_width;
	texture->height = p_height;
}

void RasterizerStorageVK::texture_set_path(RID p_texture, const String &p_path) {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND(!texture);

	texture->path = p_path;
}

String RasterizerStorageVK::texture_get_path(RID p_texture) const {
	Texture *texture = texture_owner.get(p_texture);
	ERR_FAIL_COND_V(!texture, String());

	return texture->path;
}

void RasterizerStorageVK::texture_set_shrink_all_x2_on_set_data(bool p_enable) {
	config.shrink_textures_x2 = p_enable;
}

void RasterizerStorageVK::textures_keep_original(bool p_enable) {
	config.keep_original_textures = p_enable;
}

void RasterizerStorageVK::texture_debug_usage(List<VS::TextureInfo> *r_info) {
	List<RID> textures;
	texture_owner.get_owned_list(&textures);

	for (List<RID>::Element *E = textures.front(); E; E = E->next()) {
		Texture *t = texture_owner.get(E->get());
		if (!t) continue;

		VS::TextureInfo tinfo;
		tinfo.path = t->path;
		tinfo.format = t->format;
		tinfo.size.x = t->alloc_width;
		tinfo.size.y = t->alloc_height;
		tinfo.bytes = t->total_data_size;
		r_info->push_back(tinfo);
	}
}

RID RasterizerStorageVK::texture_create_radiance_cubemap(RID p_source, int p_resolution) const {
	Texture *texture = texture_owner.get(p_source);
	ERR_FAIL_COND_V(!texture, RID());
	ERR_FAIL_COND_V(!(texture->flags & VS::TEXTURE_FLAG_CUBEMAP), RID());

	// TODO: implement

	return RID();
}

void RasterizerStorageVK::texture_set_detect_3d_callback(
		RID p_texture,
		VisualServer::TextureDetectCallback p_callback,
		void *p_userdata) {

	// TODO: implement
}

void RasterizerStorageVK::texture_set_detect_srgb_callback(
		RID p_texture,
		VisualServer::TextureDetectCallback p_callback,
		void *p_userdata) {

	// TODO: implement
}

void RasterizerStorageVK::texture_set_detect_normal_callback(
		RID p_texture,
		VisualServer::TextureDetectCallback p_callback,
		void *p_userdata) {

	// TODO: implement
}
