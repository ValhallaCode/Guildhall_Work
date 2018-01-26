#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Character.hpp"
#include "Game/NPC.hpp"
#include "Game/Interactable.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Game.hpp"
#include "Engine/EngineConfig.hpp"
#include "Game/MapGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/PathGenerator.hpp"
#include "Game/MapDescription.hpp"

Map::Map()
	:m_hasPlayerActedThisFrame(false)
	, m_pathGenerator(nullptr)
	, m_debugGoal(nullptr)
{
	
}

Map::Map(MapDescription& desc)
	: m_hasPlayerActedThisFrame(false)
	, m_pathGenerator(nullptr)
	, m_currentPlayer(nullptr)
{
	m_name = desc.m_name;
	m_dimensions = desc.m_dimensions;
	m_tiles = desc.m_tiles;

	m_debugGoal = GetRandomTileThatIsNotSolid();


	
}

Map::~Map()
{

}

void Map::Update(float deltaSeconds)
{

	if (m_currentPlayer != nullptr)
	{
		for (unsigned int index = 0; index < m_entities.size(); ++index)
		{
			Player* player = dynamic_cast<Player*>(m_entities[index]);
			if (player != nullptr)
			{
				m_currentPlayer->SetCurrentTileOn(player->GetCurrentTileOn());
				break;
			}
		}
	}

	if(m_currentPlayer == nullptr)
	{
		for (unsigned int index = 0; index < m_entities.size(); ++index)
		{
			Player* player = dynamic_cast<Player*>(m_entities[index]);
			if (player != nullptr)
			{
				m_currentPlayer = new Player(*player);
				break;
			}
		}
	}

	UpdateAllEntities(deltaSeconds);
	AllEntitiesActIfTurn();
	CheckForDeathOfEntities();
}

void Map::AllEntitiesActIfTurn()
{
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		Player* player = dynamic_cast<Player*>(m_entities[index]);
		if (player != nullptr && !m_hasPlayerActedThisFrame)
		{
			player->Act();
		}

		NPC* npc = dynamic_cast<NPC*>(m_entities[index]);
		if (npc != nullptr && m_hasPlayerActedThisFrame)
		{
			npc->Act();
		}
	}

	if(m_hasPlayerActedThisFrame)
		m_hasPlayerActedThisFrame = false;
}

void Map::UpdateAllEntities(float deltaSeconds)
{
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		m_entities[index]->Update(deltaSeconds);
	}
}

void Map::Render() const
{
	RenderTiles();
	RenderEntities();
	RenderToolTip();
	DebugRenderPath();
}

