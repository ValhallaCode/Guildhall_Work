#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Game/Stat.hpp"
#include <string>
#include <map>

class InteractableDescription
{
public:
	InteractableDescription();
	InteractableDescription(tinyxml2::XMLElement& element);
	void PopulateStatArray(tinyxml2::XMLElement* statElement);
	~InteractableDescription();
public:
	IntVector2 m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES)];
	std::string m_name;
	std::string m_charToDraw;
	std::string m_tileNameToPlace;
	Rgba m_charColor;
	Rgba m_color;
	int m_lightValue;
	bool m_isLIght;
	bool m_isExit;
	bool m_isSolid;
	bool m_isOpaque;
	static std::map<std::string, InteractableDescription*> s_interactableDefRegistry;
};