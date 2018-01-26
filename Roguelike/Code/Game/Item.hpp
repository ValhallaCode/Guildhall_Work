#pragma once
#include "Game/Stat.hpp"
#include "Game/ItemDescription.hpp"
#include "Engine/Render/Rgba.hpp"
#include <string>
#include <map>

enum class EquipType :  int
{
	NONE = -1,
	HEAD,
	BODY,
	LEGS,
	WEAPON,
	SHIELD,
	RING,
	NUM_EQUIP_TYPES
};

class Item
{
public:
	Item();
	Item(ItemDescription* itemDesc);
	~Item();
	EquipType ConvertStringToEquipType(std::string& string);
	std::string GetNameOfEquipType(EquipType& type);
public:
	std::string m_charToDraw;
	Rgba m_charColor;
	Stat* m_stats;
	std::string m_name;
	EquipType m_type;
};