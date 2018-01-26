#include "Game/Character.hpp"
#include "Game/GameCommons.hpp"

Character::Character()
	:m_target(nullptr)
{
	for (int index = 0; index < (int)EquipType::NUM_EQUIP_TYPES; ++index)
	{
		m_equippedItems[index] = nullptr;
	}
}

Character::~Character()
{

}

void Character::Update(float deltaSeconds)
{
	deltaSeconds;
}

void Character::Act()
{

}

void Character::Render() const
{

}

void Character::EquipItemIfAvailableSlot(Item& item)
{
	if (item.m_type == EquipType::NONE)
		return;

	Item* checkedSlot = m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item.m_type)];
	if (checkedSlot == nullptr)
	{
		m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item.m_type)] = &item;
		ApplyEquippedStats(item.m_stats);
	}
}

void Character::EquipItemIfAvailableSlot(Item* item)
{
	if (item->m_type == EquipType::NONE)
		return;

	Item* checkedSlot = m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item->m_type)];
	if (checkedSlot == nullptr)
	{
		m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item->m_type)] = item;
		ApplyEquippedStats(item->m_stats);
	}
}

void Character::ApplyEquippedStats(Stat* statToAdd)
{
	for (unsigned int index = 0; index < static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES); ++index)
	{
		m_stats->m_statCollection[index] += statToAdd->m_statCollection[index];
	}
}

void Character::PickupItem(Item& item, Inventory& inventoryToRemove)
{
	if (m_inventory->m_itemList.size() < m_inventory->m_maxItemCount)
	{
		inventoryToRemove.RemoveItem(&item);
		m_inventory->m_itemList.push_back(&item);
		EquipItemIfAvailableSlot(item);
	}
}

void Character::DropItem(Item& item, Inventory& inventoryToAdd)
{
	if (!m_inventory->m_itemList.empty())
	{
		inventoryToAdd.m_itemList.push_back(&item);
		RemoveEquippedItem(item);
		m_inventory->RemoveItem(&item);
	}
}

Stat Character::GetBaseWithoutEquipmentStats()
{
	Stat currentStats = *m_stats;

	for(unsigned int equipIndex = 0; equipIndex < 6; ++equipIndex)
	{
		Item* currentItem = m_equippedItems[equipIndex];
		if (currentItem == nullptr)
			continue;

		//Remove Stats
		for (unsigned int indexStat = 0; indexStat < static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES); ++indexStat)
		{
			currentStats.m_statCollection[indexStat] -= currentItem->m_stats->m_statCollection[indexStat];
		}
	}

	return currentStats;
}

void Character::RemoveEquippedStats(Stat* statToRemove)
{
	for (unsigned int index = 0; index < static_cast<std::underlying_type<StatType>::type>(StatType::NUM_STAT_TYPES); ++index)
	{
		m_stats->m_statCollection[index] -= statToRemove->m_statCollection[index];
	}
}

void Character::RemoveEquippedItem(Item& item)
{
	if (!IsItemEquiped(item))
		return;

	Item* checkedSlot = m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item.m_type)];
	if (checkedSlot != nullptr)
	{
		m_equippedItems[static_cast<std::underlying_type<StatType>::type>(item.m_type)] = nullptr;
		RemoveEquippedStats(item.m_stats);
	}
}

bool Character::IsItemEquiped(Item& item)
{
	for (unsigned int index = 0; index < (unsigned int)EquipType::NUM_EQUIP_TYPES; ++index)
	{
		Item* equipItem = m_equippedItems[index];
		
		if (equipItem == &item)
			return true;
	}
	return false;
}

int Character::GetStatByName(std::string& name)
{
	unsigned int index = m_stats->GetStatIndexByName(name);
	return m_stats->m_statCollection[index];
}

Item* Character::GetEquippedItemByType(EquipType& type)
{
	return m_equippedItems[static_cast<std::underlying_type<StatType>::type>(type)];
}