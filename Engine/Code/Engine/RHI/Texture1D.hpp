#pragma once
#include "Engine/RHI/Image.hpp"
#include "Engine/RHI/TextureDX.hpp"
#include <string>

class RHIDevice;
struct ID3D11Texture1D;
class RHIOutput;

class Texture1D : public TextureDX
{
public:
	Texture1D(RHIDevice* device, ID3D11Texture1D* texture);
	Texture1D(RHIDevice* device, RHIOutput* output);
	Texture1D(RHIDevice* owner);
	Texture1D(RHIDevice* owner, const Image& img, bool is_uav = false);
	Texture1D(RHIDevice* owner, const std::string& filename, eImageFormat format = IMAGEFORMAT_RGBA8, bool is_uav = false);
	Texture1D(RHIDevice *owner, unsigned int w, eImageFormat format, bool is_uav = false);
	~Texture1D();

	bool LoadFromImage(const Image& image, bool is_uav = false);
	void CreateViews();

	inline bool IsValid() const { return (dx_resource != nullptr); }
	inline bool IsRenderTarget() const { return (dx_renderTarget != nullptr); }

	RHIDevice* m_device;
	ID3D11Texture1D* dx_resource;
};