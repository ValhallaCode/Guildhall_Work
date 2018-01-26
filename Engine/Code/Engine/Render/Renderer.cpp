#include "Engine/Render/Renderer.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Render/Texture.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "ThirdParty/OpenGL/glext.h"
#include "ThirdParty/OpenGL/wglExt.h"


unsigned int PRIMITIVE_QUADS = GL_QUADS;
unsigned int PRIMITIVE_TRIANGLES = GL_TRIANGLES;
unsigned int PRIMITIVE_LINES = GL_LINES;
unsigned int PRIMITIVE_LINE_LOOP = GL_LINE_LOOP;
unsigned int PRIMITIVE_POINTS = GL_POINTS;
unsigned int TRIANGLE_FAN = GL_TRIANGLE_FAN;

// Globals "function pointer" variables (these go in OpenGLExtensions.cpp or similar)
PFNGLGENBUFFERSPROC		glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC		glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC		glBufferData = nullptr;
PFNGLDELETEBUFFERSPROC	glDeleteBuffers = nullptr;


Renderer::Renderer()
	:defaultTexture(nullptr)
{
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	
	GUARANTEE_OR_DIE(glGenBuffers != nullptr, "ERROR: Video card does not support glGenBuffers");
	GUARANTEE_OR_DIE(glBindBuffer != nullptr, "ERROR: Video card does not support glBindBuffer");
	GUARANTEE_OR_DIE(glBufferData != nullptr, "ERROR: Video card does not support glBufferData");
	GUARANTEE_OR_DIE(glDeleteBuffers != nullptr, "ERROR: Video card does not support glDeleteBuffers");

	defaultTexture = CreateOrGetTexture("Data/Images/DebugBlock.png");
}

Renderer::~Renderer()
{}

void Renderer::ClearScreen(const Rgba& clearColor)
{
	float r, g, b, a;
	glDisable( GL_TEXTURE_2D );
	clearColor.GetAsFloats( r, g, b, a );
	glClearColor( r, g, b, a );
	glClearDepth( 1.f ); 
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EnableDepthTestAndWrite()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

void Renderer::DisableDepthTestAndWrite()
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void Renderer::SetOrtho2D(const Vector2& bottomLeft, const Vector2& topRight)
{
	glDisable(GL_TEXTURE_2D);
	glLoadIdentity();
	glOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f);
}

void Renderer::DrawLine2D(const Vector2& start, const Vector2& end, const Rgba& startColor, const Rgba& endColor, float lineThickness)
{
	Vector3 start3(start.x, start.y, 0.f);
	Vector3 end3(end.x, end.y, 0.f);
	DrawLine3D(start3, end3, startColor, endColor, lineThickness);
}

void Renderer::DrawLine3D(const Vector3& start, const Vector3& end, const Rgba& startColor, const Rgba& endColor, float lineThickness)
{
	Vertex3_PCT vertexes[2];
	vertexes[0].m_position = start;
	vertexes[0].m_color = startColor;
	vertexes[1].m_position = end;
	vertexes[1].m_color = endColor;
	
	SetLineWidth(lineThickness);
	DrawVertexArray3D_PCT(&vertexes[0], 2, GL_LINES, nullptr);
}

void Renderer::DrawAABB2D(const AABB2D& bounds, const Rgba& boxColor, unsigned int drawMode)
{
	AABB3D bounds3(Vector3(bounds.mins.x, bounds.mins.y, 0.f), Vector3(bounds.maxs.x, bounds.maxs.y, 0.f));
	DrawAABB3D(bounds3, boxColor, drawMode);
}

