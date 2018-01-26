#pragma once
#include "Engine/RHI/RHI.hpp"

class StructuredBuffer
{
public:
	// Note, it takes an object size, and object_count instead
	// of the arryas total size, as object_size matters in 
	// creation!
	StructuredBuffer(RHIDevice* owner, const void* buffer, uint object_size, uint object_count);
	~StructuredBuffer();

	// New for a structured buffer!
	bool CreateViews();

	bool Update(RHIDeviceContext* context, const void* buffer);

	inline bool IsValid() const { return (dx_buffer != nullptr); }

public:
	ID3D11Buffer *dx_buffer;
	ID3D11ShaderResourceView *dx_shaderResourceView;
	RHIDevice* m_device;

	uint m_count;
	uint m_size;
	size_t m_bufferSize;
};