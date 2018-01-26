#include "Game/TileDescription.hpp"

TileDescription::TileDescription()
{

}

TileDescription::TileDescription(tinyxml2::XMLElement& element)
{
	ValidateXmlElement(element, "TileDescription", "name,isOpaque,isSolid,char,color,charColor");
	m_name = ParseXmlAttribute(element, "name", std::string("ERROR_NAME_NOT_FOUND!"));
	m_isSolid = ParseXmlAttribute(element, "isSolid", false);
	m_isOpaque = ParseXmlAttribute(element, "isOpaque", true);
	m_char.push_back(ParseXmlAttribute(element, "char", std::string(".")));
	m_color.push_back(ParseXmlAttribute(element, "color", Rgba(255, 128, 0, 255)));
	m_charColor.push_back(ParseXmlAttribute(element, "charColor", Rgba(255, 128, 0, 255)));

	auto child = element.FirstChildElement("Additional");
	if (child != nullptr)
	{
		ValidateXmlElement(*child, "Additional", "charColor,char,color");
		for (auto iterate = child; iterate != nullptr; iterate = iterate->NextSiblingElement())
		{
			m_char.push_back(ParseXmlAttribute(*iterate, "char", std::string(".")));
			m_color.push_back(ParseXmlAttribute(*iterate, "color", Rgba(255, 128, 0, 255)));
			m_charColor.push_back(ParseXmlAttribute(*iterate, "charColor", Rgba(255, 128, 0, 255)));
		}
	}
}

TileDescription::~TileDescription()
{

}

std::map<std::string, TileDescription*> TileDescription::s_tileDefRegistry;

