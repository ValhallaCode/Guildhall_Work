#include "Engine/Render/SpriteSheet.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Engine/Render/SimpleRenderer.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/EngineConfig.hpp"

SpriteSheet::SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh)
	:m_spriteLayout(tilesWide, tilesHigh)
	, m_spriteSheetTextureDX(nullptr)
{
	m_spriteSheetTextureGL = g_myRenderer->CreateOrGetTexture(imageFilePath);
}

SpriteSheet::SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh, SimpleRenderer* renderer)
	: m_spriteLayout(tilesWide, tilesHigh)
	, m_spriteSheetTextureGL(nullptr)
{
	m_spriteSheetTextureDX = CreateOrGetTexture2D(imageFilePath, renderer, imageFilePath);
}

AABB2D SpriteSheet::GetTexCoordsForSpriteCoords(int spriteX, int spriteY) const
{
	AABB2D texBox;
	texBox.mins.SetXY((float)spriteX / m_spriteLayout.x, (float)spriteY / m_spriteLayout.y);
	texBox.maxs.SetXY(texBox.mins.x + (1.f / m_spriteLayout.x), texBox.mins.y + (1.f / m_spriteLayout.y));

	if (m_spriteSheetTextureDX)
	{
		float min_y = texBox.mins.y;
		texBox.mins.y = texBox.maxs.y;
		texBox.maxs.y = min_y;
	}

	return texBox;
}

AABB2D SpriteSheet::GetTexCoordsForSpriteCoords(const IntVector2& spriteCoords) const
{
	return GetTexCoordsForSpriteCoords(spriteCoords.x, spriteCoords.y);
}

AABB2D SpriteSheet::GetTexCoordsForSpriteIndex(int spriteIndex) const
{
	IntVector2 spriteCoords = GetSpriteCoordsForSpriteIndex(spriteIndex);
	Vector2 texCoordsPerSpriteSheet(1.f / (float)m_spriteLayout.x, 1.f / (float)m_spriteLayout.y);

	AABB2D texBox;
	texBox.mins.SetXY((float)spriteCoords.x / (float)m_spriteLayout.x, (float)spriteCoords.y / (float)m_spriteLayout.y);
	texBox.maxs = texBox.mins + texCoordsPerSpriteSheet;

	if (m_spriteSheetTextureDX)
	{
		float min_y = texBox.mins.y;
		texBox.mins.y = texBox.maxs.y;
		texBox.maxs.y = min_y;
	}

	return texBox;
}

int SpriteSheet::GetNumSprites() const
{
	return m_spriteLayout.x * m_spriteLayout.y;
}

Texture* SpriteSheet::GetSpritesheetTextureGL() const
{
	return m_spriteSheetTextureGL;
}

Texture2D* SpriteSheet::GetSpritesheetTextureDX() const
{
	return m_spriteSheetTextureDX;
}

IntVector2 SpriteSheet::GetSpriteCoordsForSpriteIndex(int spriteIndex) const
{
	int spriteSheetX = spriteIndex % m_spriteLayout.x;
	int spriteSheetY = spriteIndex / m_spriteLayout.x;
	return IntVector2(spriteSheetX, spriteSheetY);
}