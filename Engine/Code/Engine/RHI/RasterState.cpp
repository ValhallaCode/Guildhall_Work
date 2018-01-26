#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITypes.hpp"

RasterState::RasterState(RHIDevice *owner, eFillMode fillType /*= SOLID*/, eCullMode cullType /*= BACK*/) 
	: device(owner)
	, dx_state(nullptr)
	, m_fill(fillType)
	, m_cull(cullType)
{
	D3D11_RASTERIZER_DESC desc;
	memset(&desc, 0, sizeof(D3D11_RASTERIZER_DESC));

	desc.FillMode = ConvertFillMode(fillType);
	desc.CullMode = ConvertCullMode(cullType);

	// Make it act like OpenGL
	desc.FrontCounterClockwise = true;

	desc.AntialiasedLineEnable = false;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.SlopeScaledDepthBias = 0.0f;
	desc.DepthClipEnable = true;
	desc.ScissorEnable = false;
	desc.MultisampleEnable = false;

	HRESULT result = device->dx_device->CreateRasterizerState(&desc, &dx_state);
	
	ASSERT_OR_DIE(SUCCEEDED(result), "Raster State Has Failed to Create!");
}

RasterState::~RasterState()
{
	dx_state->Release();
}

bool RasterState::IsValid() const
{
	return (nullptr != dx_state);
}
