#include "Game/Stat.hpp"
#include "Engine/Math/MathUtils.hpp"

Stat::Stat()
{

}

Stat::Stat(IntVector2* statRangeArray)
{
	IntVector2 health = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)];
	IntVector2 evasion = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];
	IntVector2 toughness = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
	IntVector2 reflexes = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
	IntVector2 accuracy = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)];
	IntVector2 strength = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)];
	IntVector2 defense = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
	IntVector2 range = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::RANGE)];
	IntVector2 view = statRangeArray[static_cast<std::underlying_type<StatType>::type>(StatType::VIEW)];

	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)] = GetRandomIntInRange(strength.x, strength.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)] = GetRandomIntInRange(defense.x, defense.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)] = GetRandomIntInRange(health.x, health.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::MAX_HEALTH)] = m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)];
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)] = GetRandomIntInRange(evasion.x, evasion.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)] = GetRandomIntInRange(toughness.x, toughness.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)] = GetRandomIntInRange(reflexes.x, reflexes.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)] = GetRandomIntInRange(accuracy.x, accuracy.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::RANGE)] = GetRandomIntInRange(range.x, range.y);
	m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::VIEW)] = GetRandomIntInRange(view.x, view.y);
}

std::string Stat::GetStatNameByIndex(unsigned int index)
{
	StatType stat = static_cast<StatType>(index);
	switch (stat)
	{
		case StatType::STRENGTH: return "Strength";
		case StatType::DEFENSE: return "Defense";
		case StatType::HEALTH: return "Current Health";
		case StatType::MAX_HEALTH: return "Max Health";
		case StatType::EVASION: return "Evasion";
		case StatType::TOUGHNESS: return "Toughness";
		case StatType::REFLEXES: return "Reflexes";
		case StatType::ACCURACY: return "Accuracy";
		case StatType::RANGE: return "Range";
		case StatType::VIEW: return "View";
		default: break;
	}
	return "ERROR_INVALID_INDEX!";
}

unsigned int Stat::GetStatIndexByName(std::string& name)
{
	if (name == "Strength")
		return static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH);
	else if (name == "Defense")
		return static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE);
	else if (name == "Current Health")
		return static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH);
	else if (name == "Max Health")
		return static_cast<std::underlying_type<StatType>::type>(StatType::MAX_HEALTH);
	else if (name == "Evasion")
		return static_cast<std::underlying_type<StatType>::type>(StatType::EVASION);
	else if (name == "Toughness")
		return static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS);
	else if (name == "Reflexes")
		return static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES);
	else if (name == "Accuracy")
		return static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY);
	else if (name == "Range")
		return static_cast<std::underlying_type<StatType>::type>(StatType::RANGE);
	else if (name == "View")
		return static_cast<std::underlying_type<StatType>::type>(StatType::VIEW);
	return (unsigned int)-1;
}

Stat::~Stat()
{

}