void Renderer::DrawAABB3D(const AABB3D& bounds, const Rgba& boxColor, unsigned int drawMode)
{
	Vertex3_PCT vertexes[24];
	vertexes[0].m_position = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[0].m_color = boxColor;
	vertexes[0].m_texCoords = Vector2(0.f, 1.f);
	vertexes[1].m_position = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
	vertexes[1].m_color = boxColor;
	vertexes[1].m_texCoords = Vector2(1.f, 1.f);
	vertexes[2].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[2].m_color = boxColor;
	vertexes[2].m_texCoords = Vector2(1.f, 0.f);
	vertexes[3].m_position = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[3].m_color = boxColor;
	vertexes[3].m_texCoords = Vector2(0.f, 0.f);

	vertexes[4].m_position = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[4].m_color = boxColor;
	vertexes[4].m_texCoords = Vector2(0.f, 1.f);
	vertexes[5].m_position = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[5].m_color = boxColor;
	vertexes[5].m_texCoords = Vector2(1.f, 1.f);
	vertexes[6].m_position = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[6].m_color = boxColor;
	vertexes[6].m_texCoords = Vector2(1.f, 0.f);
	vertexes[7].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[7].m_color = boxColor;
	vertexes[7].m_texCoords = Vector2(0.f, 0.f);

	vertexes[8].m_position = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[8].m_color = boxColor;
	vertexes[8].m_texCoords = Vector2(0.f, 1.f);
	vertexes[9].m_position = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[9].m_color = boxColor;
	vertexes[9].m_texCoords = Vector2(1.f, 1.f);
	vertexes[10].m_position = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[10].m_color = boxColor;
	vertexes[10].m_texCoords = Vector2(1.f, 0.f);
	vertexes[11].m_position = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[11].m_color = boxColor;
	vertexes[11].m_texCoords = Vector2(0.f, 0.f);

	vertexes[12].m_position = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
	vertexes[12].m_color = boxColor;
	vertexes[12].m_texCoords = Vector2(0.f, 1.f);
	vertexes[13].m_position = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[13].m_color = boxColor;
	vertexes[13].m_texCoords = Vector2(1.f, 1.f);
	vertexes[14].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[14].m_color = boxColor;
	vertexes[14].m_texCoords = Vector2(1.f, 0.f);
	vertexes[15].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[15].m_color = boxColor;
	vertexes[15].m_texCoords = Vector2(0.f, 0.f);

	vertexes[16].m_position = Vector3(bounds.mins.x, bounds.maxs.y, bounds.mins.z);
	vertexes[16].m_color = boxColor;
	vertexes[16].m_texCoords = Vector2(0.f, 1.f);
	vertexes[17].m_position = Vector3(bounds.mins.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[17].m_color = boxColor;
	vertexes[17].m_texCoords = Vector2(1.f, 1.f);
	vertexes[18].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.maxs.z);
	vertexes[18].m_color = boxColor;
	vertexes[18].m_texCoords = Vector2(1.f, 0.f);
	vertexes[19].m_position = Vector3(bounds.maxs.x, bounds.maxs.y, bounds.mins.z);
	vertexes[19].m_color = boxColor;
	vertexes[19].m_texCoords = Vector2(0.f, 0.f);

	vertexes[20].m_position = Vector3(bounds.maxs.x, bounds.mins.y, bounds.mins.z);
	vertexes[20].m_color = boxColor;
	vertexes[20].m_texCoords = Vector2(0.f, 1.f);
	vertexes[21].m_position = Vector3(bounds.maxs.x, bounds.mins.y, bounds.maxs.z);
	vertexes[21].m_color = boxColor;
	vertexes[21].m_texCoords = Vector2(1.f, 1.f);
	vertexes[22].m_position = Vector3(bounds.mins.x, bounds.mins.y, bounds.maxs.z);
	vertexes[22].m_color = boxColor;
	vertexes[22].m_texCoords = Vector2(1.f, 0.f);
	vertexes[23].m_position = Vector3(bounds.mins.x, bounds.mins.y, bounds.mins.z);
	vertexes[23].m_color = boxColor;
	vertexes[23].m_texCoords = Vector2(0.f, 0.f);

	DrawVertexArray3D_PCT(&vertexes[0], 24, drawMode, nullptr);
}

void Renderer::StartManipulatingTheDrawnObject()
{
	glPushMatrix();
}

void Renderer::TranslateDrawing2D(const Vector2& pointToTranslate)
{
	glTranslatef(pointToTranslate.x, pointToTranslate.y, 0.f);
}

void Renderer::TranslateDrawing3D(const Vector3& pointToTranslate)
{
	glTranslatef(pointToTranslate.x, pointToTranslate.y, pointToTranslate.z);
}

void Renderer::RotateDrawing(float angle, float xValue, float yValue, float zValue)
{
	glRotatef(angle, xValue, yValue, zValue);
}

