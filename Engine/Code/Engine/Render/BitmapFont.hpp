#pragma once
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Render/SpriteSheet.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include <string>

class BitmapFont
{
	friend class Renderer;
	friend class SimpleRenderer;

public:
	AABB2D GetTexCoordsForGlyph(int glyphUnicode) const; // pass ‘A’ or 65 for A, etc.
	float GetGlyphAspect(int glyphUnicode) const; // used more later
	Texture* GetFontTexture() const;
	Texture2D* GetDirectXFontTexture() const;
	//void SetSpriteSheetForFont(const std::string& imageFilePath, int tilesWide, int tilesHigh);
	BitmapFont(const std::string& bitmapFontName, SimpleRenderer* renderer);
	~BitmapFont();

private:
	BitmapFont(const std::string& bitmapFontName); // must be constructed by Renderer
	SpriteSheet* m_spriteSheet; // used internally; assumed to be a 16x16 glyph sheet
};