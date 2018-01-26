#pragma once
#include "Engine/RHI/Texture2D.hpp"
#include <string>
#include <map>
#include <vector>

struct Glyph {
	int x;
	int y;
	int width;
	int height;
	int xoffset;
	int yoffset;
	int xadvance;
	int chnl;
	Glyph()
		:x(0), 
		y(0), 
		width(0), 
		height(0), 
		xoffset(0), 
		yoffset(0), 
		xadvance(0), 
		chnl(0) 
	{};
};

using CharMap = std::map<int, Glyph>;
using KernMap = std::map<std::pair<int, int>, int>;
using PageMap = std::map<int, std::string>;

class KerningFont {
public:
	KerningFont();
	KerningFont(const std::string& filepath);
	~KerningFont();
	void LoadFontFromFile(const std::string& filename);
	unsigned int GetTextWidth(std::string text, float scale = 1.0f, float aspect = 1.0f);
	float GetTextHeight(std::string text, float scale /*= 1.0f*/);
	Glyph GetGlyph(const char character);
	Glyph GetInvalidGlyph();
	int GetKerning(int prevCharCode, const char character);
	
	PageMap m_pageMap;
	CharMap m_charMap;
	KernMap m_kernMap;
	std::vector<std::string> m_textureFilepaths;
	std::string m_name;
	std::string m_charset;
	unsigned char m_size;
	unsigned char m_superSamplerLevel;
	unsigned char m_alphaChnl;
	unsigned char m_redChnl;
	unsigned char m_greenChnl;
	unsigned char m_blueChnl;
	unsigned int m_lineHeight;
	unsigned int m_textureCount;
	unsigned int m_scaleW;
	unsigned int m_scaleH;
	unsigned int m_numPages;
	unsigned int m_paddingUp;
	unsigned int m_paddingDown;
	unsigned int m_paddingRight;
	unsigned int m_paddingLeft;
	unsigned int m_outline;
	unsigned int m_spacingH;
	unsigned int m_spacingV;
	unsigned int m_base;
	float m_stretchH;
	bool m_bold;
	bool m_italic;
	bool m_unicode;
	bool m_smooth;
	bool m_packed;
};