void Map::DebugRenderPath() const 
{
	if (!g_debug)
		return;

	IntVector2 playerPos = m_currentPlayer->GetPositionInMap();

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);
	g_simpleRenderer->DrawOneSidedQuad(Vector3((float)playerPos.x + 0.5f, (float)playerPos.y + 0.5f, 0.0f), Vector3(((float)WORLD_WIDTH * 0.5f), ((float)WORLD_HEIGHT * 0.5f), 0.0f), Rgba(0, 0, 0, 128));
	g_simpleRenderer->DisableBlend();

	if (m_pathGenerator == nullptr)
		return;

	Vector2 startWorld = Vector2((float)m_pathGenerator->m_start.x + 0.5f, (float)m_pathGenerator->m_start.y + 0.5f);
	Vector2 endWorld = Vector2((float)m_pathGenerator->m_end.x + 0.5f, (float)m_pathGenerator->m_end.y + 0.5f);
	//startWorld -= Vector2((float)playerPos.x, (float)playerPos.y);
	//endWorld -= Vector2((float)playerPos.x, (float)playerPos.y);

	int windowWidth = DEFAULT_WINDOW_WIDTH;
	int orthoWidth = WORLD_WIDTH;
	float pixelRatio = (float)orthoWidth / (float)windowWidth;
	float fontSize = 0.5f;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, startWorld.x - 0.5f, startWorld.y - 0.3f, "start", Rgba(255, 255, 255, 255), fontSize * pixelRatio);
	g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, endWorld.x - 0.5f, endWorld.y - 0.3f, "goal", Rgba(255, 255, 255, 255), fontSize * pixelRatio);

	//#TODO: Loop through generated paths and draw in debug
	for (PathNode* node : m_pathGenerator->m_openList)
	{
		IntVector2 nodeLocation = node->m_myTile->GetPositionInMap();
		IntVector2 parentLocation;
		if (node->m_parent != nullptr)
			parentLocation = node->m_parent->m_myTile->GetPositionInMap();
		else
			parentLocation = m_pathGenerator->m_start;

		Vector2 nodeWorld = Vector2((float)nodeLocation.x + 0.5f, (float)nodeLocation.y + 0.5f);
		Vector2 parentWorld = Vector2((float)parentLocation.x + 0.5f, (float)parentLocation.y + 0.5f);

		Vector2 arrowBufferStart(0.0f, 0.0f);
		Vector2 arrowBufferEnd(0.0f, 0.0f);
		float buffer = 0.3f;
		if (nodeWorld.x > parentWorld.x)
		{
			arrowBufferStart.x = -buffer;
			arrowBufferEnd.x = buffer;
		}
		else if (nodeWorld.x < parentWorld.x)
		{
			arrowBufferStart.x = buffer;
			arrowBufferEnd.x = -buffer;
		}

		if (nodeWorld.y > parentWorld.y)
		{
			arrowBufferStart.y = -buffer;
			arrowBufferEnd.y = buffer;
		}
		else if (nodeWorld.y < parentWorld.y)
		{
			arrowBufferStart.y = buffer;
			arrowBufferEnd.y = -buffer;
		}

		Vector2 arrowStart = nodeWorld + arrowBufferStart;
		Vector2 arrowEnd = parentWorld + arrowBufferEnd;
		g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, nodeWorld.x - 0.1f, nodeWorld.y + 0.4f, "o", Rgba(0, 255, 0, 255), fontSize * pixelRatio);
		Rgba color(0, 255, 0, 255);
		g_simpleRenderer->DrawArrow2D(arrowStart, arrowEnd, color, color, 0.1f);
	}

	for (PathNode* node : m_pathGenerator->m_closeList)
	{
		IntVector2 nodeLocation = node->m_myTile->GetPositionInMap();
		IntVector2 parentLocation;
		if (node->m_parent != nullptr)
			parentLocation = node->m_parent->m_myTile->GetPositionInMap();
		else
			parentLocation = m_pathGenerator->m_start;

		Vector2 nodeWorld = Vector2((float)nodeLocation.x + 0.5f, (float)nodeLocation.y + 0.5f);
		Vector2 parentWorld = Vector2((float)parentLocation.x + 0.5f, (float)parentLocation.y + 0.5f);

		Vector2 arrowBufferStart(0.0f, 0.0f);
		Vector2 arrowBufferEnd(0.0f, 0.0f);
		float buffer = 0.3f;
		if (nodeWorld.x > parentWorld.x)
		{
			arrowBufferStart.x = -buffer;
			arrowBufferEnd.x = buffer;
		}
		else if (nodeWorld.x < parentWorld.x)
		{
			arrowBufferStart.x = buffer;
			arrowBufferEnd.x = -buffer;
		}

		if (nodeWorld.y > parentWorld.y)
		{
			arrowBufferStart.y = -buffer;
			arrowBufferEnd.y = buffer;
		}
		else if (nodeWorld.y < parentWorld.y)
		{
			arrowBufferStart.y = buffer;
			arrowBufferEnd.y = -buffer;
		}

		Vector2 arrowStart = nodeWorld + arrowBufferStart;
		Vector2 arrowEnd = parentWorld + arrowBufferEnd;
		g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, nodeWorld.x - 0.1f, nodeWorld.y + 0.4f, "x", Rgba(255, 0, 0, 255), fontSize * pixelRatio);
		Rgba color(255, 0, 0, 255);
		g_simpleRenderer->DrawArrow2D(arrowStart, arrowEnd, color, color, 0.1f);
	}

	for (unsigned int index = 0; index < m_debugPath.size(); ++index)
	{
		Tile* tile = m_debugPath[index];
		Tile* parent = nullptr;
		if(index + 1 < m_debugPath.size())
			parent = m_debugPath[index + 1];
		IntVector2 nodeLocation = tile->GetPositionInMap();
		IntVector2 parentLocation;
		if (parent != nullptr)
			parentLocation = parent->GetPositionInMap();
		else
			parentLocation = m_pathGenerator->m_start;

		Vector2 nodeWorld = Vector2((float)nodeLocation.x + 0.5f, (float)nodeLocation.y + 0.5f);
		Vector2 parentWorld = Vector2((float)parentLocation.x + 0.5f, (float)parentLocation.y + 0.5f);

		Vector2 arrowBufferStart(0.0f, 0.0f);
		Vector2 arrowBufferEnd(0.0f, 0.0f);
		float buffer = 0.3f;
		if (nodeWorld.x > parentWorld.x)
		{
			arrowBufferStart.x = -buffer;
			arrowBufferEnd.x = buffer;
		}
		else if (nodeWorld.x < parentWorld.x)
		{
			arrowBufferStart.x = buffer;
			arrowBufferEnd.x = -buffer;
		}

		if (nodeWorld.y > parentWorld.y)
		{
			arrowBufferStart.y = -buffer;
			arrowBufferEnd.y = buffer;
		}
		else if (nodeWorld.y < parentWorld.y)
		{
			arrowBufferStart.y = buffer;
			arrowBufferEnd.y = -buffer;
		}

		Vector2 arrowStart = nodeWorld + arrowBufferStart;
		Vector2 arrowEnd = parentWorld + arrowBufferEnd;
		g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, nodeWorld.x - 0.13f, nodeWorld.y + 0.45f, "x", Rgba(0, 195, 255, 255), (fontSize + 0.1f) * pixelRatio);
		Rgba color(0, 195, 255, 255);
		g_simpleRenderer->DrawArrow2D(arrowStart, arrowEnd, color, color, 0.15f);
	}

	g_simpleRenderer->DisableBlend();
}

void Map::RenderEntities() const
{
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		Player* player = dynamic_cast<Player*>(m_entities[index]);
		if (player != nullptr)
		{
			player->Render();
		}

		NPC* npc = dynamic_cast<NPC*>(m_entities[index]);
		if (npc != nullptr)
		{
			npc->Render();
		}

		Interactable* interactable = dynamic_cast<Interactable*>(m_entities[index]);
		if (interactable != nullptr)
		{
			interactable->Render();
		}
	}
}

void Map::RenderTiles() const 
{
	for (unsigned int index = 0; index < m_tiles.size(); ++index)
	{
		m_tiles[index].Render();
	}
}

IntVector2 Map::GetTileCoordsForWorldPosition(Vector2& worldPosition) const
{
	if (worldPosition.x > (float)m_dimensions.x)
		worldPosition.x = (float)((m_dimensions.x / 2) - 1.0f);

	if (worldPosition.y > (float)m_dimensions.y)
		worldPosition.y = (float)((m_dimensions.y / 2) - 1.0f);

	if (worldPosition.x < (float)(-(m_dimensions.x / 2)))
		worldPosition.x = (float)((-(m_dimensions.x / 2)) - 1.0f);

	if (worldPosition.y < (float)(-(m_dimensions.y / 2)))
		worldPosition.y = (float)((-(m_dimensions.y / 2)) - 1.0f);

	int xValue = (int)floor(worldPosition.x);
	int yValue = (int)floor(worldPosition.y);
	return IntVector2(xValue, yValue);
}

