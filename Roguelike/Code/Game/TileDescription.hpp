#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Render/Rgba.hpp"
#include <string>
#include <map>
#include <vector>

class TileDescription
{
public:
	TileDescription();
	TileDescription(tinyxml2::XMLElement& element);
	~TileDescription();
public:
	std::string m_name;
	bool m_isOpaque;
	bool m_isSolid;
	std::vector<Rgba> m_color;
	std::vector<Rgba> m_charColor;
	std::vector<std::string> m_char;
	static std::map<std::string, TileDescription*> s_tileDefRegistry;
};