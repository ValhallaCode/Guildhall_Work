#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/RHI/RHIDevice.hpp"

DepthStencilState::DepthStencilState(RHIDevice* owner, const DepthStencilDesc_T& desc)
	:device(owner),
	dx_state(nullptr)
{
	D3D11_DEPTH_STENCIL_DESC dxdesc;
	memset(&dxdesc, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));

	dxdesc.DepthEnable = desc.depthWritingEnabled || desc.depthTestEnabled;
	dxdesc.DepthWriteMask = desc.depthWritingEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dxdesc.DepthFunc = desc.depthTestEnabled ? D3D11_COMPARISON_LESS : D3D11_COMPARISON_ALWAYS;
	dxdesc.StencilEnable = FALSE;

	HRESULT result = device->dx_device->CreateDepthStencilState(&dxdesc, &dx_state);
	ASSERT_OR_DIE(SUCCEEDED(result), "Depth Stencil Creation has failed!");
}

DepthStencilState::~DepthStencilState()
{
	dx_state->Release();
}