bool Map::IsTileSolid(IntVector2& tileCoords)
{
	int tileIndex = GetTileIndexForTileCoords(tileCoords);
	return m_tiles[tileIndex].GetTileDescription()->m_isSolid;
}

Tile Map::GetTileAtCoords(const IntVector2& tileCoords) const
{
	int tileIndex = GetTileIndexForTileCoords(tileCoords);
	return m_tiles[tileIndex];
}

Tile* Map::GetTileAtCoords(const IntVector2& tileCoords)
{
	int tileIndex = GetTileIndexForTileCoords(tileCoords);
	return &m_tiles[tileIndex];
}

int Map::GetTileIndexForTileCoords(const IntVector2& tileCoords) const
{
	int index = ((tileCoords.y * m_dimensions.x) + tileCoords.x) + (((m_dimensions.y / 2) * m_dimensions.x) + (m_dimensions.x / 2));
	if (index >= m_tiles.size())
		index = m_tiles.size() - 1;
	return index;
}

IntVector2 Map::GetTileCoordsForTileIndex(int tileIndex)
{
	int xValue = (tileIndex % m_dimensions.x) - (m_dimensions.x / 2);
	int yValue = (tileIndex / m_dimensions.x) - (m_dimensions.x / 2);
	return IntVector2(xValue, yValue);
}

bool Map::AreCoordsOutOfBounds(const IntVector2& tileCoords)
{
	if (tileCoords.x < (-m_dimensions.x * 0.5f) || tileCoords.y < (-m_dimensions.y * 0.5f))
		return true;
	if (tileCoords.x >= (m_dimensions.x * 0.5f) || tileCoords.y >= (m_dimensions.y * 0.5f))
		return true;
	return false;
}

Tile* Map::GetTilesNorthernNeighbor(Tile* tile)
{
	if (AreCoordsOutOfBounds(tile->m_positionInMap + IntVector2(0, 1)))
		return nullptr;

	return GetTileAtCoords(tile->m_positionInMap + IntVector2(0,1));
}

Tile* Map::GetTilesSouthernNeighbor(Tile* tile)
{
	if (AreCoordsOutOfBounds(tile->m_positionInMap + IntVector2(0, -1)))
		return nullptr;

	return GetTileAtCoords(tile->m_positionInMap + IntVector2(0, -1));
}

Tile* Map::GetTilesWesternNeighbor(Tile* tile)
{
	if (AreCoordsOutOfBounds(tile->m_positionInMap + IntVector2(-1, 0)))
		return nullptr;

	return GetTileAtCoords(tile->m_positionInMap + IntVector2(-1, 0));
}

Tile* Map::GetTilesEasternNeighbor(Tile* tile)
{
	if (AreCoordsOutOfBounds(tile->m_positionInMap + IntVector2(1, 0)))
		return nullptr;

	return GetTileAtCoords(tile->m_positionInMap + IntVector2(1, 0));
}

bool Map::IsTileOccupied(Tile* tile)
{
	
	if (tile->m_interactableOnTile != nullptr)
	{
		tile->m_interactableOnTile->m_interacted = true;

		if (!tile->m_interactableOnTile->m_isSolid)
		{
			tile->m_interactableOnTile->m_canBeDrawn = false;
			return false;
		}
		else
		{
			tile->m_interactableOnTile->m_canBeDrawn = true;
			return true;
		}
	}

	if (tile->m_characterOnTile == nullptr)
		return false;

	return true;
}

