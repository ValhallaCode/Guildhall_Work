#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"

ConstantBuffer::ConstantBuffer(RHIDevice* owner, const void* buffer, const size_t size)
{
	m_bufferSize = size;

	D3D11_BUFFER_DESC vb_desc;
	memset(&vb_desc, 0, sizeof(vb_desc));
	vb_desc.ByteWidth = m_bufferSize;							// How much data are we putting into this buffer
	vb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;				// What can we bind this data as (in this case, only vertex data)
	vb_desc.Usage = D3D11_USAGE_DYNAMIC;						// Hint on how this memory is used (in this case, it is immutable, or constant - can't be changed)
	vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;			// for limitations/strenghts of each, see;  
	vb_desc.StructureByteStride = m_bufferSize;					// How large is each element in this buffer
																// Next, setup the initial data (required since this is an immutable buffer - so it must be instantiated at creation time)
	D3D11_SUBRESOURCE_DATA initial_data;
	memset(&initial_data, 0, sizeof(initial_data));
	initial_data.pSysMem = buffer;
	dx_buffer = nullptr;
	owner->dx_device->CreateBuffer(&vb_desc, &initial_data, &dx_buffer);
}

bool ConstantBuffer::Update(RHIDeviceContext* context, const void* buffer)
{
	D3D11_MAPPED_SUBRESOURCE resource;

	ID3D11DeviceContext* dx_context = context->dx_deviceContext;
	if (SUCCEEDED(dx_context->Map(dx_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource)))
	{
		memcpy(resource.pData, buffer, m_bufferSize);
		dx_context->Unmap(dx_buffer, 0);
		return true;
	}
	//#TODO: Assert
	return false;
}


ConstantBuffer::~ConstantBuffer()
{
	dx_buffer->Release();
}

bool ConstantBuffer::IsValid() const
{
	return (dx_buffer != nullptr);
}
