#include "Engine/RHI/IndexBuffer.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

IndexBuffer::IndexBuffer(RHIDevice* owner, const std::vector<unsigned int>& vertices, const unsigned int vertex_count, const eBufferUsage& usageType)
{
	m_indexCount = vertex_count;
	D3D11_BUFFER_DESC vb_desc;
	memset(&vb_desc, 0, sizeof(vb_desc));
	vb_desc.ByteWidth = sizeof(unsigned int) * vertex_count;		// How much data are we putting into this buffer
	vb_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;				// What can we bind this data as (in this case, only vertex data)
	if (usageType == eBufferUsage::BUFFERUSAGE_STATIC)
		vb_desc.Usage = D3D11_USAGE_IMMUTABLE;					// Hint on how this memory is used (in this case, it is immutable, or constant - can't be changed)
	if (usageType == eBufferUsage::BUFFERUSAGE_DYNAMIC)			// for limitations/strenghts of each, see;  
	{
		vb_desc.Usage = D3D11_USAGE_DYNAMIC;
		vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}
	vb_desc.StructureByteStride = sizeof(unsigned int);			// How large is each element in this buffer
																// Next, setup the initial data (required since this is an immutable buffer - so it must be instantiated at creation time)
	D3D11_SUBRESOURCE_DATA initial_data;
	memset(&initial_data, 0, sizeof(initial_data));
	initial_data.pSysMem = vertices.data();
	dx_buffer = nullptr;
	owner->dx_device->CreateBuffer(&vb_desc, &initial_data, &dx_buffer);

	m_usageType = usageType;
}

IndexBuffer::~IndexBuffer()
{
}

bool IndexBuffer::IsValid() const
{
	return (dx_buffer != nullptr);
}

bool IndexBuffer::Update(RHIDeviceContext* context, const std::vector<unsigned int>& verts, const unsigned int numberVerts)
{
	if (m_usageType != eBufferUsage::BUFFERUSAGE_DYNAMIC)
	{
		ERROR_RECOVERABLE("Warning: Trying to update a static VBO!");
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE resource;

	ID3D11DeviceContext* dx_context = context->dx_deviceContext;
	if (SUCCEEDED(dx_context->Map(dx_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0U, &resource)))
	{
		memcpy(resource.pData, &verts[0], numberVerts * sizeof(unsigned int));
		dx_context->Unmap(dx_buffer, 0);
		return true;
	}
	//#TODO: Assert
	return false;
}