void Map::RenderToolTip() const
{
	if (m_currentPlayer == nullptr)
		return;

	Vector2 mouseInWindow = g_theInputSystem->GetCursorNormalizedPosition((float)DEFAULT_WINDOW_WIDTH, (float)DEFAULT_WINDOW_HEIGHT, *g_simpleRenderer);

	float orthoX = RangeMapFloat(0.0f, (float)DEFAULT_WINDOW_WIDTH, -(float)WORLD_WIDTH * 0.5f, (float)WORLD_WIDTH * 0.5f, mouseInWindow.x);
	float orthoY = RangeMapFloat(0.0f, (float)DEFAULT_WINDOW_HEIGHT, -(float)WORLD_HEIGHT * 0.5f, (float)WORLD_HEIGHT * 0.5f, -mouseInWindow.y);
	Vector2 playerPosInWorld((float)m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_currentPlayer->GetPositionInMap().y + 0.5f);
	Vector2 mouseInOrtho(orthoX + playerPosInWorld.x, -orthoY + playerPosInWorld.y);

	if (mouseInOrtho.x > m_dimensions.x / 2 || mouseInOrtho.x < -m_dimensions.x / 2 || mouseInOrtho.y > m_dimensions.y / 2 || mouseInOrtho.y < -m_dimensions.y / 2)
		return;

	Tile tileOn = GetTileAtCoords(GetTileCoordsForWorldPosition(mouseInOrtho));

	if (tileOn.m_visibleState == HAS_NOT_SEEN)
		return;

	std::vector<ToolTip> tooltipInfo;
	PopulateToolTipVector(tileOn, tooltipInfo);

	//Pixel ratio
	int windowWidth = DEFAULT_WINDOW_WIDTH;
	int orthoWidth = WORLD_WIDTH;
	float pixelRatio = (float)orthoWidth / (float)windowWidth;
	//Calculate box size
	float maxTextWidth = 0;
	float boxHeight = 0;
	float borderThickness = 0.25f;

	//float boxOrthoX = RangeMapFloat(0.0f, (float)DEFAULT_WINDOW_WIDTH, -(float)WORLD_WIDTH * 0.5f, (float)WORLD_WIDTH * 0.5f, (float)windowWidth * 0.25f);
	//float orthoY = RangeMapFloat(0.0f, (float)DEFAULT_WINDOW_HEIGHT, -(float)WORLD_HEIGHT * 0.5f, (float)WORLD_HEIGHT * 0.5f, -mouseInWindow.y);
	//Vector2 playerPosInWorld((float)m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_currentPlayer->GetPositionInMap().y + 0.5f);
	Vector2 boxPos(playerPosInWorld.x, 0.0f + playerPosInWorld.y);

	//Vector2 boxPos;
	if (mouseInWindow.x >= (float)DEFAULT_WINDOW_WIDTH * 0.5f)
	{
		//boxPos = Vector2(-(float)orthoWidth * 0.25f, 0.0f);
		//boxPos.x -= playerPosInWorld.x;
		boxPos.x -= orthoWidth * 0.25f;
	}
	else
	{
		//boxPos = Vector2((float)orthoWidth * 0.25f, 0.0f);
		boxPos.x += orthoWidth * 0.25f;
	}

	for (size_t tooltipInfoIndex = 0; tooltipInfoIndex < tooltipInfo.size(); tooltipInfoIndex++)
	{
		float messageTextWidth = (float)g_theGame->m_font->GetTextWidth(tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_size * pixelRatio);
		if (messageTextWidth > maxTextWidth)
			maxTextWidth = messageTextWidth;
		boxHeight += g_theGame->m_font->GetTextHeight(tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_size * pixelRatio);
	}

	Vector2 tooltipHalfExtents((maxTextWidth) + (borderThickness), (boxHeight) + (borderThickness));
	//Draw box
	AABB2D box(boxPos, tooltipHalfExtents.x, tooltipHalfExtents.y);
	Vector2 pos = box.CalcCenter();
	Rgba borderColor(255, 0, 0, 255);
	Vector2 toolTipExtents(tooltipHalfExtents.x, tooltipHalfExtents.y);
	g_simpleRenderer->Draw2DDebugQuad(pos, toolTipExtents, borderColor, borderColor, *g_simpleRenderer->m_whiteTexture, borderThickness);
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawOneSidedQuad((Vector3(pos.x, pos.y, 0.0f)), Vector3(tooltipHalfExtents.x, tooltipHalfExtents.y, 0.0f), Rgba(0, 0, 0, 128));
	//Draw text
	Vector2 linePosition = Vector2(boxPos.x - tooltipHalfExtents.x + borderThickness, boxPos.y + tooltipHalfExtents.y - (borderThickness * 0.25f));
	float lineSpacer = 10.f;
	for (size_t tooltipInfoIndex = 0; tooltipInfoIndex < tooltipInfo.size(); tooltipInfoIndex++)
	{
		g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, linePosition.x, linePosition.y, tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_color, tooltipInfo[tooltipInfoIndex].m_size * pixelRatio);
		linePosition.y -= g_theGame->m_font->GetTextHeight(tooltipInfo[tooltipInfoIndex].m_text, tooltipInfo[tooltipInfoIndex].m_size * pixelRatio);
		linePosition.y -= lineSpacer * pixelRatio;
	}
	g_simpleRenderer->DisableBlend();

	Vector2 tileCenter = tileOn.m_bounds.CalcCenter();
	Vector2 tileBounds = tileOn.m_bounds.CalcSize() * Vector2(0.3f, 0.3f); //Multiplied by 0.5 - tile box thickness
	g_simpleRenderer->Draw2DDebugQuad(tileCenter, tileBounds, borderColor, borderColor, *g_simpleRenderer->m_whiteTexture, 0.1f);
}

void Map::PopulateToolTipVector(Tile& tile, std::vector<ToolTip>& toolTipInfo) const
{
	ToolTip temp;
	temp.m_text = tile.m_description->m_name;
	temp.m_color = Rgba(255, 255, 255, 255);
	temp.m_size = 0.8f;
	toolTipInfo.push_back(temp);

	temp.m_text = std::to_string(tile.m_positionInMap.x) + ", " + std::to_string(tile.m_positionInMap.y);
	temp.m_color = Rgba(255, 255, 255, 255);
	temp.m_size = 0.6f;
	toolTipInfo.push_back(temp);

	if (tile.GetCharacterOnTile() != nullptr)
	{
		temp.m_text = tile.GetCharacterOnTile()->m_name;
		temp.m_color = Rgba(252, 197, 0, 255);
		temp.m_size = 0.6f;
		toolTipInfo.push_back(temp);

		for (unsigned int index = 0; index < (unsigned int)StatType::NUM_STAT_TYPES; ++index) 
		{
			temp.m_text = tile.GetCharacterOnTile()->m_stats->GetStatNameByIndex(index) + " : " + std::to_string(tile.GetCharacterOnTile()->m_stats->m_statCollection[index]);
			temp.m_color = Rgba(218, 112, 214, 255);
			temp.m_size = 0.5f;
			toolTipInfo.push_back(temp);
		}

		if (!tile.GetCharacterOnTile()->m_inventory->m_itemList.empty())
		{
			for (unsigned int index = 0; index < tile.GetCharacterOnTile()->m_inventory->m_itemList.size(); ++index)
			{
				if (tile.GetCharacterOnTile()->IsItemEquiped(*tile.GetCharacterOnTile()->m_inventory->m_itemList[index]))
				{
					temp.m_text = tile.GetCharacterOnTile()->m_inventory->m_itemList[index]->m_name + "*";
				}
				else
				{
					temp.m_text = tile.GetCharacterOnTile()->m_inventory->m_itemList[index]->m_name;
				}

				temp.m_color = Rgba(0, 252, 252, 255);
				temp.m_size = 0.4f;
				toolTipInfo.push_back(temp);
			}
		}
	}

	if (tile.GetInteractableOnTile() != nullptr)
	{
		temp.m_text = tile.GetInteractableOnTile()->m_name;
		temp.m_color = Rgba(6, 201, 0, 255);
		temp.m_size = 0.6f;
		toolTipInfo.push_back(temp);

		for (unsigned int index = 0; index < (unsigned int)StatType::NUM_STAT_TYPES; ++index)
		{
			temp.m_text = tile.GetInteractableOnTile()->m_stats->GetStatNameByIndex(index) + " : " + std::to_string(tile.GetInteractableOnTile()->m_stats->m_statCollection[index]);
			temp.m_color = Rgba(218, 112, 214, 255);
			temp.m_size = 0.5f;
			toolTipInfo.push_back(temp);
		}

		if (!tile.GetInteractableOnTile()->m_inventory->m_itemList.empty())
		{
			for (unsigned int index = 0; index < tile.GetInteractableOnTile()->m_inventory->m_itemList.size(); ++index)
			{
				temp.m_text = tile.GetInteractableOnTile()->m_inventory->m_itemList[index]->m_name;
				temp.m_color = Rgba(0, 252, 252, 255);
				temp.m_size = 0.4f;
				toolTipInfo.push_back(temp);
			}
		}
	}

	if (!tile.m_inventory.m_itemList.empty())
	{
		for (unsigned int index = 0; index < tile.m_inventory.m_itemList.size(); ++index)
		{
			temp.m_text = tile.m_inventory.m_itemList[index]->m_name;
			temp.m_color = Rgba(0, 252, 252, 255);
			temp.m_size = 0.4f;
			toolTipInfo.push_back(temp);
		}
	}
}

