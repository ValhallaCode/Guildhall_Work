#pragma once
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHITypes.hpp"

class RHIDevice;

struct DepthStencilDesc_T
{
	bool depthWritingEnabled; // if writing a pixel - also write a depth
	bool depthTestEnabled;    // only write a pixel if depth written is less than or equal current current depth
};

class DepthStencilState
{
public:
	DepthStencilState(RHIDevice* owner, const DepthStencilDesc_T& desc);
	~DepthStencilState();
	inline bool IsValid() const { return (nullptr != dx_state); }

	RHIDevice *device;
	ID3D11DepthStencilState *dx_state;
};