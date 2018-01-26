#pragma once
#include "Engine/RHI/RHITypes.hpp"
#include "Engine/RHI/DX11.hpp"
#include "Engine/Window.hpp"
#include "Engine/Render/Vertex.hpp"
#include <vector>


class ShaderProgram;
class RHIDeviceContext;
class VertexBuffer;
class IndexBuffer;
class RHIOutput;
struct ID3D11Device;

// Created from RenderInstance when initial output is created
class RHIDevice
{
public:
	RHIDevice();
	~RHIDevice();
	RHIDeviceContext* GetImmediateContext();
	RHIOutput* CreateOutput(Window* window/*OPTIONAL*/ /*, eRHIOutputMode const mode*/);
	RHIOutput* CreateOutput(UINT const px_width, UINT const px_height, const IntVector2& position/*OPTIONAL*/ /*, eRHIOutputMode const mode*/);
	ShaderProgram* CreateShaderFromHLSLFile(const std::string& filename);
	VertexBuffer* CreateVertexBuffer(Vertex3_PCT* vertices, unsigned int vertex_count, const eBufferUsage& usage);
	IndexBuffer* CreateIndexBuffer(const std::vector<unsigned int>& vertices, unsigned int vertex_count, const eBufferUsage& usage);

	RHIDeviceContext* m_immediateContext;
	ID3D11Device* dx_device;
};