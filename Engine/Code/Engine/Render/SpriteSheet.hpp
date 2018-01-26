#pragma once
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Render/Texture.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include <string>

class SimpleRenderer;

class SpriteSheet
{
public:
	SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh);
	SpriteSheet(const std::string& imageFilePath, int tilesWide, int tilesHigh, SimpleRenderer* renderer);
	AABB2D GetTexCoordsForSpriteCoords(int spriteX, int spriteY) const;
	AABB2D GetTexCoordsForSpriteCoords(const IntVector2& spriteCoords) const;
	AABB2D GetTexCoordsForSpriteIndex(int spriteIndex) const;
	int GetNumSprites() const;
	Texture* GetSpritesheetTextureGL() const;
	Texture2D* GetSpritesheetTextureDX() const;
	IntVector2 GetSpriteCoordsForSpriteIndex(int spriteIndex) const;

private:
	Texture* 	m_spriteSheetTextureGL;
	Texture2D* m_spriteSheetTextureDX;
	IntVector2	m_spriteLayout;
};
