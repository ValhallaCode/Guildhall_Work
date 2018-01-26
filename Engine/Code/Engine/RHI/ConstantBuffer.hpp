#pragma once
#include "Engine/RHI/DX11.hpp"
#include "Engine/Render/Vertex.hpp"
#include "Engine/RHI/RHIDevice.hpp"

class ConstantBuffer
{
public:
	ConstantBuffer(RHIDevice* owner, const void* buffer, const size_t size);
	~ConstantBuffer();
	inline bool IsValid() const;
	bool ConstantBuffer::Update(RHIDeviceContext* context, const void* buffer);

	ID3D11Buffer* dx_buffer;
	size_t m_bufferSize;
};