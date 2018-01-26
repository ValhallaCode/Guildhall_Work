#pragma once
#include "Engine/RHI/DX11.hpp"
#include "Engine/Render/Vertex.hpp"
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/RHIDeviceContext.hpp"
#include <vector>

class IndexBuffer
{
public:
	IndexBuffer(RHIDevice* owner, const std::vector<unsigned int>& vertices, const unsigned int vertex_count, const eBufferUsage& usageType);
	~IndexBuffer();
	inline bool IsValid() const;
	bool Update(RHIDeviceContext* context, const std::vector<unsigned int>& verts, const unsigned int numberVerts);

	ID3D11Buffer* dx_buffer;
	eBufferUsage m_usageType;
	unsigned int m_indexCount;
};