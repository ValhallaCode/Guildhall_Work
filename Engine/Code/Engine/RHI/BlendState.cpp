#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include <locale>

D3D11_BLEND ToDXType(const eBlendFactor& factor);

BlendState::BlendState(RHIDevice *owner, bool enabled, const eBlendFactor sourceColorFactor /*= BLEND_ONE*/, const eBlendFactor destColorFactor /*= BLEND_ZERO*/)
	:m_device(owner),
	dx_state(nullptr)
{
	D3D11_BLEND_DESC desc;
	memset(&desc, 0, sizeof(D3D11_BLEND_DESC));

	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;
	
	desc.RenderTarget[0].BlendEnable = enabled;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlend = ToDXType(sourceColorFactor);
	desc.RenderTarget[0].DestBlend = ToDXType(destColorFactor);

	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT result = m_device->dx_device->CreateBlendState(&desc, &dx_state);
	if (FAILED(result))
	{
		//ASSERT
	}
}

BlendState::~BlendState()
{
	if (dx_state != nullptr) 
	{
		dx_state->Release();
		dx_state = nullptr;
	}
}

bool BlendState::IsValid() const
{
	return (nullptr != dx_state);
}

D3D11_BLEND ToDXType(const eBlendFactor& factor)
{
	switch (factor)
	{
	case BLEND_ONE:
		return D3D11_BLEND_ONE;
	case BLEND_ZERO:
		return D3D11_BLEND_ZERO;
	case BLEND_SRC_ALPHA:
		return D3D11_BLEND_SRC_ALPHA;
	case BLEND_INV_SRC_ALPHA:
		return D3D11_BLEND_INV_SRC_ALPHA;
	default:
		//ASSERT
		return D3D11_BLEND_ONE;
	}
}

eBlendFactor ConvertStringToBlendFactor(const std::string& string)
{
	std::locale local;
	std::string casedType = string;
	for (unsigned int i = 0; i < casedType.length(); ++i)
		casedType[i] = std::tolower(casedType[i], local);

	if (casedType == "blend_one")
	{
		return BLEND_ONE;
	}
	else if (casedType == "blend_zero")
	{
		return BLEND_ZERO;
	}
	else if (casedType == "blend_src_alpha")
	{
		return BLEND_SRC_ALPHA;
	}
	else if (casedType == "blend_inv_src_alpha")
	{
		return BLEND_INV_SRC_ALPHA;
	}
	else if (casedType == "blend_src_color")
	{
		return BLEND_SRC_COLOR;
	}
	else if (casedType == "blend_dest_color")
	{
		return BLEND_DEST_COLOR;
	}
	else
	{
		return BLEND_ONE;
	}
}
