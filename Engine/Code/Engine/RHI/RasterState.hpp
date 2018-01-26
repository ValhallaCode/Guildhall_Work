#pragma once
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/DX11.hpp"

class RHIDevice;

class RasterState
{
public:
	RasterState(RHIDevice *owner, eFillMode fillType = SOLID, eCullMode cullType = BACK);
	~RasterState();
	inline bool IsValid() const;

	RHIDevice *device;
	ID3D11RasterizerState *dx_state;
	eFillMode m_fill;
	eCullMode m_cull;
};
