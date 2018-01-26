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

float RangeMap(float startMin, float startMax, float endMin, float endMax, float startValue)
{
	if ((startMax - startMin) != 0.f)
	{
		float normalizedRange = (startValue - startMin) / (startMax - startMin);
		return (normalizedRange * (endMax - endMin)) + endMin;
	}
}

// NOP VERTEX FUNCTION
vertex_to_fragment_t VertexFunction( vertex_in_t vertex ) 
{
	vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

	float width = 1280;
	float height = 720;

	float transformX = RangeMap(0.0f, width, -1.0f, 1.0f, vertex.position.x);
	float transformY = RangeMap(0.0f, height, -1.0f, 1.0f, vertex.position.y);

	out_data.position = float4(float3(transformX, transformY, vertex.position.z), 1.0f);
	out_data.uv = vertex.uv;

	return out_data;
}

// COLOR (PIXEL/FRAGMENT) FUNCTION
float4 FragmentFunction( vertex_to_fragment_t data ) : SV_Target0
{
   return float4( data.uv, 0.0f, 1.0f );
}




