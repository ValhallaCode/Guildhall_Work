#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Game/Stat.hpp"
#include <string>
#include <map>

class ItemDescription
{
public:
	ItemDescription();
	ItemDescription(tinyxml2::XMLElement& element);
	void PopulateStatArray(tinyxml2::XMLElement* statElement);
	~ItemDescription();
public:
	IntVector2 m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES)];
	std::string m_equipType;
	std::string m_name;
	std::string m_charToDraw;
	Rgba m_charColor;
	static std::map<std::string, ItemDescription*> s_itemDefRegistry;
};