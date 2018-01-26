#pragma once
#include "Game/Entity.hpp"

class Character : public Entity
{
public:
	Character();
	virtual ~Character();
	virtual void Update(float deltaSeconds) override;
	virtual void Act();
	virtual void Render() const override;
	void EquipItemIfAvailableSlot(Item& item);
	void EquipItemIfAvailableSlot(Item* item);
	void ApplyEquippedStats(Stat* statToAdd);
	virtual void PickupItem(Item& item, Inventory& inventoryToRemove) override;
	virtual void DropItem(Item& item, Inventory& inventoryToAdd) override;
	Stat GetBaseWithoutEquipmentStats() override;
	void RemoveEquippedStats(Stat* statToRemove);
	void RemoveEquippedItem(Item& item);
	bool IsItemEquiped(Item& item);
	int GetStatByName(std::string& name);
	Item* GetEquippedItemByType(EquipType& type);
public:
	Item* m_equippedItems[static_cast<std::underlying_type<StatType>::type>(EquipType::NUM_EQUIP_TYPES)];
	std::string m_faction;
	Character* m_target;
};