#include "Game/Entity.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Tile.hpp"
#include "Engine/EngineConfig.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Character.hpp"

Entity::Entity()
	:m_currentTile(nullptr)
	, m_currentMap(nullptr)
{
	
}

Entity::~Entity()
{

}

void Entity::Update(float deltaSeconds)
{
	deltaSeconds;
}

void Entity::SetCurrentTileOn(Tile* tile)
{
	if (m_currentTile != nullptr) {
		m_currentTile->SetEntityOnTile(nullptr);
	}

	m_currentTile = tile;
	IntVector2 position = tile->GetPositionInMap();
	UpdateBoxPosition(position);
	m_currentTile->SetEntityOnTile(this);
}

Tile* Entity::GetCurrentTileOn()
{
	return m_currentTile;
}

void Entity::UpdateBoxPosition(IntVector2& position)
{
	m_bounds = AABB2D(Vector2((float)position.x, (float)position.y), Vector2((float)position.x + 1.0f, (float)position.y + 1.0f));
}

IntVector2 Entity::GetPositionInMap()
{
	return m_currentTile->GetPositionInMap();
}

void Entity::SetCurrentMap(Map* map)
{
	m_currentMap = map;
}

Map* Entity::GetCurrentMap()
{
	return m_currentMap;
}

void Entity::Render() const
{
	
}

void Entity::DropItem(Item& item, Inventory& inventoryToAdd)
{
	if (m_inventory->m_itemList.size() > 0)
	{
		inventoryToAdd.m_itemList.push_back(&item);
		m_inventory->RemoveItem(&item);
	}
}

void Entity::PickupItem(Item& item, Inventory& inventoryToRemove)
{
	if (m_inventory->m_itemList.size() <= m_inventory->m_maxItemCount)
	{
		inventoryToRemove.RemoveItem(&item);
		m_inventory->m_itemList.push_back(&item);
	}
}

void Entity::UpdateDamageNumbers(float deltaSeconds)
{
	if (m_damageNumbers.empty())
		return;

	for (unsigned int index = 0; index < m_damageNumbers.size(); ++index)
	{
		DamageIndicator& indicator = m_damageNumbers[index];
		indicator.m_position.y += 1.0f * deltaSeconds;
		if (indicator.m_color.a - 5 > 0)
			indicator.m_color.a -= 5;
		else
			indicator.m_color.a = 0;
	}
}

void Entity::RemoveOldDamageNumbers()
{
	for (unsigned int index = 0; index < m_damageNumbers.size(); ++index)
	{
		DamageIndicator& indicator = m_damageNumbers[index];
		if (indicator.m_color.a == 0)
		{	
			m_damageNumbers.erase(m_damageNumbers.begin() + index);
			--index;
		}
	}
}

void Entity::RenderDamageNumbers() const 
{
	int windowWidth = DEFAULT_WINDOW_WIDTH;
	int orthoWidth = WORLD_WIDTH;
	float pixelRatio = (float)orthoWidth / (float)windowWidth;
	float fontSize = 0.5f;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	for(DamageIndicator indicator : m_damageNumbers)
	{
		//g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, indicator.m_position.x, indicator.m_position.y, indicator.m_damage, indicator.m_color, (fontSize + 0.1f) * pixelRatio);
		g_simpleRenderer->DrawTextCenteredOnPosition2D(g_theGame->m_font, Vector2(indicator.m_position.x, indicator.m_position.y), indicator.m_damage, indicator.m_color, (fontSize + 0.1f) * pixelRatio);// (fontSize + 0.1f) * pixelRatio);
	}

	g_simpleRenderer->DisableBlend();
}

Stat Entity::GetBaseWithoutEquipmentStats()
{
	return *m_stats;
}

void Entity::PlayerCheckForVisibleTiles()
{
	if (!m_seenTiles.empty())
	{
		for (IntVector2 location : m_seenTiles)
		{
			Tile* tile = m_currentMap->GetTileAtCoords(location);
			tile->m_visibleState = HAS_SEEN;
		}

		m_seenTiles.clear();
	}

	const int NUM_SIDES = 30;
	const float DEGREES_PER_SIDE = 360.0f / (float)NUM_SIDES;

	std::string statName = "View";
	unsigned int viewIndex = m_stats->GetStatIndexByName(statName);
	int viewStat = m_stats->m_statCollection[viewIndex];

	Vector2 myPos = Vector2((float)m_currentTile->m_positionInMap.x + 0.5f, (float)m_currentTile->m_positionInMap.y + 0.5f);

	for (int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex)
	{

		float angleDegrees = DEGREES_PER_SIDE * (float)sideIndex;
		float x = CosInDegrees(angleDegrees) * (float)viewStat;
		float y = SinInDegrees(angleDegrees) * (float)viewStat;

		Vector2 endPoint = myPos + Vector2(x, y) - Vector2(1.0f, 1.0f);
		m_currentMap->RayCastViewUntilTargetForPlayer(myPos, endPoint, this, true, false);
	}
}

void Entity::LookForActorsInView()
{
	const int NUM_SIDES = 30;
	const float DEGREES_PER_SIDE = 360.0f / (float)NUM_SIDES;

	std::string statName = "View";
	unsigned int viewIndex = m_stats->GetStatIndexByName(statName);
	int viewStat = m_stats->m_statCollection[viewIndex];

	Vector2 myPos = Vector2((float)m_currentTile->m_positionInMap.x + 0.5f, (float)m_currentTile->m_positionInMap.y + 0.5f);

	for (int sideIndex = 0; sideIndex < NUM_SIDES; ++sideIndex)
	{

		float angleDegrees = DEGREES_PER_SIDE * (float)sideIndex;
		float x = CosInDegrees(angleDegrees) * (float)viewStat;
		float y = SinInDegrees(angleDegrees) * (float)viewStat;

		Vector2 endPoint = myPos + Vector2(x, y) - Vector2(1.0f, 1.0f);
		m_currentMap->RayCastForActorsInView(myPos, endPoint, this, true, false);
	}
}