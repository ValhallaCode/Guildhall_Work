#include "Engine/RHI/StructuredBuffer.hpp"


StructuredBuffer::StructuredBuffer(RHIDevice* owner, const void* buffer, uint object_size, uint object_count)
{
	dx_shaderResourceView = nullptr;
	m_bufferSize = object_size * object_count;
	m_count = object_count;
	m_size = object_size;
	m_device = owner;

	// First, describe the buffer
	D3D11_BUFFER_DESC vb_desc;
	memset(&vb_desc, 0, sizeof(vb_desc));

	vb_desc.ByteWidth = (uint)m_bufferSize;                // How much data are we putting into this buffer
	vb_desc.Usage = D3D11_USAGE_DYNAMIC;                  // Hint on how this memory is used (in this case, it is immutable, or constant - can't be changed)
														  // for limitations/strenghts of each, see;  
														  //    https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
	vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// CHANGES FROM CONSTANT BUFFER!!!
	vb_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;       // What can we bind this data as (in this case, only vertex data)
	vb_desc.StructureByteStride = object_size;            // How large is each element in this buffer
	vb_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	// Next, setup the initial data (required since this is an immutable buffer - so it must be instantiated at creation time)
	D3D11_SUBRESOURCE_DATA initial_data;
	memset(&initial_data, 0, sizeof(initial_data));
	initial_data.pSysMem = buffer;

	// Finally create the vertex buffer
	dx_buffer = nullptr;
	m_device->dx_device->CreateBuffer(&vb_desc, &initial_data, &dx_buffer);
	CreateViews();
}

StructuredBuffer::~StructuredBuffer()
{
	if (dx_buffer != nullptr) { dx_buffer->Release(); }
	if (dx_shaderResourceView != nullptr) { dx_shaderResourceView->Release(); }
}

bool StructuredBuffer::CreateViews()
{
	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	desc.Buffer.ElementOffset = 0U;
	desc.Buffer.NumElements = m_count;

	HRESULT result = m_device->dx_device->CreateShaderResourceView(dx_buffer, &desc, &dx_shaderResourceView);

	return SUCCEEDED(result);
}

bool StructuredBuffer::Update(RHIDeviceContext* context, const void* buffer)
{
	D3D11_MAPPED_SUBRESOURCE resource;

	ID3D11DeviceContext *dx_context = context->dx_deviceContext;
	if (SUCCEEDED(dx_context->Map(dx_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource))) {
		memcpy(resource.pData, buffer, m_bufferSize);
		dx_context->Unmap(dx_buffer, 0);

		return true;
	}

	// ASSERT?
	return false;
}
 