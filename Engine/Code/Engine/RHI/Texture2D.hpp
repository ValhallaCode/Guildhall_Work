#pragma once
#include "Engine/RHI/Image.hpp"
#include "Engine/RHI/TextureDX.hpp"
#include <string>

class SimpleRenderer;
class RHIDevice;
class RHIOutput;
struct ID3D11Texture2D;

class Texture2D : public TextureDX
{
public:
	Texture2D(RHIDevice* device, ID3D11Texture2D* texture);
	Texture2D(RHIDevice* device, RHIOutput* output);
	Texture2D(RHIDevice* owner);
	Texture2D(RHIDevice* owner, const Image& img, bool is_uav = false);
	Texture2D(RHIDevice* owner, const std::string& filename, eImageFormat format = IMAGEFORMAT_RGBA8, bool is_uav = false);
	Texture2D(RHIDevice *owner, unsigned int w, unsigned int h, eImageFormat format, bool is_uav = false);
	~Texture2D();

	bool LoadFromImage(const Image& image, bool is_uav = false);
	void CreateViews();

	inline bool IsValid() const { return (dx_resource != nullptr); }
	inline bool IsRenderTarget() const { return (dx_renderTarget != nullptr); }
	inline bool IsLoaded() const { return m_loaded; }

	RHIDevice* m_device;
	ID3D11Texture2D* dx_resource;
	bool m_loaded;
};



struct load_texture_job_t
{
	std::string filename;
	Image image;
	Texture2D *texture; // texture to load into
};

typedef void(*on_texture_loaded_cb)(void *user_arg);

struct load_texture_callback_t
{
	Texture2D* texture;
	on_texture_loaded_cb cb;
	void* user_args;
};

Texture2D* LoadTextureAsync(char const *filename, SimpleRenderer* renderer, on_texture_loaded_cb cb = nullptr, void *user_arg = nullptr);
void LoadTextureFromImageJob(void* data);
void LoadImageFromFileJob(void* data);
void CallLoadTextureCallbackJob(void* data);