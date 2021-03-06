struct vertex_in_t
{
   float3 position : POSITION;
   float2 uv : UV;
   float4 color : COLOR;
};

struct vertex_to_fragment_t
{
   float4 position : SV_Position; 
   float2 uv : UV;
   float4 color : COLOR;
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
   float4 worldPosition = mul(float4(vertex.position, 1.0f), MODEL);
   float4 viewPosition = mul(worldPosition, VIEW);
   float4 clipPosition = mul(viewPosition, PROJECTION);
   out_data.position = clipPosition;
   out_data.uv = vertex.uv;
   out_data.color = vertex.color;
   return out_data;
}

// COLOR (PIXEL/FRAGMENT) FUNCTION
float4 FragmentFunction( vertex_to_fragment_t data ) : SV_Target0
{
   float4 color = tTexture.Sample( sSampler, data.uv );
   return color * data.color;
}