void Map::ResolveAttack(Character& attacker, Tile& target)
{
	// Is anyone here
	if (target.GetCharacterOnTile() == nullptr)
		return;

	Character* targetEnemy = target.GetCharacterOnTile();

	if (targetEnemy->m_faction == attacker.m_faction)
		return;

	// Gather Necessary Stats
	int targetEvasion = targetEnemy->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::EVASION)];
	int targetToughness = targetEnemy->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::TOUGHNESS)];
	int targetDefense = targetEnemy->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::DEFENSE)];

	int attackerAccuracy = attacker.m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::ACCURACY)];
	int attackerStrength = attacker.m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::STRENGTH)];

	// Check if attacker missed (Accuracy) or target evaded (Evasion)
	int hitValue = attackerAccuracy - targetEvasion;
	if (hitValue < 0)
	{
		DamageIndicator indicator;
		indicator.m_color = Rgba(0, 255, 255, 255);
		indicator.m_damage = "whif";

		IntVector2 targetPos = targetEnemy->GetPositionInMap();
		Vector2 targetWorldPos((float)targetPos.x + 0.4f, (float)targetPos.y + 1.0f);
		indicator.m_position = targetWorldPos + Vector2(0.0f, 0.3f);

		attacker.m_damageNumbers.push_back(indicator);
		return;
	}

	// Modify Damage (reduce Strength from Toughness, Defense)
	if (targetDefense > attackerStrength)
	{
		DamageIndicator indicator;
		indicator.m_color = Rgba(0, 0, 255, 255);
		indicator.m_damage = "ding";

		IntVector2 targetPos = targetEnemy->GetPositionInMap();
		Vector2 targetWorldPos((float)targetPos.x + 0.4f, (float)targetPos.y + 1.0f);
		indicator.m_position = targetWorldPos + Vector2(0.0f, 0.3f);

		attacker.m_damageNumbers.push_back(indicator);
		return;
	}

	int damage = attackerStrength - targetToughness;

	// Check chance of Status Effect

	// Check for status resist if that is added

	// Apply Damage
	targetEnemy->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)] -= damage;


	DamageIndicator indicator;
	indicator.m_color = Rgba(255, 0, 0, 255);
	indicator.m_damage = std::to_string(damage);

	IntVector2 targetPos = targetEnemy->GetPositionInMap();
	Vector2 targetWorldPos((float)targetPos.x + 0.4f,(float)targetPos.y + 1.0f);
	indicator.m_position = targetWorldPos + Vector2(0.0f, 0.3f);

	attacker.m_damageNumbers.push_back(indicator);


	Player* player = dynamic_cast<Player*>(&attacker);
	if(player != nullptr && targetEnemy->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)] <= 0)
	{
		player->m_killList.push_back(targetEnemy->m_name);
	}

}

void Map::CheckForDeathOfEntities()
{
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		int currentEntityHealth = m_entities[index]->m_stats->m_statCollection[static_cast<std::underlying_type<StatType>::type>(StatType::HEALTH)];
		if (currentEntityHealth > 0)
			continue;

		Entity* currentEntity = m_entities[index];
		//unsigned int initialSize = currentEntity->m_inventory->m_itemList.size();
		for (unsigned int itemIndex = 0; itemIndex < currentEntity->m_inventory->m_itemList.size(); itemIndex)
		{
			currentEntity->DropItem(*currentEntity->m_inventory->m_itemList[itemIndex], currentEntity->GetCurrentTileOn()->m_inventory);
		}

		Character* character = dynamic_cast<Character*>(currentEntity);
		if(character != nullptr)
		{
			for (unsigned int seenIndex = 0; seenIndex < m_entities.size(); ++seenIndex)
			{
				Entity* entityWithList = m_entities[seenIndex];
				if(std::find(entityWithList->m_actorsSeen.begin(), entityWithList->m_actorsSeen.end(), character) != entityWithList->m_actorsSeen.end())
					entityWithList->m_actorsSeen.erase(std::find(entityWithList->m_actorsSeen.begin(), entityWithList->m_actorsSeen.end(), character));
			}
		}

		currentEntity->GetCurrentTileOn()->SetEntityOnTile(nullptr);
		m_entities.erase(std::find(m_entities.begin(), m_entities.end(), currentEntity));


		Player* player = dynamic_cast<Player*>(currentEntity);
		if(player == nullptr)
		{
			delete currentEntity;
			currentEntity = nullptr;
		}
		else
		{
			g_theGame->m_world->m_currentPlayer = nullptr;
			delete currentEntity;
			currentEntity = nullptr;
		}
	}
}

