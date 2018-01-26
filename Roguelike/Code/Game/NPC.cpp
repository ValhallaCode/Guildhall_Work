#include "Game/NPC.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Map.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Flee.hpp"
#include "Game/Behavior.hpp"
#include "Game/Item.hpp"
#include "Game/ItemDescription.hpp"
#include <iterator>

NPC::NPC()
{
	m_target = nullptr;
}

NPC::NPC(CharacterDescription* charDesc)
{
	m_target = nullptr;
	m_currentTile = nullptr;
	m_currentMap = nullptr;

	m_name = charDesc->m_name;
	m_charToDraw = charDesc->m_charToDraw;
	m_charColor = charDesc->m_charColor;
	m_faction = charDesc->m_faction;

	m_stats = new Stat(charDesc->m_statRanges);

	m_inventory = new Inventory(20);
	SelectFromLootList();
	EquipBestItemsFromInventory();

	m_behavior = Behavior::s_behaviorList.find(m_name)->second.find(charDesc->m_initBehavior)->second->Clone();
	m_behavior->SetActingCharacter(this);

	m_font = g_simpleRenderer->CreateOrGetBitmapFont("PixelFont");
	m_sampler = g_simpleRenderer->CreateOrGetSampler("PointToPoint", eFilterMode::FILTER_POINT, eFilterMode::FILTER_POINT);
	m_shaderProgram = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/nop_textured.hlsl");
}

void NPC::SelectFromLootList()
{
	unsigned int totalNumOfRandomItems;
	unsigned int numItemsInLootTable = s_lootList.find(m_name)->second.size();
	if (numItemsInLootTable < m_inventory->m_maxItemCount)
		totalNumOfRandomItems = numItemsInLootTable;
	else
		totalNumOfRandomItems = m_inventory->m_maxItemCount;

	int randomNumItemsToGet = GetRandomIntInRange(1, (int)totalNumOfRandomItems);
	std::map<std::string, int> myLootList = s_lootList.find(m_name)->second;

	for (int numItemsGot = 0; numItemsGot < randomNumItemsToGet; ++numItemsGot)
	{
		int randomIndexToGet = GetRandomIntInRange(0, numItemsInLootTable);

		auto iterate = myLootList.begin();
		std::advance(iterate, randomIndexToGet);

		if (iterate == myLootList.end())
			std::advance(iterate, -1);

		std::string itemName = iterate->first;
		//int itemWeight = iterate->second;

		ItemDescription* itemDesc = ItemDescription::s_itemDefRegistry.find(itemName)->second;
		m_inventory->m_itemList.push_back(new Item(itemDesc));
	}
}

void NPC::EquipBestItemsFromInventory()
{
	for (int equipIndex = 0; equipIndex < (int)EquipType::NUM_EQUIP_TYPES; ++equipIndex)
	{
		switch (equipIndex) 
		{
			case (int)EquipType::HEAD: EquipBestHelm();
			case (int)EquipType::BODY: EquipBestChestPiece();
			case (int)EquipType::LEGS: EquipBestLegPiece();
			case (int)EquipType::SHIELD: EquipBestShield();
			case (int)EquipType::WEAPON: EquipBestWeapon();
			case (int)EquipType::RING: EquipBestRing();
			default: continue;
		}
	}
}

void NPC::EquipBestHelm()
{
	Item* bestItem = nullptr;
	for (unsigned int index = 0; index < m_inventory->m_itemList.size(); ++index)
	{
		if (bestItem == nullptr && m_inventory->m_itemList[index]->m_type == EquipType::HEAD)
			bestItem = m_inventory->m_itemList[index];

		if (m_inventory->m_itemList[index]->m_type == EquipType::HEAD)
		{
			Item* currentItem = m_inventory->m_itemList[index];
			int currentItemDefense = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int currentItemToughness = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int currentItemReflexes = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int currentItemEvasion = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			int bestItemDefense = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int bestItemToughness = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int bestItemReflexes = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int bestItemEvasion = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			if (currentItemDefense + currentItemToughness + currentItemReflexes + currentItemEvasion > bestItemDefense + bestItemToughness + bestItemReflexes + bestItemEvasion)
				bestItem = currentItem;
		}
	}
	if(bestItem != nullptr)
		EquipItemIfAvailableSlot(*bestItem);
}

