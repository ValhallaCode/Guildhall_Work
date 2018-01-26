#include "Engine/RHI/TextureCube.hpp"


TextureCube::~TextureCube()
{
	delete m_texture;
	m_texture = nullptr;
}

TextureCube::TextureCube(RHIDevice* device, Image& image)
	:m_cube(new Mesh())
{
	MakeTextureBoxFromImage(device,image);
}

TextureCube::TextureCube(RHIDevice* device, std::string filePath)
	: m_cube(new Mesh())
{
	Image image;
	image.LoadFromFile(filePath.c_str());
	MakeTextureBoxFromImage(device,image);
}

void TextureCube::MakeTextureBoxFromImage(RHIDevice* device, const Image& src)
{
	unsigned int width = src.GetWidth() / 4;
	unsigned int mip_levels = 1;

	// Setup our cubemap desc
	D3D11_TEXTURE2D_DESC cube_desc;
	memset(&cube_desc, 0, sizeof(D3D11_TEXTURE2D_DESC));

	cube_desc.Width = width;
	cube_desc.Height = width;
	cube_desc.MipLevels = mip_levels;
	cube_desc.ArraySize = 6;
	cube_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	cube_desc.Usage = D3D11_USAGE_IMMUTABLE;
	cube_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	cube_desc.CPUAccessFlags = 0;
	cube_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// multisampling
	cube_desc.SampleDesc.Count = 1;
	cube_desc.SampleDesc.Quality = 0;

	// create the texture array
	// we're creating from 6 existing textures
	D3D11_SUBRESOURCE_DATA data[6];
	D3D11_SUBRESOURCE_DATA *pdata = nullptr;
	if (src.IsValid()) {
		pdata = data;
		memset(&data, 0, sizeof(D3D11_SUBRESOURCE_DATA));
		unsigned int pitch = width * 4;
		const byte_t* start = (const byte_t*)src.m_buffer;
		unsigned int total_pitch = 4 * pitch;
		unsigned int row = width * total_pitch; // how far to get to the next row

		unsigned int offsets[] = {
			(1 * row) + (2 * pitch),   // right
			(1 * row) + (0 * pitch),   // left
			(0 * row) + (1 * pitch),   // top
			(2 * row) + (1 * pitch),   // bottom
			(1 * row) + (1 * pitch),   // front
			(1 * row) + (3 * pitch)    // back
		};

		for (unsigned int i = 0; i < 6; ++i) {
			data[i].pSysMem = start + offsets[i];
			data[i].SysMemPitch = total_pitch;
		}
	}

	m_texture = new Texture2D(device, src);

	// create my resource
	if (FAILED(device->dx_device->CreateTexture2D(&cube_desc, pdata, &m_texture->dx_resource))) {
		return;
	}

	CD3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
	memset(&view_desc, 0, sizeof(CD3D11_SHADER_RESOURCE_VIEW_DESC));

	view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	view_desc.TextureCube.MostDetailedMip = 0;
	view_desc.TextureCube.MipLevels = mip_levels;

	device->dx_device->CreateShaderResourceView(m_texture->dx_resource, &view_desc, &m_texture->dx_shaderResource);
}

