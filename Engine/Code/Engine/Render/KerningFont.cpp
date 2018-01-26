#include "Engine/Render/KerningFont.hpp"
#include "ThirdParty/tinyXML/tinyxml2.h"
#include <map>
#include <tuple>
#include <sstream>
#include <utility>
#include <string>

KerningFont::KerningFont()
	:m_pageMap{},
	m_charMap{},
	m_kernMap{},
	m_name(""),
	m_textureCount(0),
	m_textureFilepaths{},
	m_size(0),
	m_lineHeight(0),
	m_scaleW(0),
	m_scaleH(0),
	m_numPages(0),
	m_bold(false),
	m_italic(false),
	m_charset(""),
	m_unicode(false),
	m_stretchH(0.0f),
	m_smooth(false),
	m_superSamplerLevel(0),
	m_paddingUp(0),
	m_paddingDown(0),
	m_paddingRight(0),
	m_paddingLeft(0),
	m_outline(0),
	m_spacingH(0),
	m_spacingV(0),
	m_base(0),
	m_packed(false),
	m_alphaChnl(0),
	m_redChnl(0),
	m_greenChnl(0),
	m_blueChnl(0)
{

}

KerningFont::KerningFont(const std::string& filepath)
	:KerningFont()
{
	LoadFontFromFile(filepath);
}

KerningFont::~KerningFont()
{

}
//TODO: Apply ASSERTs and Attribute queries
void KerningFont::LoadFontFromFile(const std::string& filename)
{
	tinyxml2::XMLDocument document;
	tinyxml2::XMLError result = document.LoadFile(filename.c_str());
	if (result != tinyxml2::XML_SUCCESS)
	{
		return;
	}

	auto xmlRoot = document.RootElement();
	if (xmlRoot == nullptr)
	{
		return;
	}

	auto xmlInfo = xmlRoot->FirstChildElement("info");
	if (xmlInfo == nullptr)
	{
		return;
	}
	else
	{
		m_name = xmlInfo->Attribute("face");
		m_size = static_cast<char>(xmlInfo->IntAttribute("size"));
		m_bold = xmlInfo->BoolAttribute("bold");
		m_italic = xmlInfo->BoolAttribute("italic");
		m_charset = xmlInfo->Attribute("charset");
		m_unicode = xmlInfo->BoolAttribute("unicode");
		m_stretchH = xmlInfo->FloatAttribute("stretchH") / 100.0f;
		m_smooth = xmlInfo->BoolAttribute("smooth");
		m_superSamplerLevel = xmlInfo->BoolAttribute("aa");
		m_outline = static_cast<unsigned int>(xmlInfo->IntAttribute("outline"));

		std::istringstream padding;
		padding.str(xmlInfo->Attribute("padding"));
		int temp = 0;
		std::string padString;
		while (std::getline(padding, padString, ','))
		{
			switch (temp)
			{
			case 0:
				m_paddingUp = static_cast<unsigned int>(std::stoi(padString));
				break;
			case 1:
				m_paddingRight = static_cast<unsigned int>(std::stoi(padString));
				break;
			case 2:
				m_paddingDown = static_cast<unsigned int>(std::stoi(padString));
				break;
			case 3:
				m_paddingLeft = static_cast<unsigned int>(std::stoi(padString));
				break;
			default:;
				//ASSERT format change
			}
			++temp;
		};

		temp = 0;

		std::istringstream spacing;
		spacing.str(xmlInfo->Attribute("spacing"));
		std::string spaceString;
		while (std::getline(spacing, spaceString, ','))
		{
			switch (temp)
			{
			case 0:
				m_spacingH = static_cast<unsigned int>(std::stoi(spaceString));
				break;
			case 1:
				m_spacingV = static_cast<unsigned int>(std::stoi(spaceString));
				break;
			default:;
				//ASSERT format change
			}
			++temp;
		};
	}

	auto xmlCommon = xmlRoot->FirstChildElement("common");
	if (xmlCommon == nullptr)
	{
		return;
	}
	else
	{
		m_lineHeight = static_cast<unsigned int>(xmlCommon->IntAttribute("lineHeight"));
		m_base = static_cast<unsigned int>(xmlCommon->IntAttribute("base"));
		m_scaleW = static_cast<unsigned int>(xmlCommon->IntAttribute("scaleW"));
		m_scaleH = static_cast<unsigned int>(xmlCommon->IntAttribute("scaleH"));
		m_numPages = static_cast<unsigned int>(xmlCommon->IntAttribute("pages"));
		m_packed = xmlCommon->BoolAttribute("packed");
		m_alphaChnl = static_cast<char>(xmlCommon->IntAttribute("alphaChnl"));
		m_redChnl = static_cast<char>(xmlCommon->IntAttribute("redChnl"));
		m_greenChnl = static_cast<char>(xmlCommon->IntAttribute("greenChnl"));
		m_blueChnl = static_cast<char>(xmlCommon->IntAttribute("blueChnl"));
	}

	auto xmlPages = xmlRoot->FirstChildElement("pages");
	if (xmlPages != nullptr)
	{
		for (auto xmlPage = xmlPages->FirstChildElement("page"); xmlPage != nullptr; xmlPage = xmlPage->NextSiblingElement())
		{
			int id = xmlRoot->IntAttribute("id");
			std::string pageName = std::string(xmlPage->Attribute("file"));
			m_pageMap.insert_or_assign(id, pageName);
			m_textureFilepaths.push_back(pageName);
		}
	}

	auto xmlChars = xmlRoot->FirstChildElement("chars");
	if (xmlChars != nullptr)
	{
		for (auto xmlChar = xmlChars->FirstChildElement("char"); xmlChar != nullptr; xmlChar = xmlChar->NextSiblingElement())
		{
			int id = xmlChar->IntAttribute("id");
			Glyph temp;
			temp.x = xmlChar->IntAttribute("x");
			temp.y = xmlChar->IntAttribute("y");
			temp.width = xmlChar->IntAttribute("width");
			temp.height = xmlChar->IntAttribute("height");
			temp.xoffset = xmlChar->IntAttribute("xoffset");
			temp.yoffset = xmlChar->IntAttribute("yoffset");
			temp.xadvance = xmlChar->IntAttribute("xadvance");
			temp.chnl = xmlChar->IntAttribute("chnl");
			m_charMap.insert_or_assign(id, temp);
		}
	}

	auto xmlKerns = xmlRoot->FirstChildElement("kernings");
	if (xmlKerns != nullptr)
	{
		for (auto xmlKern = xmlKerns->FirstChildElement("kerning"); xmlKern != nullptr; xmlKern = xmlKern->NextSiblingElement())
		{
			std::pair<int, int> temp;
			std::get<0>(temp) = xmlKern->IntAttribute("first");
			std::get<1>(temp) = xmlKern->IntAttribute("second");
			int amount = xmlKern->IntAttribute("amount");
			m_kernMap.insert_or_assign(temp, amount);
		}
	}
}

