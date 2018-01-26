#pragma once
#include "Game/Character.hpp"
#include "Game/CharacterDescription.hpp"
#include <map>

class Behavior;

class NPC : public Character
{
public:
	NPC();
	void SelectFromLootList();
	void EquipBestItemsFromInventory();
	void EquipBestHelm();
	void EquipBestChestPiece();
	void EquipBestLegPiece();
	void EquipBestWeapon();
	void EquipBestShield();
	Behavior* NPC::ChooseBestBehaviorBasedOnUtilityCost();
	void EquipBestRing();
	virtual ~NPC();
	void NPC::UpdateBehavior();
	NPC(CharacterDescription* charDesc);
	virtual void Update(float deltaSeconds) override;
	virtual void Act() override;
	void PickTargetFromSeenList();
	virtual void Render() const override;
public:
	Behavior* m_behavior;
	static std::map < std::string, std::map<std::string, int>> s_lootList;
};