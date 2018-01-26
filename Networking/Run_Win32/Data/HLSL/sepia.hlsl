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
vertex_to_fragment_t VertexFunction(vertex_in_t vertex)
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;
	float4 worldPosition = mul(float4(vertex.position, 1.0f), MODEL);
	float4 viewPosition = mul(worldPosition, VIEW);
	float4 clipPosition = mul(viewPosition, PROJECTION);
	out_data.position = clipPosition;
	out_data.uv = vertex.uv;
	return out_data;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 FragmentFunction(vertex_to_fragment_t input) : SV_Target0 // semeantic of what I'm returning
{
   float4 diffuse = tTexture.Sample( sSampler, input.uv );
   
   /* EXPLICIT WAY */
   float3 sepia_r = float3( 0.393f, 0.769f, 0.189f );
   float3 sepia_g = float3( 0.349f, 0.686f, 0.168f );
   float3 sepia_b = float3( 0.272f, 0.534f, 0.131f );
   float3 sepia = float3( 
      dot( diffuse.xyz, sepia_r ),
      dot( diffuse.xyz, sepia_g ),
      dot( diffuse.xyz, sepia_b )
   );

   float4 final_color = float4( sepia, 1.0f );
   return final_color;
}