void NPC::EquipBestChestPiece()
{
	Item* bestItem = nullptr;
	for (unsigned int index = 0; index < m_inventory->m_itemList.size(); ++index)
	{
		if (bestItem == nullptr && m_inventory->m_itemList[index]->m_type == EquipType::BODY)
			bestItem = m_inventory->m_itemList[index];

		if (m_inventory->m_itemList[index]->m_type == EquipType::BODY)
		{
			Item* currentItem = m_inventory->m_itemList[index];
			int currentItemDefense = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int currentItemToughness = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int currentItemReflexes = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int currentItemEvasion = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			int bestItemDefense = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int bestItemToughness = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int bestItemReflexes = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int bestItemEvasion = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			if (currentItemDefense + currentItemToughness + currentItemReflexes + currentItemEvasion > bestItemDefense + bestItemToughness + bestItemReflexes + bestItemEvasion)
				bestItem = currentItem;
		}
	}
	if (bestItem != nullptr)
		EquipItemIfAvailableSlot(*bestItem);
}

void NPC::EquipBestLegPiece()
{
	Item* bestItem = nullptr;
	for (unsigned int index = 0; index < m_inventory->m_itemList.size(); ++index)
	{
		if (bestItem == nullptr && m_inventory->m_itemList[index]->m_type == EquipType::LEGS)
			bestItem = m_inventory->m_itemList[index];

		if (m_inventory->m_itemList[index]->m_type == EquipType::LEGS)
		{
			Item* currentItem = m_inventory->m_itemList[index];
			int currentItemDefense = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int currentItemToughness = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int currentItemReflexes = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int currentItemEvasion = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			int bestItemDefense = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int bestItemToughness = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int bestItemReflexes = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int bestItemEvasion = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			if (currentItemDefense + currentItemToughness + currentItemReflexes + currentItemEvasion > bestItemDefense + bestItemToughness + bestItemReflexes + bestItemEvasion)
				bestItem = currentItem;
		}
	}
	if (bestItem != nullptr)
		EquipItemIfAvailableSlot(*bestItem);
}

void NPC::EquipBestWeapon()
{
	Item* bestItem = nullptr;
	for (unsigned int index = 0; index < m_inventory->m_itemList.size(); ++index)
	{
		if (bestItem == nullptr && m_inventory->m_itemList[index]->m_type == EquipType::WEAPON)
			bestItem = m_inventory->m_itemList[index];

		if (m_inventory->m_itemList[index]->m_type == EquipType::WEAPON)
		{
			Item* currentItem = m_inventory->m_itemList[index];
			int currentItemAccuracy = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)];
			int currentItemStrength = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)];
			int currentItemReflexes = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];

			int bestItemAccuracy = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)];
			int bestItemStrength = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)];
			int bestItemReflexes = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];

			if (currentItemAccuracy + currentItemStrength + currentItemReflexes > bestItemAccuracy + bestItemStrength + bestItemReflexes)
				bestItem = currentItem;
		}
	}
	if (bestItem != nullptr)
		EquipItemIfAvailableSlot(*bestItem);
}

void NPC::EquipBestShield()
{
	Item* bestItem = nullptr;
	for (unsigned int index = 0; index < m_inventory->m_itemList.size(); ++index)
	{
		if (bestItem == nullptr && m_inventory->m_itemList[index]->m_type == EquipType::SHIELD)
			bestItem = m_inventory->m_itemList[index];

		if (m_inventory->m_itemList[index]->m_type == EquipType::SHIELD)
		{
			Item* currentItem = m_inventory->m_itemList[index];
			int currentItemDefense = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int currentItemToughness = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int currentItemReflexes = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int currentItemEvasion = currentItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			int bestItemDefense = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];
			int bestItemToughness = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
			int bestItemReflexes = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::REFLEXES)];
			int bestItemEvasion = bestItem->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];

			if (currentItemDefense + currentItemToughness + currentItemReflexes + currentItemEvasion > bestItemDefense + bestItemToughness + bestItemReflexes + bestItemEvasion)
				bestItem = currentItem;
		}
	}
	if (bestItem != nullptr)
		EquipItemIfAvailableSlot(*bestItem);
}

