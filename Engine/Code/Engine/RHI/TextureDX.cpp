#include "Engine/RHI/TextureDX.hpp"
#include "Engine/RHI/DX11.hpp"

TextureDX::TextureDX()
	:dx_renderTarget(nullptr),
	m_width(0),
	m_height(0),
	dx_depthStencil(nullptr),
	dx_uav(nullptr)
{

}


uint TextureDX::GetWidth() const
{
	return m_width;
}

uint TextureDX::GetHeight() const
{
	return m_height;
}


TextureDX::~TextureDX()
{

}

