#include "Game/Item.hpp"

Item::Item()
{

}

Item::Item(ItemDescription* itemDesc)
{
	m_name = itemDesc->m_name;
	m_charToDraw = itemDesc->m_charToDraw;
	m_charColor = itemDesc->m_charColor;

	std::string equipType = itemDesc->m_equipType;
	m_type = ConvertStringToEquipType(equipType);

	m_stats = new Stat(itemDesc->m_statRanges);
}

Item::~Item()
{

}

EquipType Item::ConvertStringToEquipType(std::string& string)
{
	if (string == "head")
		return EquipType::HEAD;
	else if (string == "body")
		return EquipType::BODY;
	else if (string == "legs")
		return EquipType::LEGS;
	else if (string == "weapon")
		return EquipType::WEAPON;
	else if (string == "ring")
		return EquipType::RING;
	else if (string == "shield")
		return EquipType::SHIELD;
	else 
		return EquipType::NONE;
}

std::string Item::GetNameOfEquipType(EquipType& type)
{
	switch (type)
	{
	case EquipType::HEAD: return "Head";
	case EquipType::BODY: return "Body";
	case EquipType::LEGS: return "Legs";
	case EquipType::WEAPON: return "Weapon";
	case EquipType::RING: return "Ring";
	case EquipType::SHIELD: return "Shield";
	default: return "None";
	}
}

