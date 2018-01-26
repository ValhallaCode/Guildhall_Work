#pragma once
#include "Engine/RHI/RHI.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/RHI/ShaderProgram.hpp"
#include "Engine/RHI/Sampler.hpp"
#include "Engine/RHI/VertexBuffer.hpp"
#include "Engine/RHI/ConstantBuffer.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/RHI/BlendState.hpp"
#include "Engine/Render/KerningFont.hpp"
#include "Engine/RHI/DepthStencilState.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/RHI/Mesh.hpp"
#include "Engine/RHI/Material.hpp"
#include "Engine/RHI/TextureCube.hpp"
#include "Engine/RHI/RenderMesh.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/Render/Motion.hpp"
#include <map>

class IndexBuffer;
class RasterState;
class Skeleton;
class Animator3D;
class TextureDX;
class Texture1D;

struct MatrixBuffer
{
	MatrixBuffer()
		:eyePosition(0.0f, 0.0f, 0.0f, 0.0f)
	{};
	Matrix4 model;
	Matrix4 view;
	Matrix4 projection;
	Vector4 eyePosition;
};

struct BlendState_T
{
	BlendState_T()
	: enabled(false)
		, sourceFactor(BLEND_ONE)
		, destFactor(BLEND_ONE)
		, sourceAlpha(BLEND_ONE)
		, destAlpha(BLEND_ONE)
	{};
	bool enabled;
	eBlendFactor sourceFactor;
	eBlendFactor destFactor;
	eBlendFactor sourceAlpha;
	eBlendFactor destAlpha;
};


struct PointLight
{
	PointLight()
		: pointLightColor{1,1,1,0}
		, lightPosition(0, 0, 0, 255)
		, attenuation(255, 0, 0, 0)     // will effectively dampen the light - at intensity 1, this will make the light constant
		, specularAttenuation(1.0f, 0.0f, 0.0f, 0.0f)
	{}

	// POINT LIGHT
	float pointLightColor[4]; // <r, g, b, intensity>
	Vector4 lightPosition; // <x, y, z, padding>  // variables can not cross a 16-byte boundary, so we pad out 
	Vector4 attenuation;    // <constant, linear, quadratic, unused>
	Vector4 specularAttenuation;
};

struct LightBuffer_T 
{
	LightBuffer_T()
		:ambientLightColor{ 1,1,1,1 }
		, lightsArray{}
		, specularFactor(0.0f)
		, specularPower(0.0f)
		, diretionOfView(0.0f, 0.0f,0.0f,0.0f)
		, directionalColor{1,1,1,0}
	{};

	PointLight lightsArray[8];
	float ambientLightColor[4];  // <r, g, b, intensity>
	float specularFactor;
	float specularPower;
	Vector2 unused;
	Vector4 diretionOfView;
	float directionalColor[4];
};

class SimpleRenderer
{
public:
	SimpleRenderer();
	~SimpleRenderer();

