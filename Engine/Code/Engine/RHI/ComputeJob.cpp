#include "Engine/RHI/ComputeJob.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/ComputeShader.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/RHI/TextureDX.hpp"
#include "Engine/RHI/RHIDevice.hpp"

ComputeJob::ComputeJob()
{
	m_textures.resize(7);

	m_bufferdata.resize(5);

	m_buffers.resize(5);

	m_samplers.resize(3);
}

ComputeJob::~ComputeJob()
{
	m_buffers.clear();
	m_bufferdata.clear();
}

void ComputeJob::set_shader(ComputeShader *cs)
{
	shader_to_run = cs;
}

void ComputeJob::set_grid_dimensions(uint x, uint y, uint z /*= 1*/)
{
	m_grid.SetXYZ(x, y, z);
}

void ComputeJob::set_sampler(uint idx, Sampler* sampler)
{
	m_samplers[idx] = sampler;
}

void ComputeJob::set_resource(uint idx, TextureDX* res)
{
	m_textures[idx] = res;
}

void ComputeJob::set_rw_resource(uint idx, TextureDX const *t)
{
	m_uavIndex = idx;
	m_uav = t;
}

void ComputeJob::set_uint(uint id, uint const *v)
{
	if (m_bufferdata[id] == nullptr)
		m_bufferdata[id] = new ComputeBuffer();

	m_bufferdata[id]->integer = *v;
}

void ComputeJob::set_vec2(uint id, Vector2 const *v)
{
	if (m_bufferdata[id] == nullptr)
		m_bufferdata[id] = new ComputeBuffer();

	m_bufferdata[id]->padding = *v;
}

void ComputeJob::set_float(uint id, float const *v)
{
	if (m_bufferdata[id] == nullptr)
		m_bufferdata[id] = new ComputeBuffer();

	m_bufferdata[id]->decimal = *v;
}


void ComputeJob::set_vec4(uint id, Vector4 const *v)
{
	if (m_bufferdata[id] == nullptr)
		m_bufferdata[id] = new ComputeBuffer();

	m_bufferdata[id]->vec4 = *v;
}

void ComputeJob::set_color(uint id, Rgba const *v)
{
	if (m_bufferdata[id] == nullptr)
		m_bufferdata[id] = new ComputeBuffer();

	v->GetAsFloats(m_bufferdata[id]->color[0], m_bufferdata[id]->color[1], m_bufferdata[id]->color[2], m_bufferdata[id]->color[3]);
}

