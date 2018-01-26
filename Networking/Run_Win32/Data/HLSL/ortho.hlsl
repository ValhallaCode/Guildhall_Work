struct vertex_in_t
{
   float3 position : POSITION;
   float2 uv : UV;
};

struct vertex_to_fragment_t
{
   float4 position : SV_Position; 
   float2 uv : UV;
};

cbuffer matrix_cb : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
};

//Max Size: 4096 Elements (float4), 16KB
//Alignment: must be aligned to 16B
cbuffer time_cb : register(b1)
{
	float GAME_TIME;
	float SYSTEM_TIME;
	float GAME_FRAME_TIME;
	float SYSTEM_FRAME_TIME;
};

Texture2D <float4> tTexture : register(t0);
SamplerState sSampler : register(s0);


// NOP VERTEX FUNCTION
vertex_to_fragment_t VertexFunction( vertex_in_t vertex ) 
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	float nx = 0.0f;
	float fx = 1280.0f;
	float ny = 0.0f;
	float fy = 720.0f;
	float nz = -10.0f;
	float fz = 10.0f;

	float sx = 1.0f / (fx - nx);
	float sy = 1.0f / (fy - ny);
	float sz = 1.0f / (fz - nz);

	float4x4 projection = {
		float4(2.0f * sx,		0.0f,			 0.0f,		0.0f),
		float4(0.0f,			2.0f * sy,		 0.0f,		0.0f),
		float4(0.0f,			0.0f,			 1.0f * sz, 0.0f),
		float4(-(fx + nx) * sx, -(fy + ny) * sy, -nz * sz,	1.0f)
	};

	float4 transform_position = mul(float4(vertex.position, 1.0f), projection);

	out_data.position = transform_position;
	out_data.uv = vertex.uv;

	return out_data;
}

// COLOR (PIXEL/FRAGMENT) FUNCTION
float4 FragmentFunction( vertex_to_fragment_t data ) : SV_Target0
{
   float4 color = tTexture.Sample( sSampler, data.uv );

   float t = frac(SYSTEM_TIME);

   float3 desaturate = float3(0.3, 0.59, 0.11);
   float g = dot( desaturate, color.xyz );
   
   float4 gray = { g, g, g, 1 };
   float4 final = lerp(color, gray, t);

   return final;
}




