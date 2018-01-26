#pragma once
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "Glu32" ) // Link in the Glu32.lib static library
#include "Engine/Render/Rgba.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/AABB3D.hpp"
#include "Engine/Render/Texture.hpp"
#include "Engine/Render/BitmapFont.hpp"
#include "Engine/Render/Vertex.hpp"
#include <string>
#include <vector>
#include <map>



extern unsigned int PRIMITIVE_QUADS;
extern unsigned int PRIMITIVE_TRIANGLES;
extern unsigned int PRIMITIVE_LINES;
extern unsigned int PRIMITIVE_POINTS;
extern unsigned int PRIMITIVE_LINE_LOOP;


class Renderer
{
public:
	Renderer();
	~Renderer();
	void DrawAABB3D(const AABB3D& bounds, const Rgba& boxColor, unsigned int drawMode);
	void ClearScreen(const Rgba& clearColor);
	void EnableDepthTestAndWrite();
	void DisableDepthTestAndWrite();
	void SetOrtho2D(const Vector2& bottomLeft, const Vector2& topRight);
	void DrawLine2D(const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, float lineThickness);
	void DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& startColor, const Rgba& endColor, float lineThickness);
	void DrawAABB2D(const AABB2D& bounds, const Rgba& boxColor, unsigned int drawMode);
	void StartManipulatingTheDrawnObject();
	void TranslateDrawing2D(const Vector2& pointToTranslate);
	void TranslateDrawing3D(const Vector3& pointToTranslate);
	void RotateDrawing(float angle, float xValue, float yValue, float zValue);
	void ScaleDrawing(float xScaleFactor, float yScaleFactor, float zScaleFactor);
	void EndManipulationOfDrawing();
	void SetLineWidth(float lineWidth);
	void SetLineColor(const Rgba& lineColor);
	void SetPointToDraw2D(Vector2& point);
	void SetPointToDraw3D(Vector3& point);
	Texture* CreateOrGetTexture(const std::string& imageFilePath);
	void DrawTexturedQuad3D(const Vertex3_PCT& vertA, const Vertex3_PCT& vertB, const Vertex3_PCT& vertC, const Vertex3_PCT& vertD, Texture& texture);
	void DrawTexturedSprite(const AABB2D& worldBounds, Texture& texture, const AABB2D& texBounds, const Rgba& tint, bool useBlending);
	void DrawCircle2D(const Vector2& center, float radius, const Rgba& lineColor);
	void DrawCircle3D(const Vector3& center, float radius, const Rgba& lineColor);
	void DrawVertexArray2D_PC(const Vertex2_PC* vertexArray, int numVertexes, unsigned int drawMode);
	void DrawVertexArray3D_PCT(const Vertex3_PCT* vertexArray, int numVertexes, unsigned int drawMode, Texture* texture = nullptr);
	void DrawVBO3D_PCT(unsigned int vboID, int numVertexes, unsigned int drawMode);
	unsigned int CreateVBOID();
	void DestroyVBO(unsigned int vboID);
	void UpdateVBO(unsigned int vboID, Vertex3_PCT* vertexArray, int numVertexes);
	void DrawVertexArray3D_PC(const Vertex3_PC* vertexArray, int numVertexes, unsigned int drawMode);
	void SetPointSize(float pointSize);
	IntVector2 GetDimensionsOfImage(const Texture& texture);
	void SetPerspective(float fovyDegrees, float aspectRatio, float nearClipDist, float farClipDist);
	void DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellHeight, const Rgba& tint = Rgba(255, 255, 255, 255), float cellAspectScale = 1.f, const BitmapFont* font = nullptr);
	void EnableBackFaceCulling();
	void BindTexture(Texture* texture);
	BitmapFont* CreateOrGetFont(const std::string& bitmapFontName);
	void DrawFilledCircle2D(const Vertex3_PCT& center, const Vertex3_PCT& edge, float numSides = 64.0f);


private:
	std::vector< Texture* >	m_alreadyLoadedTextures;
	std::map< std::string, BitmapFont* > m_loadedFonts;
	Texture* defaultTexture;
	Texture* CreateTextureFromFile(const std::string& imageFilePath);
	Texture* GetTexture(const std::string& imageFilePath);
	unsigned int CreateOpenGLTexture(unsigned char* imageTexelBytes, int width, int height, int bytesPerTexel);
};

