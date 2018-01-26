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


Texture2D <float4> tTexture : register(t0);
SamplerState sSampler : register(s0);


// NOP VERTEX FUNCTION
vertex_to_fragment_t VertexFunction( vertex_in_t vertex ) 
{
   vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;
   out_data.position = float4( vertex.position, 1.0f );
   out_data.uv = vertex.uv;
   return out_data;
}

// COLOR (PIXEL/FRAGMENT) FUNCTION
float4 FragmentFunction( vertex_to_fragment_t data ) : SV_Target0
{
   float4 color = tTexture.Sample( sSampler, data.uv );

   color.xyz = (color.x + color.y + color.z) / 3.0f;

   if (color.x < 0.2f || color.x > 0.9f)
	   color.x = 0.0f;
   else
	   color.x = 1.0f;

   if (color.y < 0.2f || color.y > 0.9f)
	   color.y = 0.0f;
   else
	   color.y = 1.0f;

   if (color.z < 0.2f || color.z > 0.9f)
	   color.z = 0.0f;
   else
	   color.z = 1.0f;
   
   return float4( color.x, color.y, color.z, 1 );
}




