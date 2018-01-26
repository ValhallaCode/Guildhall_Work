#include "Engine/RHI/Texture1D.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/EngineConfig.hpp"



Texture1D::Texture1D(RHIDevice* device, ID3D11Texture1D* texture)
	:m_device(device),
	dx_resource(texture)
{
	if (texture != nullptr)
	{
		texture->AddRef();
		dx_resource = texture;

		D3D11_TEXTURE1D_DESC desc;
		texture->GetDesc(&desc);

		m_width = desc.Width;
		m_height = 1;
		dx_bindFlags = desc.BindFlags;

		CreateViews();
	}
}

Texture1D::Texture1D(RHIDevice* device, RHIOutput* output)
	:m_device(device),
	dx_resource(nullptr)
{
	if (output != nullptr) {
		ID3D11Texture1D *backBuffer = nullptr;
		output->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture1D), (LPVOID*)&backBuffer);
		if (nullptr != backBuffer) {
			dx_resource = backBuffer;

			// Get info about it.
			D3D11_TEXTURE1D_DESC desc;
			dx_resource->GetDesc(&desc);

			// Set flags for back buffer texture
			m_width = desc.Width;
			m_height = 1;
			dx_bindFlags = desc.BindFlags;

			CreateViews();
		}
	}
}

Texture1D::Texture1D(RHIDevice* owner)
	: m_device(owner),
	dx_resource(nullptr)
{
	
}

Texture1D::Texture1D(RHIDevice* owner, const Image& img, bool is_uav /*= false*/)
	: Texture1D(owner)
{
	LoadFromImage(img, is_uav);
}

Texture1D::Texture1D(RHIDevice* owner, const std::string& filename, eImageFormat format /*= IMAGEFORMAT_RGBA8*/, bool is_uav /*= false*/)
	: Texture1D(owner)
{
	Image image;
	if (!image.LoadFromFile(filename.c_str(), format)) {
		return;
	}

	LoadFromImage(image, is_uav);
}

Texture1D::Texture1D(RHIDevice *owner, unsigned int w, eImageFormat format, bool is_uav /*= false*/)
	: Texture1D(owner)
{
	// default usage - this is going to be written to by the GPU
	D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
	if (is_uav)
		usage = D3D11_USAGE_DEFAULT;

	m_width = w;
	m_height = 1;

	DXGI_FORMAT dx_format;
	unsigned int dx_bindings = 0U;
	switch (format) {
	case IMAGEFORMAT_RGBA8:
		dx_format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dx_bindings = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		break;
	case IMAGEFORMAT_D24S8:
		// depth textures are 24-bit depth, 8-bit stencil
		dx_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dx_bindings = D3D11_BIND_DEPTH_STENCIL; // binding this to as a shader resource requires a couple extra steps - saved for later
		break;
	default:
		// ASSERTORDIE
		return;
	};

	if (is_uav)
		dx_bindings |= D3D11_BIND_UNORDERED_ACCESS;

	dx_bindFlags = dx_bindings;

	D3D11_TEXTURE1D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = m_width;
	tex_desc.MipLevels = 1;                // no mip-mapping
	tex_desc.ArraySize = 1;                // NO ARRAYS!  
	tex_desc.Usage = usage;
	tex_desc.Format = dx_format;
	tex_desc.BindFlags = dx_bindings;
	tex_desc.CPUAccessFlags = 0U;
	if (is_uav)
		tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	tex_desc.MiscFlags = 0U; // there is one for generating mip-maps.

							 // multi sampling options
	//tex_desc.SampleDesc.Count = 1;
	//tex_desc.SampleDesc.Quality = 0;
	

	// no initial data - we're creating render-able targets

	ID3D11Device *dx_device = m_device->dx_device;
	HRESULT hr = dx_device->CreateTexture1D(&tex_desc, nullptr, &dx_resource);

	if (is_uav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = 0;
		desc.Format = dx_format;

		hr = dx_device->CreateUnorderedAccessView(dx_resource, &desc, &dx_uav);
	}

	if (SUCCEEDED(hr)) {
		CreateViews();
	}
}

