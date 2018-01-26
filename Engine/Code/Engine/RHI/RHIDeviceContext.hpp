#pragma once
#include "Engine/RHI/RHIDevice.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/RHI/DepthStencilState.hpp"

class RasterState;
class StructuredBuffer;
class Material;
class Shader;
class ConstantBuffer;
class Texture2D;
class Sampler;
class ShaderProgram;
class ComputeShader;
class ComputeJob;
class TextureDX;

class RHIDeviceContext
{
public:
	RHIDeviceContext(RHIDevice* owner, ID3D11DeviceContext* deviceContext);
	~RHIDeviceContext();

	// For an immediate/display context, we need the window we're rendering to
	// and the swap chain that manages the render target
	void ClearState();  // clears all rendering state
	void Flush();        // flushes all commands
	void SetRasterizer(const RasterState* raster);
	void SetBlendState(BlendState* blendState);
	void ClearColorTarget(Texture2D* output, Rgba const &color); // RGBA as float
	void ClearDepthTarget(Texture2D* output, float depth = 1.0f, uint8_t stencil = 0);
	void SetColorTarget(Texture2D* color, Texture2D* depthStencilTarget = nullptr);
	void SetDepthStencilState(DepthStencilState* dss);
	void SetStructuredBuffer(uint bind_point, StructuredBuffer *buffer);
	void SetConstantBuffer(unsigned int bufferIndex, const ConstantBuffer* buffer);
	void SetUAVConstantBuffer(unsigned int index, const ConstantBuffer* buffer);
	void UnsetConstantBuffer(unsigned int index);
	void SetTexture(unsigned int textureIndex, const TextureDX* texture);
	void UnsetTexture(unsigned int textureIndex);
	void SetUAVTexture(unsigned int index, const TextureDX* texture);
	void UnsetUAVTexture(unsigned int index);
	void SetShaderProgram(const ShaderProgram* shader);
	void SetSampler(unsigned int samplerIndex, const Sampler* sampler);
	void UnsetSampler(unsigned int index);
	void SetMaterial(Material* mat);
	void SetShader(Shader* shader);
	void DispatchComputeJob(ComputeJob* job);
	void SetComputeShader(const ComputeShader* cs_shader);
	void UnsetComputeShader();
public:
	RHIDevice* m_device;
	ID3D11DeviceContext* dx_deviceContext;
};