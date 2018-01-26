#include "Game/CharacterDescription.hpp"


CharacterDescription::CharacterDescription()
{

}

CharacterDescription::CharacterDescription(tinyxml2::XMLElement& element)
{
	ValidateXmlElement(element, "Character", "name,char,color,faction,start");
	m_name = ParseXmlAttribute(element, "name", std::string("ERROR_NAME_NOT_FOUND!"));
	m_charToDraw = ParseXmlAttribute(element, "char", std::string("."));
	m_charColor = ParseXmlAttribute(element, "color", Rgba(255, 128, 0, 255));
	m_faction = ParseXmlAttribute(element, "faction", std::string("ERROR_FACTION_NOT_FOUND!"));
	m_initBehavior = ParseXmlAttribute(element, "start", std::string("ERROR_BEHAVIOR_NOT_FOUND!"));

	tinyxml2::XMLElement* statElement = element.FirstChildElement("Stats");
	PopulateStatArray(statElement);
}

void CharacterDescription::PopulateStatArray(tinyxml2::XMLElement* statElement)
{
	ValidateXmlElement(*statElement, "Stats", "strength,defense,accuracy,health,evasion,toughness,reflexes,view");
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)] = ParseXmlAttribute(*statElement, "health", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)] = ParseXmlAttribute(*statElement, "evasion", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)] = ParseXmlAttribute(*statElement, "toughness", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)] = ParseXmlAttribute(*statElement, "reflexes", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)] = ParseXmlAttribute(*statElement, "accuracy", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)] = ParseXmlAttribute(*statElement, "strength", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)] = ParseXmlAttribute(*statElement, "defense", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::RANGE)] = ParseXmlAttribute(*statElement, "range", IntVector2(1, 1));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::VIEW)] = ParseXmlAttribute(*statElement, "view", IntVector2(1, 1));
}

CharacterDescription::~CharacterDescription()
{

}

std::map<std::string, CharacterDescription*> CharacterDescription::s_characterDefRegistry;
