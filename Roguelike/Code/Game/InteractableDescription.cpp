#include "Game/InteractableDescription.hpp"


InteractableDescription::InteractableDescription()
{

}

InteractableDescription::InteractableDescription(tinyxml2::XMLElement& element)
{
	ValidateXmlElement(element, "Player", "name,char,charColor,color,isSolid,isOpaque");
	m_name = ParseXmlAttribute(element, "name", std::string("ERROR_NAME_NOT_ASSIGNED_TO_PLAYER"));
	m_charToDraw = ParseXmlAttribute(element, "char", std::string("."));
	m_charColor = ParseXmlAttribute(element, "charColor", Rgba(255, 128, 0, 255));
	m_color = ParseXmlAttribute(element, "color", Rgba(255, 128, 0, 255));
	m_isSolid = ParseXmlAttribute(element, "isSolid", false);
	m_isOpaque = ParseXmlAttribute(element, "isOpaque", false);

	tinyxml2::XMLElement* exitElement = element.FirstChildElement("Exit");

	m_isExit = ParseXmlAttribute(*exitElement, "isExit", false);
	m_tileNameToPlace = ParseXmlAttribute(*exitElement, "tile", std::string("TILE_NAME_NOT_GIVEN!"));

	tinyxml2::XMLElement* lightElement = element.FirstChildElement("Light");
	m_lightValue = ParseXmlAttribute(*lightElement, "value", 10);
	m_isLIght = ParseXmlAttribute(*lightElement, "isLight", false);

	tinyxml2::XMLElement* statElement = element.FirstChildElement("Stats");
	PopulateStatArray(statElement);
}

void InteractableDescription::PopulateStatArray(tinyxml2::XMLElement* statElement)
{
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)] = ParseXmlAttribute(*statElement, "health", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)] = ParseXmlAttribute(*statElement, "evasion", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)] = ParseXmlAttribute(*statElement, "toughness", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)] = ParseXmlAttribute(*statElement, "reflexes", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)] = ParseXmlAttribute(*statElement, "accuracy", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)] = ParseXmlAttribute(*statElement, "strength", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)] = ParseXmlAttribute(*statElement, "defense", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::RANGE)] = ParseXmlAttribute(*statElement, "range", IntVector2(0, 0));
	m_statRanges[static_cast<std::underlying_type<StatType>::type>(StatType::VIEW)] = ParseXmlAttribute(*statElement, "view", IntVector2(0, 0));
}

InteractableDescription::~InteractableDescription()
{

}

std::map<std::string, InteractableDescription*> InteractableDescription::s_interactableDefRegistry;
