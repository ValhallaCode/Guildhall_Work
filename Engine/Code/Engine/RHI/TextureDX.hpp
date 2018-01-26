#pragma once

struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11UnorderedAccessView;
typedef unsigned int uint;

class TextureDX
{
public:
	TextureDX();
	virtual ~TextureDX();
	uint GetWidth() const;
	uint GetHeight() const;
public:
	ID3D11UnorderedAccessView* dx_uav;
	ID3D11RenderTargetView* dx_renderTarget;
	uint m_width;
	uint m_height;
	ID3D11ShaderResourceView* dx_shaderResource;
	ID3D11DepthStencilView* dx_depthStencil;
	uint dx_bindFlags;
};