Texture1D::~Texture1D()
{
	if (IsValid())
	{
		dx_resource->Release();

		if (dx_renderTarget != nullptr)
			dx_renderTarget->Release();
		if (dx_depthStencil != nullptr)
			dx_depthStencil->Release();
		if (dx_uav != nullptr)
			dx_uav->Release();
	}
}

bool Texture1D::LoadFromImage(const Image& image, bool is_uav /*= false*/)
{
	m_width = image.GetWidth();
	m_height = image.GetHeight();

	ASSERT_OR_DIE(m_height == 1, "Image does not include a 1D Texture!");

	D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE;
	if (is_uav)
		usage = D3D11_USAGE_DEFAULT;

	DXGI_FORMAT dx_format;
	unsigned int dx_bindings = 0U;
	switch (image.m_format) {
	case IMAGEFORMAT_RGBA8:
		dx_format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dx_bindings = D3D11_BIND_SHADER_RESOURCE;
		break;
	case IMAGEFORMAT_D24S8:
		// depth textures are 24-bit depth, 8-bit stencil
		dx_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dx_bindings = D3D11_BIND_DEPTH_STENCIL; // binding this to as a shader resource requires a couple extra steps - saved for later
		break;
	default:
		ASSERT_OR_DIE(false, "Unsupported Image Format!");
		return false;
	};

	if (is_uav)
		dx_bindings |= D3D11_BIND_UNORDERED_ACCESS;

	// Helps us figure out what views to make
	dx_bindFlags = dx_bindings;

	// Setup initial data
	D3D11_SUBRESOURCE_DATA data;
	memset(&data, 0, sizeof(data));
	data.pSysMem = image.m_buffer;
	data.SysMemPitch = image.m_bpp * m_width;

	ID3D11Device* dx_device = m_device->dx_device;
	HRESULT hr;

	D3D11_TEXTURE1D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = image.GetWidth();
	tex_desc.MipLevels = 1;									// no mip mapping
	tex_desc.ArraySize = 1;									// NO ARRAYS!  
	tex_desc.Usage = usage;
	tex_desc.Format = dx_format;
	tex_desc.BindFlags = dx_bindings;
	tex_desc.CPUAccessFlags = 0U;
	if (is_uav)
		tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	tex_desc.MiscFlags = 0U;								// there is one for generating mip maps.

															// multi-sampling options
	//tex_desc.SampleDesc.Count = 1;
	//tex_desc.SampleDesc.Quality = 0;

	hr = dx_device->CreateTexture1D(&tex_desc, &data, &dx_resource);

	ASSERT_OR_DIE(SUCCEEDED(hr), "Loading D3D11 Resource Has Failed!");

	if (is_uav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
		desc.Texture1D.MipSlice = 0;
		desc.Format = dx_format;

		hr = dx_device->CreateUnorderedAccessView(dx_resource, &desc, &dx_uav);
	}

	if (SUCCEEDED(hr)) {
		CreateViews();
		return true;
	}
	else {
		return false;
	}
}

void Texture1D::CreateViews()
{
	ID3D11Device* device = m_device->dx_device;

	dx_renderTarget = nullptr;
	dx_shaderResource = nullptr;
	dx_depthStencil = nullptr;

	if (dx_bindFlags & D3D11_BIND_RENDER_TARGET) {
		device->CreateRenderTargetView(dx_resource, nullptr, &dx_renderTarget);
	}

	if (dx_bindFlags & D3D11_BIND_SHADER_RESOURCE) {
		device->CreateShaderResourceView(dx_resource, nullptr, &dx_shaderResource);
	}

	if (dx_bindFlags & D3D11_BIND_DEPTH_STENCIL) {
		device->CreateDepthStencilView(dx_resource, nullptr, &dx_depthStencil);
	}
}
