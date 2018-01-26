#pragma once
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHIOutput.hpp"
#include "Engine/RHI/Image.hpp"
#include "Engine/Core/Job.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"

Texture2D::Texture2D(RHIDevice* device, ID3D11Texture2D* texture)
	:m_device(device),
	dx_resource(texture),
	m_loaded(false)
{
	if (texture != nullptr) 
	{
		texture->AddRef();
		dx_resource = texture;

		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);

		m_width = desc.Width;
		m_height = desc.Height;
		dx_bindFlags = desc.BindFlags;

		CreateViews();
	}
}

Texture2D::Texture2D(RHIDevice* device)
	: m_device(device),
	dx_resource(nullptr),
	m_loaded(false)
{}

//------------------------------------------------------------------------
Texture2D::Texture2D(RHIDevice* device, const Image& image, bool is_uav /*= false*/)
	: Texture2D(device)
{
	LoadFromImage(image, is_uav);
}

Texture2D::Texture2D(RHIDevice *owner, unsigned int w, unsigned int h, eImageFormat format, bool is_uav /*= false*/)
	: Texture2D(owner)
{
	// default usage - this is going to be written to by the GPU
	D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
	if (is_uav)
		usage = D3D11_USAGE_DEFAULT;

	m_width = w;
	m_height = h;

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

	D3D11_TEXTURE2D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = m_width;
	tex_desc.Height = m_height;
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
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;

	// no initial data - we're creating render-able targets

	ID3D11Device *dx_device = m_device->dx_device;
	HRESULT hr = dx_device->CreateTexture2D(&tex_desc, nullptr, &dx_resource);

	if (is_uav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		desc.Format = dx_format;

		hr = dx_device->CreateUnorderedAccessView(dx_resource, &desc, &dx_uav);
	}

	if (SUCCEEDED(hr)) {
		CreateViews();
	}
}

//------------------------------------------------------------------------
Texture2D::Texture2D(RHIDevice* device, const std::string& filename, eImageFormat format /*= IMAGEFORMAT_RGBA8*/, bool is_uav /*= false*/)
	: Texture2D(device)
{
	Image image;
	if (!image.LoadFromFile(filename.c_str(), format)) {
		return;
	}

	LoadFromImage(image, is_uav);

}

Texture2D::Texture2D(RHIDevice* device, RHIOutput* output)
	:m_device(device),
	dx_resource(nullptr),
	m_loaded(false)
{
	if (output != nullptr) {
		ID3D11Texture2D *backBuffer = nullptr;
		output->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
		if (nullptr != backBuffer) {
			dx_resource = backBuffer;

			// Get info about it.
			D3D11_TEXTURE2D_DESC desc;
			dx_resource->GetDesc(&desc);

			// Set flags for back buffer texture
			m_width = desc.Width;
			m_height = desc.Height;
			dx_bindFlags = desc.BindFlags;

			CreateViews();
		}
	}
}

Texture2D::~Texture2D()
{
	if(IsValid())
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

void Texture2D::CreateViews()
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

bool Texture2D::LoadFromImage(const Image& image, bool is_uav /*= false*/)
{
	m_width = image.GetWidth();
	m_height = image.GetHeight();

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

	D3D11_TEXTURE2D_DESC tex_desc;
	memset(&tex_desc, 0, sizeof(tex_desc));

	tex_desc.Width = image.GetWidth();
	tex_desc.Height = image.GetHeight();
	tex_desc.MipLevels = 1;									// no mip mapping
	tex_desc.ArraySize = 1;									// NO ARRAYS!  
	tex_desc.Usage = usage;
	tex_desc.Format = dx_format;
	tex_desc.BindFlags = dx_bindings;
	tex_desc.CPUAccessFlags = 0U;
	if (is_uav)
		tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	tex_desc.MiscFlags = 0U;								// there is one for generating mip maps.

	// multisampling options
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;

	hr = dx_device->CreateTexture2D(&tex_desc, &data, &dx_resource);

	ASSERT_OR_DIE(SUCCEEDED(hr), "Loading D3D11 Resource Has Failed!");
	
	if(is_uav)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		desc.Format = dx_format;

		hr = dx_device->CreateUnorderedAccessView(dx_resource, &desc, &dx_uav);
	}

	if (SUCCEEDED(hr)) {
		CreateViews();
		m_loaded = true;
		return true;
	}
	else {
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void LoadImageFromFileJob(void* data)
{
	load_texture_job_t* job = (load_texture_job_t*)data;
	// load the file at str into out_image.  If this fails, image will be invalid
	// and the next step will have to handle that.  One of the drawbacks of an 
	// ... 
	job->image.LoadFromFile(job->filename.c_str());
}

void LoadTextureFromImageJob(void* data)
{
	// if image is empty in this scheme, the previous step failed
	// so fill texture with something ugly, 
	// otherwise do the normal texture creation code
	// ...

	load_texture_job_t* job = (load_texture_job_t*)data;

	if (job->image.IsValid())
	{
		job->texture->LoadFromImage(job->image);
	}
	else
	{
		Image image;
		image.CreateClear(1, 1, Rgba(255, 255, 255, 255));
		job->texture->LoadFromImage(image);
	}
}

void CleanupJob(void *ptr)
{
	// job is just here to cleanup resources when we're done
	delete ptr;
}

void CallLoadTextureCallbackJob(void* data)
{
	load_texture_callback_t* call_back = (load_texture_callback_t*)data;
	call_back->cb(call_back);
}

Texture2D* LoadTextureAsync(char const *filename, SimpleRenderer* renderer, on_texture_loaded_cb cb /*= nullptr*/, void *user_arg /*= nullptr*/)
{
	auto iterate = g_loadedTextures.find(filename);
	if (iterate != g_loadedTextures.end())
	{
		TextureDX* dx_tex = iterate->second;
		return dynamic_cast<Texture2D*>(dx_tex);
	}

	Texture2D *tex = new Texture2D(renderer->m_device);

	if (!tex->IsLoaded()) {
		// alright, texture is not loaded
		// let's create the job; 

		// we new this off - since it, or parts of it, will be used by multiple threads.
		load_texture_job_t *job_data = new load_texture_job_t();
		job_data->filename = filename;
		job_data->texture = tex;

		// I'm using some existing helpers for parameter passing.  
		Job *load_image = JobCreate(JOB_GENERIC, LoadImageFromFileJob, job_data);
		Job *image_to_texture = JobCreate(JOB_RENDER, LoadTextureFromImageJob, job_data);
		image_to_texture->dependent_on(load_image);

		// who frees are resources in this scheme?
		// no one - jobs are generic, so we need a cleanup job that will
		// free the job_data 

		Job *cleanup = JobCreate(JOB_GENERIC, CleanupJob, job_data);
		cleanup->dependent_on(image_to_texture); // after this job, no one uses anything in job_data
		JobDispatchAndRelease(cleanup);

		if (cb != nullptr) {
			load_texture_callback_t* call_back = new load_texture_callback_t();
			call_back->cb = cb;
			call_back->user_args = user_arg;
			call_back->texture = tex;
			Job *cb_job = JobCreate(JOB_MAIN, CallLoadTextureCallbackJob, call_back);
			cb_job->dependent_on(image_to_texture);

			// we dispatch immediately - it won't actually 
			// run yet since image_to_texture has not been dispatched
			// and we have a dependency.
			JobDispatchAndRelease(cb_job);
		}

		// dispatch and release the final two jobs
		JobDispatchAndRelease(load_image);
		JobDispatchAndRelease(image_to_texture);
	}

	// will return whether loaded or not - up to user to 
	// check loaded state before use in this example.
	return tex;
}
