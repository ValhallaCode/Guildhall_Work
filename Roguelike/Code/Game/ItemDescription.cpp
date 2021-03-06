#include "Game/ItemDescription.hpp"


ItemDescription::ItemDescription()
{

}

ItemDescription::ItemDescription(tinyxml2::XMLElement& element)
{
	ValidateXmlElement(element, "Item", "name,type,char,charColor");
	m_name = ParseXmlAttribute(element, "name", std::string("ERROR_ITEM_NAME_NOT_GIVEN!"));
	m_charToDraw = ParseXmlAttribute(element, "char", std::string("."));
	m_charColor = ParseXmlAttribute(element, "charColor", Rgba(255, 128, 0, 255));
	m_equipType = ParseXmlAttribute(element, "type", std::string("ERROR_ITEM_TYPE_NOT_GIVEN!"));

	tinyxml2::XMLElement* statElement = element.FirstChildElement("Stats");
	PopulateStatArray(statElement);
}

void ItemDescription::PopulateStatArray(tinyxml2::XMLElement* statElement)
{
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

ItemDescription::~ItemDescription()
{

}

std::map<std::string, ItemDescription*> ItemDescription::s_itemDefRegistry;

