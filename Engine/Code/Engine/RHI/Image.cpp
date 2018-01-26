#include "Image.hpp"
#include "ThirdParty/stb/stb_image.h"

Image::Image()
	: m_buffer(nullptr)
	, m_width(0)
	, m_height(0)
	, m_bpp(0)
{}

Image::~Image()
{
	Destroy();
}

//------------------------------------------------------------------------
bool Image::LoadFromFile(char const *filename, eImageFormat format /*= IMAGEFORMAT_RGBA8*/)
{
	Destroy();

	int w;
	int h;
	int comp;

	// Force load in with 4 components
	stbi_uc *data = stbi_load(filename, &w, &h, &comp, 4);
	if (data != nullptr) 
	{
		m_buffer = (byte_t*)data;
		m_width = w;
		m_height = h;
		m_bpp = 4;
		m_format = format;
		return true;
	}

	return false;
}

bool Image::CreateClear(unsigned int width, unsigned int height, Rgba color)
{
	Destroy();

	m_width = width;
	m_height = height;
	m_format = IMAGEFORMAT_RGBA8;
	m_bpp = 4;

	unsigned int size = m_width * m_height * m_bpp;
	m_buffer = (byte_t*) ::malloc(size);
	
	Rgba *colors = (Rgba*)m_buffer;
	unsigned int count = m_width * m_height;
	for (unsigned int i = 0; i < count; ++i) {
		colors[i] = color;
	}

	return true;
}

//------------------------------------------------------------------------
void Image::Destroy()
{
	if (nullptr != m_buffer) {
		::free(m_buffer);
	}
}

DXGI_FORMAT Image::ConvertImageFormatToDXFormat(eImageFormat& format)
{
	if (format == eImageFormat::IMAGEFORMAT_RGBA8)
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	if (format == eImageFormat::IMAGEFORMAT_D24S8)
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	return DXGI_FORMAT_R8G8B8A8_UNORM;
}