bool Map::DidRaycastReachTarget(Vector2& start, Vector2& end, bool opaqueHits /*= false*/, bool solidHits /*= false*/)
{
	IntVector2 startTileCoord = GetTileCoordsForWorldPosition(start);

	if (AreCoordsOutOfBounds(startTileCoord))
		return false;

	if (GetTileAtCoords(startTileCoord)->m_description->m_isSolid && solidHits)
		return false;

	Vector2 displacement = end - start;

	//Information for X
	float deltaX = 1.0f / std::abs(displacement.x);
	int tileStepX = (int)(displacement.x / std::abs(displacement.x));
	int offsetToLeadingEdgeX = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = (float)(startTileCoord.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing = std::abs(firstVerticalIntersectionX - start.x) * deltaX;

	//Information for Y
	float deltaY = 1.0f / std::abs(displacement.y);
	int tileStepY = (int)(displacement.y / std::abs(displacement.y));
	int offsetToLeadingEdgeY = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = (float)(startTileCoord.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing = std::abs(firstVerticalIntersectionY - start.y) * deltaY;

	IntVector2 tileLocation = startTileCoord;
	for (;;)
	{
		if (tOfNextXCrossing < tOfNextYCrossing)
		{
			if (tOfNextXCrossing > 1.0f)
				return true;

			tileLocation.x += tileStepX;

			if (AreCoordsOutOfBounds(tileLocation)) {
				return false;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits)
				return false;

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits)
				return false;

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits)
				return false;

			tOfNextXCrossing += deltaX;
		}
		else
		{
			if (tOfNextYCrossing > 1.0f)
				return true;

			tileLocation.y += tileStepY;

			if (AreCoordsOutOfBounds(tileLocation)) {
				return false;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits)
				return false;

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits)
				return false;

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits)
				return false;

			tOfNextYCrossing += deltaY;
		}
	}
}

IntVector2 Map::GetPositionInRaycast(Vector2& start, Vector2& end, bool opaqueHits /*= false*/, bool solidHits /*= false*/)
{
	IntVector2 startTileCoord = GetTileCoordsForWorldPosition(start);

	if (GetTileAtCoords(startTileCoord)->m_description->m_isSolid && solidHits)
		return startTileCoord;

	Vector2 displacement = end - start;

	//Information for X
	float deltaX = 1.0f / std::abs(displacement.x);
	int tileStepX = (int)(displacement.x / std::abs(displacement.x));
	int offsetToLeadingEdgeX = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = (float)(startTileCoord.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing = std::abs(firstVerticalIntersectionX - start.x) * deltaX;

	//Information for Y
	float deltaY = 1.0f / std::abs(displacement.y);
	int tileStepY = (int)(displacement.y / std::abs(displacement.y));
	int offsetToLeadingEdgeY = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = (float)(startTileCoord.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing = std::abs(firstVerticalIntersectionY - start.y) * deltaY;

	IntVector2 tileLocation = startTileCoord;
	IntVector2 lastTileCoord = tileLocation;
	bool isComplete = false;
	while (!isComplete)
	{
		if (tOfNextXCrossing < tOfNextYCrossing)
		{
			if (tOfNextXCrossing > 1.0f)
				isComplete = true;

			tileLocation.x += tileStepX;

			if (AreCoordsOutOfBounds(tileLocation)) {
				isComplete = true;
				break;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				isComplete = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits) {
				isComplete = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				isComplete = true;
			}

			tOfNextXCrossing += deltaX;
		}
		else
		{
			if (tOfNextYCrossing > 1.0f)
				isComplete = true;

			tileLocation.y += tileStepY;

			if (AreCoordsOutOfBounds(tileLocation)) {
				isComplete = true;
				break;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				isComplete = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits) {
				isComplete = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				isComplete = true;
			}

			tOfNextYCrossing += deltaY;
		}

		if (!isComplete)
			lastTileCoord = tileLocation;
	}

	return lastTileCoord;
}

Character* Map::FindNearestCharacterOfType(Character* center, std::string& typeToCheck)
{
	Character* nearest = nullptr;
	float bestDistance = 100.0f;
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		Character* currentCharacter = dynamic_cast<Character*>(m_entities[index]);
		if (currentCharacter == nullptr)
			continue;

		if (currentCharacter->m_name != typeToCheck)
			continue;

		if (nearest == nullptr)
		{
			nearest = currentCharacter;
			continue;
		}

		IntVector2 displacement = currentCharacter->GetPositionInMap() - center->GetPositionInMap();
		float distance = Vector2((float)displacement.x, (float)displacement.y).CalcLength();

		if (distance < bestDistance)
		{
			nearest = currentCharacter;
		}

	}
	return nearest;
}

Character* Map::FindNearestCharacterNotOfType(Character* center, std::string& typeToCheck)
{
	Character* nearest = nullptr;
	float bestDistance = 100.0f;
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		Character* currentCharacter = dynamic_cast<Character*>(m_entities[index]);
		if (currentCharacter == nullptr)
			continue;

		if (currentCharacter->m_name == typeToCheck)
			continue;

		if (nearest == nullptr)
		{
			nearest = currentCharacter;
			continue;
		}

		IntVector2 displacement = currentCharacter->GetPositionInMap() - center->GetPositionInMap();
		float distance = Vector2((float)displacement.x, (float)displacement.y).CalcLength();

		if (distance < bestDistance)
		{
			nearest = currentCharacter;
		}

	}
	return nearest;
}

