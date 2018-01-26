#pragma once
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/DX11.hpp"
#include <string>

class RHIDevice;

enum eBlendFactor
{
	BLEND_ONE,
	BLEND_ZERO,
	BLEND_SRC_ALPHA,
	BLEND_INV_SRC_ALPHA,
	BLEND_SRC_COLOR,
	BLEND_DEST_COLOR,
};


class BlendState
{
public:
	BlendState(RHIDevice *owner, bool enabled, const eBlendFactor sourceColorFactor = BLEND_ONE, const eBlendFactor destColorFactor = BLEND_ZERO);
	~BlendState();
	inline bool IsValid() const;

	RHIDevice* m_device;
	ID3D11BlendState* dx_state;
};

eBlendFactor ConvertStringToBlendFactor(const std::string& string);