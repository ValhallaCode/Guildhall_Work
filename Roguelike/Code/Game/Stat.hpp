#pragma once
#include "Engine/Math/IntVector2.hpp"
#include <type_traits>
#include <string>

enum class StatType : unsigned int 
{
	STRENGTH,
	DEFENSE,
	ACCURACY,
	HEALTH,
	MAX_HEALTH,
	EVASION,
	TOUGHNESS,
	REFLEXES,
	RANGE,
	VIEW,
	NUM_STAT_TYPES
};


class Stat
{
public:
	Stat();
	Stat(IntVector2* statRangeArray);
	std::string GetStatNameByIndex(unsigned int index);
	unsigned int GetStatIndexByName(std::string& name);
	~Stat();
public:
	int m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES)];
};