Character* Map::FindNearestCharacterOfFaction(Character* center, std::string& factionToCheck)
{
	Character* nearest = nullptr;
	float bestDistance = 100.0f;
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		Character* currentCharacter = dynamic_cast<Character*>(m_entities[index]);
		if (currentCharacter == nullptr)
			continue;

		if (currentCharacter->m_faction != factionToCheck)
			continue;

		if (nearest == nullptr)
		{
			nearest = currentCharacter;
			continue;
		}

		IntVector2 displacement = currentCharacter->GetPositionInMap() - center->GetPositionInMap();
		float distance = Vector2((float)displacement.x, (float)displacement.y).CalcLength();

		if (distance < bestDistance)
		{
			nearest = currentCharacter;
		}

	}
	return nearest;
}

Character* Map::FindNearestCharacterNotOfFaction(Character* center, std::string& factionToCheck)
{
	Character* nearest = nullptr;
	float bestDistance = 100.0f;
	for (unsigned int index = 0; index < m_entities.size(); ++index)
	{
		Character* currentCharacter = dynamic_cast<Character*>(m_entities[index]);
		if (currentCharacter == nullptr)
			continue;

		if (currentCharacter->m_faction == factionToCheck)
			continue;

		if (nearest == nullptr)
		{
			nearest = currentCharacter;
			continue;
		}

		IntVector2 displacement = currentCharacter->GetPositionInMap() - center->GetPositionInMap();
		float distance = Vector2((float)displacement.x, (float)displacement.y).CalcLength();

		if (distance < bestDistance)
		{
			nearest = currentCharacter;
		}

	}
	return nearest;
}

Tile* Map::FindNearestTileOfType(IntVector2& centerPos, std::string& typeToCheck)
{
	Tile* nearest = nullptr;
	float bestDistance = 100.0f;
	for (unsigned int index = 0; index < m_tiles.size(); ++index)
	{
		Tile& currentTile = m_tiles[index];
		if (currentTile.m_description->m_name != typeToCheck)
			continue;

		if (nearest == nullptr)
		{
			nearest = &currentTile;
			continue;
		}

		int distance = CalculateManhattanDistance(centerPos, currentTile.GetPositionInMap());

		if (distance < bestDistance) {
			bestDistance = (float)distance;
			nearest = &currentTile;
		}
	}
	return nearest;
}

void Map::StartSteppedPath(IntVector2& start, IntVector2& goal, Character* character, float hImportance /*= 1.0f*/)
{
	if (m_pathGenerator != nullptr)
		delete m_pathGenerator;

	m_pathGenerator = new PathGenerator(start, goal, this, character, hImportance);
}

Tile Map::GetRandomTileThatIsNotSolid()
{
	bool isComplete = false;
	while (!isComplete)
	{
		int randomIndex = GetRandomIntInRange(0, m_tiles.size() - 1);
		Tile& currentTile = m_tiles[randomIndex];
		if (!currentTile.m_description->m_isSolid)
		{
			return currentTile;
		}
	}
	return Tile();
}

Tile* Map::GetRandomTilePointerThatIsNotSolid()
{
	bool isComplete = false;
	while (!isComplete)
	{
		int randomIndex = GetRandomIntInRange(0, m_tiles.size() - 1);
		Tile* currentTile = &m_tiles[randomIndex];
		if (!currentTile->m_description->m_isSolid)
		{
			return currentTile;
		}
	}
	return nullptr;
}

Tile* Map::GetRandomTileByName(std::string& name)
{
	auto desc = TileDescription::s_tileDefRegistry.find(name);
	if (desc == TileDescription::s_tileDefRegistry.end())
		return nullptr;

	bool isComplete = false;
	while (!isComplete)
	{
		int randomIndex = GetRandomIntInRange(0, m_tiles.size() - 1);
		Tile* currentTile = &m_tiles[randomIndex];
		if (currentTile->m_description->m_name == name)
		{
			return currentTile;
		}
	}
	return nullptr;
}

Tile* Map::GetRandomEdgeTile()
{
	bool isComplete = false;
	while (!isComplete)
	{
		int randomIndex = GetRandomIntInRange(0, m_tiles.size() - 1);
		Tile& currentTile = m_tiles[randomIndex];
		IntVector2 tileLocation = currentTile.m_positionInMap;

		if (tileLocation.y == -(m_dimensions.y / 2) || tileLocation.y == ((m_dimensions.y / 2) - 1))
		{
			return &currentTile;
		}

		if (tileLocation.x == -(m_dimensions.x / 2) || tileLocation.x == ((m_dimensions.x / 2) - 1))
		{
			return &currentTile;
		}

	}

	return nullptr;
}