void Renderer::ScaleDrawing(float xScaleFactor, float yScaleFactor, float zScaleFactor)
{
	glScalef(xScaleFactor, yScaleFactor, zScaleFactor);
}

void Renderer::EndManipulationOfDrawing()
{
	glPopMatrix();
}

void Renderer::SetLineWidth(float lineWidth)
{
	glLineWidth(lineWidth);
}

void Renderer::SetLineColor(const Rgba& lineColor)
{
	glColor4ub(lineColor.r, lineColor.g, lineColor.b, lineColor.a);
}

void Renderer::SetPointToDraw2D(Vector2& point)
{
	Vector3 point3(point.x, point.y, 0.f);
	SetPointToDraw3D(point3);
}

void Renderer::SetPointToDraw3D(Vector3& point)
{
	glVertex3f(point.x, point.y, point.z);
}

//-----------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTexture(const std::string& imageFilePath)
{
	// Try to find that texture from those already loaded
	Texture* texture = GetTexture(imageFilePath);
	if (texture)
		return texture;

	texture = CreateTextureFromFile(imageFilePath);
	return texture;
}

void Renderer::DrawTexturedQuad3D(const Vertex3_PCT& vertA, const Vertex3_PCT& vertB, const Vertex3_PCT& vertC, const Vertex3_PCT& vertD, Texture& texture)
{
	Vertex3_PCT vertexes[4];
	vertexes[0] = vertA;
	vertexes[1] = vertB;
	vertexes[2] = vertC;
	vertexes[3] = vertD;

	DrawVertexArray3D_PCT(&vertexes[0], 4, GL_QUADS, &texture);
}

void Renderer::DrawTexturedSprite(const AABB2D& worldBounds, Texture& texture, const AABB2D& texBounds, const Rgba& tint, bool useBlending)
{
	Vertex3_PCT vertexes[4];
	vertexes[0].m_position = Vector3(worldBounds.mins.x, worldBounds.mins.y, 0.f);
	vertexes[0].m_color = tint;
	vertexes[0].m_texCoords = Vector2(texBounds.mins.x, texBounds.maxs.y);
	vertexes[1].m_position = Vector3(worldBounds.maxs.x, worldBounds.mins.y, 0.f);
	vertexes[1].m_color = tint;
	vertexes[1].m_texCoords =  Vector2(texBounds.maxs.x, texBounds.maxs.y);
	vertexes[2].m_position = Vector3(worldBounds.maxs.x, worldBounds.maxs.y, 0.f);
	vertexes[2].m_color = tint;
	vertexes[2].m_texCoords = Vector2(texBounds.maxs.x, texBounds.mins.y);
	vertexes[3].m_position = Vector3(worldBounds.mins.x, worldBounds.maxs.y, 0.f);
	vertexes[3].m_color = tint;
	vertexes[3].m_texCoords = Vector2(texBounds.mins.x, texBounds.mins.y);

	
	if(useBlending)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	DrawVertexArray3D_PCT(&vertexes[0], 4, GL_QUADS, &texture);

}



//-----------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile(const std::string& imageFilePath)
{
	// Load image data
	int width = 0;
	int height = 0;
	int bytesPerTexel = 0;
	unsigned char* imageTexelBytes = stbi_load(imageFilePath.c_str(), &width, &height, &bytesPerTexel, 0);
	GUARANTEE_OR_DIE(imageTexelBytes != nullptr, Stringf("Failed to load image file \"%s\" - file not found!", imageFilePath.c_str()));
	GUARANTEE_OR_DIE(bytesPerTexel == 3 || bytesPerTexel == 4, Stringf("Failed to load image file \"%s\" - image had unsupported %i bytes per texel (must be 3 or 4)", imageFilePath.c_str(), bytesPerTexel));

	// Create texture on video card, and send image (texel) data
	unsigned int openGLTextureID = CreateOpenGLTexture(imageTexelBytes, width, height, bytesPerTexel);
	stbi_image_free(imageTexelBytes);

	// Create (new) a Texture object
	Texture* texture = new Texture();
	texture->m_textureID = openGLTextureID;
	texture->m_imageFilePath = imageFilePath;
	texture->m_texelDimensions.SetXY(width, height);

	m_alreadyLoadedTextures.push_back(texture);
	return texture;
}