unsigned int KerningFont::GetTextWidth(std::string text, float scale /*= 1.0f*/, float aspect /*= 1.0f*/)
{
	unsigned cursor = 0;

	const char* c = text.c_str();
	int prevCharCode = -1;


	while (*c != NULL) {
		const Glyph glyph = GetGlyph(*c);

		cursor += glyph.xadvance;
		prevCharCode = *c;
		++c;

		int kerning = GetKerning(prevCharCode, *c);
		cursor += kerning;
	}

	return (unsigned int)((cursor * scale) * aspect);
}

float KerningFont::GetTextHeight(std::string text, float scale /*= 1.0f*/)
{
	int maxHeight = 0;

	for (unsigned int i = 0; i < text.length(); i++)
	{
		char c = text[i];
		Glyph glyph = GetGlyph(c);
		if (glyph.height > maxHeight)
			maxHeight = glyph.height;
	}

	return (float)maxHeight * scale;
}

Glyph KerningFont::GetGlyph(const char character) 
{
	int charVal = int(character);
	CharMap::iterator iterate = m_charMap.find(charVal);
	if (iterate != m_charMap.end())
		return iterate->second;
	else {
		//ASSERT
		return GetInvalidGlyph();
	}
}

Glyph KerningFont::GetInvalidGlyph()
{
	CharMap::iterator iterate = m_charMap.find(-1);
	if (iterate != m_charMap.end())
		return iterate->second;
	else {
		//ASSERT
		return Glyph();
	}
}

int KerningFont::GetKerning(int prevCharCode, const char character)
{
	KernMap::iterator iterate = m_kernMap.find(std::pair<int,int>(prevCharCode, int(character)));
	if (iterate != m_kernMap.end())
		return iterate->second;
	else {
		//ASSERT
		return 0;
	}
}