#pragma once
#include "Engine/Math/UintVector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Vector2.hpp"
#include <vector>

class ComputeShader;
class Sampler;
class Rgba;
class TextureDX;
class ConstantBuffer;
class RHIDevice;

struct ComputeBuffer
{
	float color[4];
	Vector4 vec4;
	float decimal;
	uint integer;
	Vector2 padding;
};

//////////////////////////////////////////////////////////////////////////
//   #TODO: Replace Constant Buffer with Property Blocks, and           //
//   Texture2D with RHIResource!                                        //
//////////////////////////////////////////////////////////////////////////

class ComputeJob
{
public:
	ComputeJob();
	~ComputeJob();

	// Sets the shader program to use for this job;
	void set_shader(ComputeShader *cs);

	// dispach grid - how many groups do we dispatch?
	// These arguments are just forwarded to ID3D11DeviceContext::Dispatch
	// when we run ("dispatch") this job.
	void set_grid_dimensions(uint x, uint y, uint z = 1);

	//void set_resource(uint idx, R<RHIResource> res);
	void set_sampler(uint idx, Sampler* sampler); 
	void set_resource(uint idx, TextureDX* res);

	// Set a UA resource view (bind points use register(u#) in the sahder)
	// I use "rw" for read/write resource - since I think it reads better than UA (unordered access)
	void set_rw_resource(uint idx, TextureDX const *t);


	// Sets the properties for this program by name
	// This is identical to proper blocks for Materials [optional task that was done
	// in AES].  This is not required for the assignment - but does help make the 
	// system more useable.
	//
	// If you have materials working in your engine - I recommend doing this step.  But since
	// it requires using shader reflection to have it work by property name, I recommend saving it for last if
	// you have not yet done it for a material class.
	/*void set_uint(uint id, uint const *v);
	void set_float(uint id, float const *v);*/
	void set_vec4(uint id, Vector4 const *v);
	void set_color(uint id, Rgba const *v);
	void set_uint(uint id, uint const *v);
	void set_vec2(uint id, Vector2 const *v);
	void set_float(uint id, float const *v);
	// etc...

public:
	ComputeShader *shader_to_run;

	// the grid/block size use for a dispatch
	// You can store this with the job - or pass it to your
	// RHIDeviceContext::Dispatch call - I just prefer to keep it with 
	// the job.
	UintVector3 m_grid;
	const TextureDX* m_uav;
	uint m_uavIndex;

	std::vector<TextureDX*> m_textures;
	std::vector<ConstantBuffer*> m_buffers;
	std::vector<ComputeBuffer*> m_bufferdata;
	std::vector<Sampler*> m_samplers;
};