//-----------------------------------------------------------------------------------------------
Texture* Renderer::GetTexture(const std::string& imageFilePath)
{
	for (int textureIndex = 0; textureIndex < (int)m_alreadyLoadedTextures.size(); ++textureIndex)
	{
		Texture* texture = m_alreadyLoadedTextures[textureIndex];
		if (imageFilePath == texture->m_imageFilePath)
			return texture;
	}

	return nullptr;
}

//-----------------------------------------------------------------------------------------------
unsigned int Renderer::CreateOpenGLTexture(unsigned char* imageTexelBytes, int width, int height, int bytesPerTexel)
{
	// Create a texture ID (or "name" as OpenGL calls it) for this new texture
	unsigned int openGLTextureID = 0xFFFFFFFF;
	glGenTextures(1, &openGLTextureID);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture(GL_TEXTURE_2D, openGLTextureID);

	// Set texture clamp vs. wrap (repeat)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // one of: GL_CLAMP or GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // one of: GL_CLAMP or GL_REPEAT

																  // Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is currently in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if (bytesPerTexel == 3)
		bufferFormat = GL_RGB;

	GLenum internalFormat = bufferFormat; // the format we want the texture to me on the card; allows us to translate into a different texture format as we upload to OpenGL

	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		width,				// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,10], and B is the border thickness [0,1]
		height,				// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,10], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color/alpha channel)
		imageTexelBytes);	// Location of the actual pixel data bytes/buffer

	return openGLTextureID;
}

void Renderer::DrawCircle2D(const Vector2& center, float radius, const Rgba& lineColor)
{
	Vector3 center3(center.x, center.y, 0.f);
	DrawCircle3D(center3, radius, lineColor);
}

void Renderer::DrawCircle3D(const Vector3& center, float radius, const Rgba& lineColor)
{
	std::vector<Vertex3_PCT> vertexes;

	const int NUM_SIDES = 100;
	const float DEGREES_PER_SIDE = 3.6f;
	Vertex3_PCT newVert;

	glPushMatrix();
	glTranslatef(center.x, center.y, center.z);
	glScalef(radius, radius, 1.f);

	for (int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex)
	{
		newVert.m_color = lineColor;

		float angleDegrees = DEGREES_PER_SIDE * (float)sideIndex;
		float x = CosInDegrees(angleDegrees);
		float y = SinInDegrees(angleDegrees);

		newVert.m_position = Vector3(x, y, 0.f);
		vertexes.push_back(newVert);
	}
	DrawVertexArray3D_PCT(&vertexes[0], vertexes.size(), GL_LINE_LOOP, nullptr);
	glPopMatrix();
}

void Renderer::DrawVertexArray2D_PC( const Vertex2_PC* vertexArray, int numVertexes, unsigned int drawMode ) //#TODO: make copies for other vertex types
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glVertexPointer(2, GL_FLOAT, sizeof(Vertex2_PC), &vertexArray[0].m_position);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex2_PC), &vertexArray[0].m_color);
	
	glDrawArrays(drawMode, 0, numVertexes);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::DrawVertexArray3D_PCT( const Vertex3_PCT* vertexArray, int numVertexes, unsigned int drawMode, Texture* texture /*= nullptr*/ )
{
	if (texture == nullptr)
		BindTexture(defaultTexture);
	else
		BindTexture(texture);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex3_PCT), &vertexArray[0].m_position);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex3_PCT), &vertexArray[0].m_color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3_PCT), &vertexArray[0].m_texCoords);

	glDrawArrays(drawMode, 0, numVertexes);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Renderer::DrawVBO3D_PCT(unsigned int vboID, int numVertexes, unsigned int drawMode) 
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vboID);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex3_PCT), (const GLvoid*) offsetof(Vertex3_PCT, m_position));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex3_PCT), (const GLvoid*) offsetof(Vertex3_PCT, m_color));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex3_PCT),	(const GLvoid*) offsetof(Vertex3_PCT, m_texCoords));

	glDrawArrays(drawMode, 0, numVertexes);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

unsigned int Renderer::CreateVBOID()
{
	unsigned int vboID;
	glGenBuffers(1, &vboID);
	return vboID;
}