bool Map::RayCastViewUntilTargetForPlayer(Vector2& start, Vector2& end, Entity* character, bool opaqueHits /*= false*/, bool solidHits /*= false*/)
{
	IntVector2 startTileCoord = GetTileCoordsForWorldPosition(start);

	if (AreCoordsOutOfBounds(startTileCoord))
		return false;

	if (GetTileAtCoords(startTileCoord)->m_description->m_isSolid && solidHits)
		return false;

	Vector2 displacement = end - start;

	//Information for X
	float deltaX = 1.0f / std::abs(displacement.x);
	int tileStepX = (int)(displacement.x / std::abs(displacement.x));
	int offsetToLeadingEdgeX = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = (float)(startTileCoord.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing = std::abs(firstVerticalIntersectionX - start.x) * deltaX;

	//Information for Y
	float deltaY = 1.0f / std::abs(displacement.y);
	int tileStepY = (int)(displacement.y / std::abs(displacement.y));
	int offsetToLeadingEdgeY = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = (float)(startTileCoord.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing = std::abs(firstVerticalIntersectionY - start.y) * deltaY;

	IntVector2 tileLocation = startTileCoord;
	IntVector2 endTileCoord = GetTileCoordsForWorldPosition(end);
	Tile* currentTile = GetTileAtCoords(tileLocation);
	currentTile->m_visibleState = CAN_SEE;
	character->m_seenTiles.push_back(tileLocation);

	bool atEnd = false;
	while(!atEnd)
	{
		if (tOfNextXCrossing < tOfNextYCrossing)
		{
			if (tOfNextXCrossing > 1.0f)
				atEnd = true;

			tileLocation.x += tileStepX;

			if (AreCoordsOutOfBounds(tileLocation)) {
				atEnd = true;
				break;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			tOfNextXCrossing += deltaX;
		}
		else
		{
			if (tOfNextYCrossing > 1.0f)
				atEnd = true;

			tileLocation.y += tileStepY;

			if (AreCoordsOutOfBounds(tileLocation)) {
				atEnd = true;
				break;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			tOfNextYCrossing += deltaY;
		}

		currentTile = GetTileAtCoords(tileLocation);

		if (currentTile->m_visibleState != CAN_SEE) {
			character->m_seenTiles.push_back(tileLocation);
			currentTile->m_visibleState = CAN_SEE;
		}
	}

	character->m_seenTiles.shrink_to_fit();
	return true;
}

bool Map::RayCastForActorsInView(Vector2& start, Vector2& end, Entity* character, bool opaqueHits /*= false*/, bool solidHits /*= false*/)
{
	IntVector2 startTileCoord = GetTileCoordsForWorldPosition(start);

	if (AreCoordsOutOfBounds(startTileCoord))
		return false;

	if (GetTileAtCoords(startTileCoord)->m_description->m_isSolid && solidHits)
		return false;

	Vector2 displacement = end - start;

	//Information for X
	float deltaX = 1.0f / std::abs(displacement.x);
	int tileStepX = (int)(displacement.x / std::abs(displacement.x));
	int offsetToLeadingEdgeX = (tileStepX + 1) / 2;
	float firstVerticalIntersectionX = (float)(startTileCoord.x + offsetToLeadingEdgeX);
	float tOfNextXCrossing = std::abs(firstVerticalIntersectionX - start.x) * deltaX;

	//Information for Y
	float deltaY = 1.0f / std::abs(displacement.y);
	int tileStepY = (int)(displacement.y / std::abs(displacement.y));
	int offsetToLeadingEdgeY = (tileStepY + 1) / 2;
	float firstVerticalIntersectionY = (float)(startTileCoord.y + offsetToLeadingEdgeY);
	float tOfNextYCrossing = std::abs(firstVerticalIntersectionY - start.y) * deltaY;

	IntVector2 tileLocation = startTileCoord;
	IntVector2 endTileCoord = GetTileCoordsForWorldPosition(end);
	Tile* currentTile = GetTileAtCoords(tileLocation);

	bool atEnd = false;
	while (!atEnd)
	{
		if (tOfNextXCrossing < tOfNextYCrossing)
		{
			if (tOfNextXCrossing > 1.0f)
				atEnd = true;

			tileLocation.x += tileStepX;

			if (AreCoordsOutOfBounds(tileLocation)) {
				atEnd = true;
				break;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			tOfNextXCrossing += deltaX;
		}
		else
		{
			if (tOfNextYCrossing > 1.0f)
				atEnd = true;

			tileLocation.y += tileStepY;

			if (AreCoordsOutOfBounds(tileLocation)) {
				atEnd = true;
				break;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits && GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isOpaque && opaqueHits) {
				atEnd = true;
			}

			if (GetTileAtCoords(tileLocation)->m_description->m_isSolid && solidHits) {
				atEnd = true;
			}

			tOfNextYCrossing += deltaY;
		}

		currentTile = GetTileAtCoords(tileLocation);

		

		if (currentTile->m_characterOnTile != nullptr && character != currentTile->m_characterOnTile) {
			bool canAdd = true;
			for (unsigned int index = 0; index < character->m_actorsSeen.size(); ++index)
			{
				if (character->m_actorsSeen[index] == currentTile->m_characterOnTile) {
					canAdd = false;
					break;
				}
			}
			if(canAdd)
				character->m_actorsSeen.push_back(currentTile->m_characterOnTile);
		}
	}

	character->m_actorsSeen.shrink_to_fit();
	return true;
}

void Map::SetLightingOnTileBasedOnCenter(const IntVector2& center, int lightValue)
{
	if (AreCoordsOutOfBounds(center))
		return;

	Tile* tile = GetTileAtCoords(center);
	if (tile->m_lightValue >= lightValue || tile->m_description->m_isOpaque)
		return;

	tile->m_lightValue = lightValue;

	SetLightingOnTileBasedOnCenter(center + IntVector2(1, 0), lightValue - 1);
	SetLightingOnTileBasedOnCenter(center + IntVector2(-1, 0), lightValue - 1);
	SetLightingOnTileBasedOnCenter(center + IntVector2(0, 1), lightValue - 1);
	SetLightingOnTileBasedOnCenter(center + IntVector2(0, -1), lightValue - 1);
}