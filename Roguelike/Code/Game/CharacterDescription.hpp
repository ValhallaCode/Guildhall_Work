#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Game/Stat.hpp"
#include <string>
#include <map>

class CharacterDescription
{
public:
	CharacterDescription();
	CharacterDescription(tinyxml2::XMLElement& element);
	void PopulateStatArray(tinyxml2::XMLElement* statElement);
	~CharacterDescription();
public:
	IntVector2 m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES)];
	std::string m_name;
	std::string m_charToDraw;
	std::string m_faction;
	std::string m_initBehavior;
	Rgba m_charColor;
	static std::map<std::string, CharacterDescription*> s_characterDefRegistry;
};