void Renderer::DestroyVBO(unsigned int vboID)
{
	glDeleteBuffers(1, &vboID);
}

void Renderer::UpdateVBO(unsigned int vboID, Vertex3_PCT* vertexArray, int numVertexes)
{
	size_t vertexNumBytes = numVertexes * sizeof(Vertex3_PCT);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, vertexNumBytes, vertexArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::DrawVertexArray3D_PC(const Vertex3_PC* vertexArray, int numVertexes, unsigned int drawMode) 
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(Vertex3_PC), &vertexArray[0].m_position);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex3_PC), &vertexArray[0].m_color);

	glDrawArrays(drawMode, 0, numVertexes);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void Renderer::SetPointSize(float pointSize)
{
	glPointSize(pointSize);
}

IntVector2 Renderer::GetDimensionsOfImage(const Texture& texture)
{
	std::string imageFilePath = texture.m_imageFilePath;
	int width = 0;
	int height = 0;
	int bytesPerTexel = 0;
	unsigned char* imageTexelBytes = stbi_load(imageFilePath.c_str(), &width, &height, &bytesPerTexel, 0);
	GUARANTEE_OR_DIE(imageTexelBytes != nullptr, Stringf("Failed to load image file \"%s\" - file not found!", imageFilePath.c_str()));
	GUARANTEE_OR_DIE(bytesPerTexel == 3 || bytesPerTexel == 4, Stringf("Failed to load image file \"%s\" - image had unsupported %i bytes per texel (must be 3 or 4)", imageFilePath.c_str(), bytesPerTexel));

	return IntVector2(width, height);
}

void Renderer::SetPerspective(float fovyDegrees, float aspectRatio, float nearClipDist, float farClipDist)
{
	glLoadIdentity();
	gluPerspective(fovyDegrees, aspectRatio, nearClipDist, farClipDist);
}

void Renderer::DrawText2D(const Vector2& startBottomLeft, const std::string& asciiText, float cellHeight, const Rgba& tint, float cellAspectScale, const BitmapFont* font)
{
	float cellWidth = cellHeight * cellAspectScale;
	Vector2 startOfText(startBottomLeft);

	//loop through text
	for (int textIndex = 0; textIndex < (int)asciiText.size(); textIndex++)
	{
		AABB2D fontTexCoords = font->GetTexCoordsForGlyph((int)asciiText[textIndex]);
		AABB2D worldBounds(startOfText, Vector2(startOfText.x + cellWidth, startOfText.y + cellHeight));
		Texture* texture = font->GetFontTexture();
		DrawTexturedSprite(worldBounds, *texture, fontTexCoords, tint, false);
		startOfText.x += cellWidth;
	}
}

BitmapFont* Renderer::CreateOrGetFont(const std::string& bitmapFontName)
{
	std::map< std::string, BitmapFont* >::iterator found = m_loadedFonts.find(bitmapFontName);

	if (found == m_loadedFonts.end())
	{
		BitmapFont* font = new BitmapFont(bitmapFontName);
		m_loadedFonts[bitmapFontName] = font;
		return font;
	}

	return found->second;
}

void Renderer::EnableBackFaceCulling()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void Renderer::BindTexture(Texture* texture)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture->m_textureID);
}

void Renderer::DrawFilledCircle2D(const Vertex3_PCT& center, const Vertex3_PCT& edge, float numSides /*= 64.0f*/)
{
	std::vector<Vertex3_PCT> verts;
	verts.reserve(static_cast<std::size_t>(numSides+1));
	verts.push_back(center);

	float anglePerVertex = 360.0f / numSides;
	float radius = CalcDistance(edge.m_position, center.m_position);
	for (float degrees = 0.0f; degrees <= 360.0f; degrees += anglePerVertex)
	{
		float radians = ConvertDegreesToRadians(degrees);
		float centerX = radius * cosf(radians) + center.m_position.x;
		float centerY = radius * sinf(radians) + center.m_position.y;
		verts.push_back(Vertex3_PCT(Vector3(centerX, centerY, 0.0f), edge.m_color, Vector2(0.0f, 0.0f)));
	}

	DrawVertexArray3D_PCT(verts.data(), verts.size(), TRIANGLE_FAN);
}