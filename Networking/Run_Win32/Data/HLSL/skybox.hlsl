// VERTEX INPUT
struct vertex_in_t
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float4 tint : COLOR;
	float2 uv : UV;
};

//-------------------------------------------------------------------------
// VERTEX TO FRAGMENT DATA
struct vertex_to_fragment_t
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float4 tint : TINT;
	float2 uv : UV;

	float3 world_position : WORLD;
};

//-------------------------------------------------------------------------
// CONSTANT BUFFERS
cbuffer matrix_cb : register(b0)
{
	float4x4 MODEL;
	float4x4 VIEW;
	float4x4 PROJECTION;
	float4 EYE_POSITION;
};

// MAX SIZE: 4096 Elements (float4), 16 KB
// ALIGNMENT:  must be aligned to 16B,
cbuffer time_cb : register(b1)
{
	float GAME_TIME;
	float SYSTEM_TIME;
	float GAME_FRAME_TIME;
	float SYSTEM_FRAME_TIME;
};

struct PointLight
{
	// POINT LIGHT, DOT3
	float4 LIGHT_COLOR; // <r, g, b, intensity>
	float4 LIGHT_POSITION;
	float4 ATTENUATION;
	float4 SPEC_ATTENUATION;
};


cbuffer light_cb : register(b2)
{
	PointLight LIGHT_ARRAY[8];
	float4 AMBIENT; // <r, g, b, intensity>
	float SPEC_FACTOR;
	float SPEC_POWER;
	float2 LIGHT_PADDING;
	float4 DIRECTIONAL_LIGHT_DIRECTION;
	float4 DIRECTIONAL_COLOR;
};


TextureCube<float4> tDiffuse : register(t0);
SamplerState sSampler : register(s0);

vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t) 0;

	float4 model_position = float4(vertex.position, 1.0f);
	float4 world_position = mul(model_position, MODEL);
	float4 view_position = mul(world_position, VIEW);
	float4 clip_position = mul(view_position, PROJECTION);

	out_data.position = clip_position;
	out_data.normal = mul(float4(vertex.normal, 0.0f), MODEL).xyz; // W get sa 0, so as not to take on the translation of the model matrix
	out_data.tint = vertex.tint;
	out_data.uv = vertex.uv;
	out_data.world_position = world_position.xyz;
	out_data.tangent = mul(float4(vertex.tangent, 0.0f), MODEL).xyz;
	out_data.bitangent = mul(float4(vertex.bitangent, 0.0f), MODEL).xyz;
	return out_data;
}

// COLOR (PIXEL/FRAGMENT) FUNCTION
float4 FragmentFunction(vertex_to_fragment_t data) : SV_Target0
{
	float4 color = tDiffuse.Sample(sSampler, data.world_position - EYE_POSITION.xyz);
	return color;
}