	void Setup(unsigned int width, unsigned int height, const std::string& title);
	void Destroy();
	void SetRenderTarget(Texture2D* colorTarget);
	void SetRenderTarget(Texture2D* colorTarget, Texture2D* depthTarget);
	void SetWindowTitle(const std::string& title);
	void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	void ClearColor(const Rgba& color);
	void ClearDepth(float depth = 1.0f, uint8_t stencil = 0);
	void ClearTargetColor(Texture2D *target, const Rgba& color);
	void Present();
	void ProcessMessages();
	void IsClosed();
	void SetConstantBuffer(unsigned int bufferIndex, const ConstantBuffer* buffer);
	void SetShaderProgram(ShaderProgram* shader);
	//inline void SetShader(ShaderProgram* shader) { SetShader(0, shader); }
	void SetTexture(unsigned int textureIndex, TextureDX* texture);
	inline void SetTexture(TextureDX* texture) { SetTexture(0, texture); }
	void SetSampler(unsigned int samplerIndex, Sampler* sampler);
	inline void SetSampler(Sampler* sampler) { SetSampler(0, sampler); }
	void UpdateBuffersAndDraw(Vertex3_PCT* vertices, const std::vector<unsigned int>& indexes, unsigned int vertCount, unsigned int indexCount, ePrimitiveType primitive = PRIMITIVE_TRIANGLE);
	void ResizeBuffers(unsigned int vertCount, unsigned int indexCount);
	void DrawLine(const Vector3& startPoint, const Vector3& endPoint, const Rgba& color = Rgba(255, 255, 255, 255));
	void DrawIndexed(ePrimitiveType topology, VertexBuffer* vbo, IndexBuffer* ibo, const unsigned int indexCount);
	void SetProjectionMatrix(const Matrix4& proj);
	void SetModelMatrix(const Matrix4& transform);
	void MakeViewMatrixIdentity();
	void MakeModelMatrixIdentity();
	void SetViewMatrix(const Vector3& rotation, const Vector3& position);
	void SetOrthoProjection(const Vector2& bottomLeft, const Vector2& topRight);
	void SetPerspectiveProjection(float fieldOfViewInDegrees, float aspectRatio, float nearZ, float farZ);
	Matrix4 MatrixMakeOrthoProjection(float nx, float fx, float ny, float fy, float nz /*= 0.0f*/, float fz /*= 1.0f*/);
	Matrix4 MatrixMakePerspectiveProjection(float fieldOfViewInDegrees, float aspectRatio, float nearZ, float farZ);
	void EnableBlend(eBlendFactor sourceColor, eBlendFactor destinationColor);
	void DisableBlend();
	void DrawTextWithFont(KerningFont* font, float sx, float sy, const std::string& text, const Rgba& color, float scale = 1.0f, float aspect = 1.0f);
	void DrawTextCenteredOnPosition2D(KerningFont* font, const Vector2& position, const std::string& text, const Rgba& color, float scale = 1.0f, float aspectRatio = 1.0f);
	void DrawTextInBox(KerningFont* font, const AABB2D& box, const std::string& text, const Rgba& color, float scaleX = 1.0f, float scaleY = 1.0f);
	void EnableDepthTest(bool enable);
	void EnableDepthWrite(bool enable);
	bool IsLightArrayFull();
	void SetAmbientLight(float intensity, const Rgba& color = Rgba(255, 255, 255, 255));
	void SetPointLight(const Vector3& pos, const Rgba& color, float intensity = 1.0f, const Vector3& attenuation = Vector3(0.0f, 0.0f, 1.0f), const Vector3& specularAttenuation = Vector3(0.0f, 0.0f, 1.0f));
	void ResetLightingIndex();
	void SetSpecularConstraints(const float specPower, const float specFactor = 1.0f);
	inline void DisablePointLight();
	void SetEyePosition(const Vector3& eyePosition);
	inline void SetDiffuse(TextureDX *texture) {SetTexture(0, (texture == nullptr) ? m_whiteTexture : texture);};
	inline void SetNormal(TextureDX *texture) {SetTexture(1, (texture == nullptr) ? m_flatNormalTexture : texture);};
	inline void SetSpecular(TextureDX *texture) {SetTexture(2, (texture == nullptr) ? m_whiteTexture : texture);};
	inline void SetAmbientOcclusion(TextureDX *texture) { SetTexture(3, (texture == nullptr) ? m_whiteTexture : texture); };
	inline void SetEmmisive(TextureDX *texture) { SetTexture(4, (texture == nullptr) ? m_blackTexture : texture); };
	inline void SetBump(TextureDX *texture) { SetTexture(5, (texture == nullptr) ? m_blackTexture : texture); };
	inline void SetHeight(TextureDX *texture) { SetTexture(6, (texture == nullptr) ? m_greyTexture : texture); };
	inline void SetStructuredBuffer(StructuredBuffer* structure) { SetStructuredBuffer(7, structure); }
	void SetStructuredBuffer(unsigned int structureIndex, StructuredBuffer* structure);
	void DrawPoint(const Vector3& position, const Rgba& color = Rgba(255,255,255,255));
	void DrawPoint(const Vector2& position, const Rgba& color = Rgba(255,255,255,255), float radius = 10.0f);
	void SetDirectionalLight(const Vector4& directionOfView, const Rgba& color);
	void DrawMesh(Mesh& mesh);
	void DrawOneSidedQuad(const Vector3& position, const Vector3& extension, const Rgba& color /*= Rgba(255,255,255,255)*/);
	void SetMaterial(Material& material);
	void SetRasterStateOnShader(Shader& shader);
	void SetDepthStencilOnShader(Shader& shader);
	void EnableBlendOnShader(Shader& shader);
	void DisableBlendOnShader(Shader& shader);
	void DrawSkyBox(Vector3& position, float zDepth, std::string& filePath);
	void DrawRenderMesh(RenderMesh& renderObject);
	void DrawTexturedSprite(const AABB2D& worldBounds, Texture2D& texture, const AABB2D& texBounds, const Rgba& tint, eBlendFactor sourceColor, eBlendFactor destinationColor);
	void Draw2DDebugQuad(Vector2& position, Vector2& extension, Rgba& innerColor, Rgba& outerColor, Texture2D& texture, float thickness = 0.0f);
	void Draw2DDebugLine(Vector2& start, Vector2& end, Rgba& startColor, Rgba& endColor, Texture2D& texture, float thickness = 0.0f);
	void DrawOrientedDebugCircle(const Vector2& position, float radius, Rgba color = Rgba(255, 255, 255, 255));
	void Draw2DDebugCircle(Vector2& position, float radius, Rgba& innerColor, Rgba& outerColor, Texture2D& texture, float thickness = 0.0f);
	void DrawCharInBox(KerningFont* font, const AABB2D& bounds, const std::string& text, const Rgba& color, float scaleX = 1.0f, float scaleY = 1.0f);
	void DrawBitmapFont(const BitmapFont* font, const Vector2& startBottomLeft, const std::string& asciiText, float cellHeight = 1.0f, const Rgba& tint = Rgba(255,255,255,255), float cellAspectScale = 1.0f);
	void DrawSkeleton(Skeleton* skeleton);
	void DrawSkeletonWithMotion(const Motion* motion, Skeleton* skeleton, float time, ePlayMode playMode);
	void DrawSkeletonWithPose(Pose* pose, Skeleton* skeleton);
	void DrawAnimatedMesh(Mesh* mesh, const Motion* motion, Skeleton* skeleton, ShaderProgram* shader, float time, ePlayMode playMode);
	void DrawAnimatedMesh(Animator3D* animator);
	void DrawArrow2D(Vector2& start, Vector2& end, Rgba& startColor, Rgba& endColor, float thickness = 0.1f, Texture2D* texture = nullptr);
	void Draw2DQuad(const Vector2& position, const Vector2& extension, Texture2D* texture /*= nullptr*/, const Rgba& color /*= Rgba(255,255,255,255)*/);
public:
	LightBuffer_T m_lightData;
	ConstantBuffer* m_lightBuffer;
	unsigned int m_lightIndex;
	DepthStencilDesc_T m_depthStencilDesc;
	DepthStencilState* m_depthStencilState;
	RHIDeviceContext* m_context;
	RHIDevice* m_device;
	RHIOutput* m_output;
	Texture2D* m_currentTarget;
	Texture2D* m_defaultDepthStencil;
	Texture2D* m_currentDepthStencil;
	VertexBuffer* m_vertBuffer;
	IndexBuffer* m_indexBuffer;
	MatrixBuffer m_matrixData;
	ConstantBuffer* m_matrixBuffer;
	RasterState* m_rasterState;
	BlendState_T m_blendStruct;
	BlendState* m_blendState;
	KerningFont* m_font;
	Texture2D* m_whiteTexture;
	Texture2D* m_flatNormalTexture;
	Texture2D* m_greyTexture;
	Texture2D* m_blackTexture;
	TextureCube* m_textureCube;
	Mesh* m_textQuad;
	ePrimitiveType m_currentPrimitive;
	unsigned int m_mostRecentIndexCount;
};