void NPC::EquipBestRing()
{
	Item* bestItem = nullptr;
	int bestItemTotal = 0;
	for (unsigned int index = 0; index < m_inventory->m_itemList.size(); ++index)
	{
		if (bestItem == nullptr && m_inventory->m_itemList[index]->m_type == EquipType::RING)
		{
			bestItem = m_inventory->m_itemList[index];
			for (unsigned int statIndex = 0; statIndex < (unsigned int)StatType::NUM_STAT_TYPES; ++statIndex)
			{
				bestItemTotal += bestItem->m_stats->m_statCollection[statIndex];
			}
		}

		if (m_inventory->m_itemList[index]->m_type == EquipType::RING)
		{
			Item* currentItem = m_inventory->m_itemList[index];
			int currentItemTotal = 0;
			for (unsigned int statIndex = 0; statIndex < (unsigned int)StatType::NUM_STAT_TYPES; ++statIndex)
			{
				currentItemTotal += currentItem->m_stats->m_statCollection[statIndex];
			}

			if (currentItemTotal > bestItemTotal)
			{
				bestItem = currentItem;
				bestItemTotal = currentItemTotal;
			}
		}
	}
	if (bestItem != nullptr)
		EquipItemIfAvailableSlot(*bestItem);
}

NPC::~NPC()
{

}

void NPC::Update(float deltaSeconds)
{
	RemoveOldDamageNumbers();
	UpdateDamageNumbers(deltaSeconds);
}

void NPC::Act()
{
	LookForActorsInView();
	PickTargetFromSeenList();
	UpdateBehavior();
	m_behavior->Act();
}

void NPC::PickTargetFromSeenList()
{
	if (m_target != nullptr)
		return;

	for (Character* character : m_actorsSeen)
	{
		if (character->m_faction != m_faction)
		{
			m_target = character;
			break;
		}
	}
}

void NPC::Render() const
{
	if (m_currentTile->m_visibleState != CAN_SEE)
		return;

	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->SetShaderProgram(m_shaderProgram);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawBitmapFont(m_font, m_bounds.mins + Vector2(-0.09f, -0.1f), m_charToDraw, m_bounds.CalcSize().y, m_charColor, 0.8f);
	m_behavior->Render();
	g_simpleRenderer->DisableBlend();

	RenderDamageNumbers();
}

Behavior* NPC::ChooseBestBehaviorBasedOnUtilityCost()
{
	std::map<std::string, Behavior*> myBehaviors = Behavior::s_behaviorList.find(m_name)->second;

	std::string nameOfBestCost;
	int bestCost = -1;
	for (auto iterate : myBehaviors)
	{
		Behavior* behavior = iterate.second;
		behavior->SetActingCharacter(this);
		if (behavior->m_name == "Flee" && m_behavior->m_name == "Flee")
		{
			Flee* flee = dynamic_cast<Flee*>(behavior);
			Flee* mflee = dynamic_cast<Flee*>(m_behavior);
			flee->m_turnsTaken = mflee->m_turnsTaken;
		}


		int currentCost = behavior->CalculateUtility();
		if (currentCost > bestCost)
		{
			bestCost = currentCost;
			nameOfBestCost = iterate.first;
		}
	}

	return myBehaviors.find(nameOfBestCost)->second;
}

void NPC::UpdateBehavior()
{
	Behavior* bestSelected = ChooseBestBehaviorBasedOnUtilityCost();

	if (m_behavior->m_name == bestSelected->m_name)
		return;

	delete m_behavior;
	m_behavior = bestSelected->Clone();
	m_behavior->SetActingCharacter(this);
}

std::map < std::string, std::map<std::string, int>> NPC::s_lootList;
