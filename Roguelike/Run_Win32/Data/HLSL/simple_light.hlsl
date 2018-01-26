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
   float4 AMBIENT;         // <r, g, b, intensity>
   float SPEC_FACTOR;
   float SPEC_POWER;
   float2 LIGHT_PADDING;
   float4 DIRECTIONAL_LIGHT_DIRECTION;
   float4 DIRECTIONAL_COLOR;
};


Texture2D <float4> tDiffuse : register(t0);
Texture2D <float4> tNormal : register(t1);
Texture2D<float4> tSpecular : register(t2);
Texture2D<float4> tAmbientOcclusion : register(t3);
Texture2D<float4> tEmissive : register(t4);
Texture2D<float4> tBump : register(t5);

SamplerState sSampler : register(s0);

//-------------------------------------------------------------------------
// NOP VERTEX FUNCTION
vertex_to_fragment_t VertexFunction( vertex_in_t vertex ) 
{
   vertex_to_fragment_t out_data = (vertex_to_fragment_t)0;

   float4 model_position = float4( vertex.position, 1.0f );
   float4 world_position = mul( model_position, MODEL );
   float4 view_position = mul( world_position, VIEW );
   float4 clip_position = mul( view_position, PROJECTION );

   out_data.position = clip_position;
   out_data.normal = mul( float4(vertex.normal, 0.0f), MODEL ).xyz; // W get sa 0, so as not to take on the translation of the model matrix
   out_data.tint = vertex.tint;
   out_data.uv = vertex.uv;
   out_data.world_position = world_position.xyz;
   out_data.tangent = mul(float4(vertex.tangent, 0.0f), MODEL).xyz;
   out_data.bitangent = mul(float4(vertex.bitangent, 0.0f), MODEL).xyz;
   return out_data;
}

float4 NormalAsColor( float3 n )
{
   return float4( (n + 1.0f) * .5f, 1.0f );
}

float SmoothStep5(float inputZeroToOne)					// 6t^5 - 15t^4 + 10t^3
{
	float firstComponent = 6.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne);
	float secondComponent = 15.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne * inputZeroToOne);
	float thirdComponent = 10.f * (inputZeroToOne * inputZeroToOne * inputZeroToOne);
	return firstComponent - secondComponent + thirdComponent;
}

//-------------------------------------------------------------------------
// COLOR (PIXEL/FRAGMENT) FUNCTION
float4 FragmentFunction( vertex_to_fragment_t data ) : SV_Target0
{
	float3 specularMap = tSpecular.Sample(sSampler, data.uv).xyz;
	float3 occlusionMap = tAmbientOcclusion.Sample(sSampler, data.uv).xyz;

	// ?  Sample Color at Texture Coordinate data.uv
	float4 color = tDiffuse.Sample(sSampler, data.uv);
	//float4 diffuse = color * data.tint;
	float4 diffuse = color * float4(data.tint.xyz, 1.f);

	float3 surface_normal = data.normal;
   
   // renormalize - the interpolation step may have skewed it
   // float3 normal = data.normal;
	float3 normal = surface_normal;

	normal = normalize(normal);

   // calculate the eye vector
	float3 vector_to_eye = EYE_POSITION.xyz - data.world_position;
	float3 eye_vector = -normalize(vector_to_eye); // get direction from eye to fragment

	float4 ambient_factor = float4(AMBIENT.xyz * AMBIENT.w, 1.0f);

	// PER LIGHT START
	// point light factor - light factors are calculated PER LIGHT and added 
	// to the ambient factor to get your final diffuse factor

	float4 accumLightFactor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 accumSpecFactor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	for (uint index = 0; index < 8; index++)
	{
		float3 vector_to_light = LIGHT_ARRAY[index].LIGHT_POSITION.xyz - data.world_position;
		float distance_to_light = length(vector_to_light);
		float3 dir_to_light = vector_to_light / distance_to_light;

		
		float attenuation = LIGHT_ARRAY[index].LIGHT_COLOR.w / (LIGHT_ARRAY[index].ATTENUATION.x
		+ distance_to_light * LIGHT_ARRAY[index].ATTENUATION.y
		+ distance_to_light * distance_to_light * LIGHT_ARRAY[index].ATTENUATION.z);

	// don't allow attenuation to go above one (so don't allow it to blow out the image)
		attenuation = saturate(attenuation);
		float dot3 = saturate(dot(dir_to_light, normal));
		float4 light_color = float4(LIGHT_ARRAY[index].LIGHT_COLOR.xyz, 1.0f);
		accumLightFactor += light_color * dot3 * attenuation;

	// Calculate Spec Component
		float spec_attenuation = LIGHT_ARRAY[index].LIGHT_COLOR.w / (LIGHT_ARRAY[index].SPEC_ATTENUATION.x
		+ distance_to_light * LIGHT_ARRAY[index].SPEC_ATTENUATION.y
		+ distance_to_light * distance_to_light * LIGHT_ARRAY[index].SPEC_ATTENUATION.z);

	// figure how much the reflected vector coincides with the eye vector
		float3 ref_light_dir = reflect(dir_to_light, normal);
		float spec_dot3 = saturate(dot(ref_light_dir, eye_vector));

	// take it to the spec power, and scale it by the spec factor and our attenuation
		float spec_factor = spec_attenuation * SPEC_FACTOR * pow(spec_dot3, SPEC_POWER);
		float4 spec_color = spec_factor * light_color;
		accumSpecFactor += (spec_color * float4(specularMap, 1.0f));
	}


	float3 dirToLight = -DIRECTIONAL_LIGHT_DIRECTION.xyz;
	float dot3 = saturate(dot(dirToLight, normal));
	float4 dirLightColor = float4(DIRECTIONAL_COLOR.xyz, 1.0f);
	accumLightFactor += ((dirLightColor * dot3)); //* DIRECTIONAL_COLOR.w);

	float3 refLightDir = reflect(dirToLight, normal);
	float specDot3 = saturate(dot(refLightDir, eye_vector));
	float specFactor = SPEC_FACTOR * pow(specDot3, SPEC_POWER);
	float4 specColor = specFactor * DIRECTIONAL_COLOR;
	accumSpecFactor += ((specColor * float4(specularMap, 1.0f))); //* DIRECTIONAL_COLOR.w);

	// END PER LIGHT PART

	// calculate final diffuse factor by adding all light factors to the ambiant factor
	float4 diffuse_factor = saturate(ambient_factor + accumLightFactor);

	// final color is our diffuse color multiplied by diffuse factor.  Spec color is added on top. 
	float4 final_color = diffuse_factor * diffuse + accumSpecFactor;

	float emisRate = (0.35f * (sin(GAME_TIME * 2.0f) + 2.0f));
	emisRate = SmoothStep5(emisRate);

	// calculate final color
	//return final_color + (tEmissive.Sample(sSampler, data.uv) * emisRate);
	return float4((final_color + (tEmissive.Sample(sSampler, data.uv) * emisRate)).xyz, 1.f);
	//return float4(data.normal, 1.0f);
}

