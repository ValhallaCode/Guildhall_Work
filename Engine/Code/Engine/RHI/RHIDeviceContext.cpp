#pragma once
#include "Engine/RHI/RHIDeviceContext.hpp"
#include "Engine/RHI/RasterState.hpp"
#include "Engine/RHI/StructuredBuffer.hpp"
#include "Engine/RHI/Material.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/RHI/Shader.hpp"
#include "Engine/RHI/ComputeJob.hpp"
#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/RHI/TextureDX.hpp"

RHIDeviceContext::RHIDeviceContext(RHIDevice* owner, ID3D11DeviceContext* deviceContext)
	:m_device(owner),
	dx_deviceContext(deviceContext)
{

}

RHIDeviceContext::~RHIDeviceContext()
{
	dx_deviceContext->Release();
	dx_deviceContext = nullptr;
	m_device = nullptr;
}

void RHIDeviceContext::ClearState()
{
	dx_deviceContext->ClearState();
}

void RHIDeviceContext::Flush()
{
	dx_deviceContext->Flush();
}

void RHIDeviceContext::SetRasterizer(const RasterState* raster)
{
	dx_deviceContext->RSSetState(raster->dx_state);
}

void RHIDeviceContext::SetBlendState(BlendState* blendState)
{
	float constant[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dx_deviceContext->OMSetBlendState(blendState->dx_state, constant, 0xffffffff);
}

void RHIDeviceContext::ClearColorTarget(Texture2D* output, Rgba const &color)
{
	if (!output->dx_renderTarget)
		return;

	float colorAsFloats[4];
	color.GetAsFloats(colorAsFloats[0], colorAsFloats[1], colorAsFloats[2], colorAsFloats[3]);
	dx_deviceContext->ClearRenderTargetView(output->dx_renderTarget, colorAsFloats);
}

void RHIDeviceContext::ClearDepthTarget(Texture2D* output, float depth, uint8_t stencil)
{
	if (output != nullptr) 
	{
		dx_deviceContext->ClearDepthStencilView(output->dx_depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
}

void RHIDeviceContext::SetColorTarget(Texture2D* color, Texture2D* depthStencilTarget)
{
	dx_deviceContext->OMSetRenderTargets(1,
		&color->dx_renderTarget,
		depthStencilTarget != nullptr ? depthStencilTarget->dx_depthStencil : nullptr);
}

void RHIDeviceContext::SetDepthStencilState(DepthStencilState* dss)
{
	dx_deviceContext->OMSetDepthStencilState(dss->dx_state, 0U);
}

void RHIDeviceContext::SetStructuredBuffer(uint bind_point, StructuredBuffer *buffer)
{
	// todo: check for null, and unbind if so.
	dx_deviceContext->VSSetShaderResources(bind_point, 1, &buffer->dx_shaderResourceView);
	dx_deviceContext->PSSetShaderResources(bind_point, 1, &buffer->dx_shaderResourceView);
}

void RHIDeviceContext::SetConstantBuffer(unsigned int bufferIndex, const ConstantBuffer* buffer)
{
	dx_deviceContext->VSSetConstantBuffers(bufferIndex, 1, &buffer->dx_buffer);
	dx_deviceContext->PSSetConstantBuffers(bufferIndex, 1, &buffer->dx_buffer);
}

void RHIDeviceContext::SetUAVConstantBuffer(unsigned int index, const ConstantBuffer* buffer)
{
	dx_deviceContext->CSSetConstantBuffers(index, 1, &buffer->dx_buffer);
}

void RHIDeviceContext::UnsetConstantBuffer(unsigned int index)
{
	ID3D11Buffer* buffer = nullptr;
	dx_deviceContext->VSSetConstantBuffers(index, 1, &buffer);
	dx_deviceContext->PSSetConstantBuffers(index, 1, &buffer);
}

void RHIDeviceContext::SetTexture(unsigned int textureIndex, const TextureDX* texture)
{
	dx_deviceContext->VSSetShaderResources(textureIndex, 1, &texture->dx_shaderResource);
	dx_deviceContext->PSSetShaderResources(textureIndex, 1, &texture->dx_shaderResource);
	dx_deviceContext->CSSetShaderResources(textureIndex, 1, &texture->dx_shaderResource);
}

void RHIDeviceContext::UnsetTexture(unsigned int textureIndex)
{
	ID3D11ShaderResourceView* srv = nullptr;
	dx_deviceContext->VSSetShaderResources(textureIndex, 1, &srv);
	dx_deviceContext->PSSetShaderResources(textureIndex, 1, &srv);
}

void RHIDeviceContext::SetUAVTexture(unsigned int index, const TextureDX* texture)
{
	dx_deviceContext->CSSetUnorderedAccessViews(index, 1, &texture->dx_uav, nullptr);
}

void RHIDeviceContext::UnsetUAVTexture(unsigned int index)
{
	ID3D11UnorderedAccessView* uav = nullptr;
	dx_deviceContext->CSSetUnorderedAccessViews(index, 1, &uav, nullptr);
}

void RHIDeviceContext::SetShaderProgram(const ShaderProgram* shader)
{
	dx_deviceContext->VSSetShader(shader->dx_vertexShader, nullptr, 0U);
	dx_deviceContext->PSSetShader(shader->dx_fragmentShader, nullptr, 0U);
	dx_deviceContext->IASetInputLayout(shader->dx_inputLayout);
}

void RHIDeviceContext::SetComputeShader(const ComputeShader* cs_shader)
{
	dx_deviceContext->CSSetShader(cs_shader->dx_computeShader, nullptr, 0);
}

void RHIDeviceContext::UnsetComputeShader()
{
	ID3D11ComputeShader* shader = nullptr;
	dx_deviceContext->CSSetShader(shader, nullptr, 0);
}

void RHIDeviceContext::SetSampler(unsigned int samplerIndex, const Sampler* sampler)
{
	dx_deviceContext->VSSetSamplers(samplerIndex, 1, &sampler->dx_sampler);
	dx_deviceContext->PSSetSamplers(samplerIndex, 1, &sampler->dx_sampler);
}

void RHIDeviceContext::UnsetSampler(unsigned int index)
{
	ID3D11SamplerState* sampler = nullptr;
	dx_deviceContext->VSSetSamplers(index, 1, &sampler);
	dx_deviceContext->PSSetSamplers(index, 1, &sampler);
}

void RHIDeviceContext::SetMaterial(Material* mat)
{
	if (nullptr == mat) {
		return;
	}

	for (uint i = 0; i < mat->m_samplers.size(); ++i) {
		if(mat->m_samplers[i] != nullptr)
			SetSampler(i, mat->m_samplers[i]);
	}

	SetShader(mat->m_shader);

	for (uint i = 0; i < mat->m_textures.size(); ++i) {
		if (mat->m_textures[i] != nullptr)
			SetTexture(i, mat->m_textures[i]);
	}

	for (uint i = 0; i < mat->m_buffers.size(); ++i) {
		if (mat->m_buffers[i] != nullptr)
			SetConstantBuffer(i, mat->m_buffers[i]);
	}

	//for (uint i = 0; i < mat->property_blocks.get_count(); ++i) {
	//	PropertyBlock *block = mat->property_blocks[i];
	//	if (nullptr != block) {
	//		block->update(this);
	//		set_constant_buffer(block->get_index(), block);
	//	}
	//}
}
// Shader is just binding program and the various state objects
void RHIDeviceContext::SetShader(Shader* shader)
{
	if (shader == nullptr) {
		SetShaderProgram(nullptr);
		SetRasterizer(nullptr);
		SetDepthStencilState(nullptr);
		SetBlendState(nullptr);
	}
	else {
		SetShaderProgram(shader->m_shaderProgram);
		SetRasterizer(shader->m_raster);
		SetDepthStencilState(shader->m_depthStencil);
		SetBlendState(shader->m_blend);
	}
}

void RHIDeviceContext::DispatchComputeJob(ComputeJob* job)
{
	for (uint i = 0; i < job->m_samplers.size(); ++i) {
		if (job->m_samplers[i] != nullptr)
			SetSampler(i, job->m_samplers[i]);
	}

	for (uint i = 0; i < job->m_textures.size(); ++i) {
		if (job->m_uavIndex == i)
			continue;

		if (job->m_textures[i] != nullptr)
			SetTexture(i, job->m_textures[i]);
	}

	for (uint i = 0; i < job->m_bufferdata.size(); ++i) {
		if (job->m_bufferdata[i] == nullptr)
			continue;

		if (job->m_buffers[i] == nullptr)
		{
			job->m_buffers[i] = new ConstantBuffer(m_device, job->m_bufferdata[i], sizeof(ComputeBuffer));
			job->m_buffers[i]->Update(this, job->m_bufferdata[i]);
		}
		else
		{
			job->m_buffers[i]->Update(this, job->m_bufferdata[i]);
		}

		SetUAVConstantBuffer(i, job->m_buffers[i]);
	}

	SetUAVTexture(job->m_uavIndex, job->m_uav);
	SetComputeShader(job->shader_to_run);

	uint width = job->m_uav->GetWidth();
	uint height = job->m_uav->GetHeight();
	dx_deviceContext->Dispatch(width / job->m_grid.x, height / job->m_grid.y, 1);

	UnsetUAVTexture(job->m_uavIndex);
	UnsetComputeShader();
}