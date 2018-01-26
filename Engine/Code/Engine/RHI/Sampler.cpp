#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include <locale>

Sampler::Sampler(RHIDevice* device)
	: m_device(device)
	, dx_sampler(nullptr)
	, m_minMipMap(0)
	, m_maxMipMap(0)
	, m_borderColor(Rgba(0,0,0,255))
	, m_anistropyAmount(1)
	, m_uWrap(eWrapMode::WRAPMODE_WRAP)
	, m_vWrap(eWrapMode::WRAPMODE_WRAP)
	, m_wWrap(eWrapMode::WRAPMODE_WRAP)
{}

Sampler::Sampler(RHIDevice* device, eFilterMode min, eFilterMode mag)
	: Sampler(device)
{
	CreateSampler(min, mag);
}

Sampler::Sampler(RHIDevice* device, eFilterMode min, eFilterMode mag, eWrapMode wrap, unsigned int anistropic, Rgba& border)
	: Sampler(device)
{
	SetWrapMode(wrap);
	SetFilterAnisotropic(anistropic);
	SetBorderColor(border);
	CreateSampler(min, mag);
}

Sampler::Sampler(RHIDevice* device, eFilterMode min, eFilterMode mag, eWrapMode uwrap, eWrapMode vwrap, eWrapMode wwrap, unsigned int anistropic, Rgba& border)
	: Sampler(device)
{
	SetWrapMode(uwrap, vwrap, wwrap);
	SetFilterAnisotropic(anistropic);
	SetBorderColor(border);
	CreateSampler(min, mag);
}

static D3D11_FILTER DXGetFilterMode(eFilterMode min, eFilterMode mag)
{
	if (min == FILTER_POINT) {
		if (mag == FILTER_POINT) {
			return D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
		else { // LINEAER
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		}
	}
	else { // assume linear
		if (mag == FILTER_POINT) {
			return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		}
		else { // assume linear
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		}
	}
}

eFilterMode ConvertStringToFilterMode(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (casedType == "filter_point")
	{
		return FILTER_POINT;
	}
	else if (casedType == "filter_linear")
	{
		return FILTER_LINEAR;
	}
	else if (casedType == "filtermode_none")
	{
		return FILTERMODE_NONE;
	}
	else if (casedType == "filtermode_anisotropic")
	{
		return FILTERMODE_ANISOTROPIC;
	}
	else
	{
		return FILTER_POINT;
	}

}

eWrapMode ConvertStringToWrapMode(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);


	if (casedType == "wrapmode_wrap")
	{
		return WRAPMODE_WRAP;
	}
	else if (casedType == "wrapmode_clamp")
	{
		return WRAPMODE_CLAMP;
	}
	else if (casedType == "wrapmode_mirror")
	{
		return WRAPMODE_MIRROR;
	}
	else if (casedType == "wrapmode_border")
	{
		return WRAPMODE_BORDER;
	}
	else
	{
		return WRAPMODE_WRAP;
	}
}

//#TODO: Ask what min and mag are
bool Sampler::CreateSampler(eFilterMode min, eFilterMode mag)
{
	D3D11_SAMPLER_DESC desc;
	memset(&desc,0,sizeof(D3D11_SAMPLER_DESC));

	desc.Filter = DXGetFilterMode(min, mag);

	if(m_uWrap == eWrapMode::WRAPMODE_BORDER)
		desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	else if(m_uWrap == eWrapMode::WRAPMODE_CLAMP)
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	else if(m_uWrap == eWrapMode::WRAPMODE_MIRROR)
		desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	else
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;

	if (m_vWrap == eWrapMode::WRAPMODE_BORDER)
		desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	else if (m_vWrap == eWrapMode::WRAPMODE_CLAMP)
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	else if (m_vWrap == eWrapMode::WRAPMODE_MIRROR)
		desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	else
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

	if (m_wWrap == eWrapMode::WRAPMODE_BORDER)
		desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	else if (m_wWrap == eWrapMode::WRAPMODE_CLAMP)
		desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	else if (m_wWrap == eWrapMode::WRAPMODE_MIRROR)
		desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	else
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	float borderColor[4];
	m_borderColor.GetAsFloats(borderColor[0], borderColor[1], borderColor[2], borderColor[3]);
	desc.BorderColor[0] = borderColor[0];
	desc.BorderColor[1] = borderColor[1];
	desc.BorderColor[2] = borderColor[2];
	desc.BorderColor[3] = borderColor[3];

	desc.MinLOD = m_minMipMap;
	desc.MaxLOD = m_maxMipMap;
	desc.MaxAnisotropy = m_anistropyAmount;

	ID3D11Device* dx_device = m_device->dx_device;
	HRESULT hr = dx_device->CreateSamplerState(&desc, &dx_sampler);

	return SUCCEEDED(hr);
}

bool Sampler::IsValid() const
{
	return (nullptr != dx_sampler);
}

void Sampler::SetWrapMode(eWrapMode uwrap, eWrapMode vwrap, eWrapMode wwrap)
{
	m_uWrap = uwrap;
	m_vWrap = vwrap;
	m_wWrap = wwrap;
}

void Sampler::SetWrapMode(eWrapMode wrap)
{
	m_uWrap = wrap;
	m_vWrap = wrap;
	m_wWrap = wrap;
}

void Sampler::SetBorderColor(const Rgba& color)
{
	m_borderColor = color;
}

void Sampler::SetFilterAnisotropic(unsigned int amount)
{
	m_anistropyAmount = amount;
}

Sampler::~Sampler()
{
	dx_sampler->Release();
}