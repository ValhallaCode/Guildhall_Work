#pragma once
#include "Engine/RHI/DX11.hpp"
#include "Engine/Render/Vertex.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"

class VertexBuffer
{
public:
	VertexBuffer(RHIDevice* owner, const Vertex3_PCT* vertices, const unsigned int vertex_count, const eBufferUsage& usageType);
	~VertexBuffer();
	inline bool IsValid() const;
	bool Update(RHIDeviceContext* context, const Vertex3_PCT& verts, const unsigned int numberVerts);

	ID3D11Buffer* dx_buffer;
	eBufferUsage m_usageType;
	unsigned int m_vertCount;
};