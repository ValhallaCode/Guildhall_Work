#pragma once
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Render/Rgba.hpp"

enum eFilterMode
{
	FILTER_POINT,
	FILTER_LINEAR,
	FILTERMODE_NONE,        // used only for mips - will select from first.
	FILTERMODE_ANISOTROPIC, // if used, must be used for all.
};

enum eWrapMode : uint8_t
{
	WRAPMODE_WRAP,
	WRAPMODE_CLAMP,
	WRAPMODE_MIRROR,
	WRAPMODE_BORDER,
};

class Sampler
{
public:
	Sampler(RHIDevice* device);
	Sampler(RHIDevice* device, eFilterMode min_filter, eFilterMode mag_filter);
	Sampler(RHIDevice* device, eFilterMode min, eFilterMode mag, eWrapMode wrap, unsigned int anistropic, Rgba& border);
	Sampler(RHIDevice* device, eFilterMode min, eFilterMode mag, eWrapMode uwrap, eWrapMode vwrap, eWrapMode wwrap, unsigned int anistropic, Rgba& border);
	~Sampler();
	bool CreateSampler(eFilterMode min_filter, eFilterMode mag_filter);
	inline bool IsValid() const;
	void SetWrapMode(eWrapMode uwrap, eWrapMode vwrap, eWrapMode wwrap);
	void SetWrapMode(eWrapMode wrap); // will set all to the same
	void SetBorderColor(const Rgba& color);
	void SetFilterAnisotropic(unsigned int amount);

	float m_minMipMap;				// 0 being most detailed.
	float m_maxMipMap;				// 0 being most detailed.  (defaults to max)
	Rgba m_borderColor;				// only used if wrap mode is border.
	unsigned int m_anistropyAmount; // default one - only applies if using anistropic filtering
	eWrapMode m_uWrap;				// How it wraps when U leaves the [0, 1] region.
	eWrapMode m_vWrap;				// ditto for V
	eWrapMode m_wWrap;				// ditto for W
	RHIDevice* m_device;
	ID3D11SamplerState* dx_sampler;
};

extern eFilterMode ConvertStringToFilterMode(const std::string& string);
extern eWrapMode ConvertStringToWrapMode(const std::string& string);