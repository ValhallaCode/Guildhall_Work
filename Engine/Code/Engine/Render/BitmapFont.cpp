#include "Engine/Render/BitmapFont.hpp"

AABB2D BitmapFont::GetTexCoordsForGlyph(int glyphUnicode) const
{
	return m_spriteSheet->GetTexCoordsForSpriteIndex(glyphUnicode);
}

float BitmapFont::GetGlyphAspect(int glyphUnicode) const
{
	#define UNUSED(glyphUnicode) (void)(glyphUnicode);
	glyphUnicode;
	return 1.0f;
}

// void BitmapFont::SetSpriteSheetForFont(const std::string& imageFilePath, int tilesWide, int tilesHigh)
// {
// 	m_spriteSheet = new SpriteSheet(imageFilePath, tilesWide, tilesHigh);
// }

BitmapFont::BitmapFont(const std::string& bitmapFontName)
{
	m_spriteSheet = new SpriteSheet("Data/Fonts/"+bitmapFontName+".png", 16, 16);
}

BitmapFont::BitmapFont(const std::string& bitmapFontName, SimpleRenderer* renderer)
{
	m_spriteSheet = new SpriteSheet("Data/Fonts/" + bitmapFontName + ".png", 16, 16, renderer);
}

Texture* BitmapFont::GetFontTexture() const
{
	return m_spriteSheet->GetSpritesheetTextureGL();
}

Texture2D* BitmapFont::GetDirectXFontTexture() const
{
	return m_spriteSheet->GetSpritesheetTextureDX();
}

BitmapFont::~BitmapFont()
{
	delete m_spriteSheet;
	m_spriteSheet = nullptr;
}
