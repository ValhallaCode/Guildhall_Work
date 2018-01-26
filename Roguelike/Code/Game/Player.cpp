#include "Game/Player.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Map.hpp"

Player::Player()
{

}

Player::Player(CharacterDescription* charDesc)
{
	m_currentTile = nullptr;
	m_currentMap = nullptr;

	m_name = charDesc->m_name;
	m_charToDraw = charDesc->m_charToDraw;
	m_charColor = charDesc->m_charColor;
	m_faction = charDesc->m_faction;

	m_stats = new Stat(charDesc->m_statRanges);

	m_inventory = new Inventory(26);

	m_font = g_simpleRenderer->CreateOrGetBitmapFont("PixelFont");
	m_sampler = g_simpleRenderer->CreateOrGetSampler("PointToPoint", eFilterMode::FILTER_POINT, eFilterMode::FILTER_POINT);
	m_shaderProgram = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/nop_textured.hlsl");
}

Player::~Player()
{

}

void Player::Update(float deltaSeconds)
{
	RemoveOldDamageNumbers();
	UpdateDamageNumbers(deltaSeconds);
}

void Player::Act()
{
	if(g_theInputSystem->WasKeyJustPressed('Z'))
		m_currentMap->m_hasPlayerActedThisFrame = true;

	if (g_theInputSystem->WasKeyJustPressed(KEY_UPARROW))
	{
		Tile* tile = m_currentMap->GetTilesNorthernNeighbor(m_currentTile);
		if (tile != nullptr)
		{
			IntVector2 position = tile->GetPositionInMap();
			if (!m_currentMap->IsTileSolid(position) && !m_currentMap->IsTileOccupied(tile))
			{
				if (m_currentTile->m_interactableOnTile != nullptr)
					m_currentTile->m_interactableOnTile->m_canBeDrawn = true;
				SetCurrentTileOn(tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
			else if (!m_currentMap->IsTileSolid(position) && m_currentMap->IsTileOccupied(tile))
			{
				m_currentMap->ResolveAttack(*this, *tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_DOWNARROW))
	{
		Tile* tile = m_currentMap->GetTilesSouthernNeighbor(m_currentTile);
		if (tile != nullptr)
		{
			IntVector2 position = tile->GetPositionInMap();
			if (!m_currentMap->IsTileSolid(position) && !m_currentMap->IsTileOccupied(tile))
			{
				if (m_currentTile->m_interactableOnTile != nullptr)
					m_currentTile->m_interactableOnTile->m_canBeDrawn = true;
				SetCurrentTileOn(tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
			else if (!m_currentMap->IsTileSolid(position) && m_currentMap->IsTileOccupied(tile))
			{
				m_currentMap->ResolveAttack(*this, *tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_LEFTARROW))
	{
		Tile* tile = m_currentMap->GetTilesWesternNeighbor(m_currentTile);
		if (tile != nullptr)
		{
			IntVector2 position = tile->GetPositionInMap();
			if (!m_currentMap->IsTileSolid(position) && !m_currentMap->IsTileOccupied(tile))
			{
				if (m_currentTile->m_interactableOnTile != nullptr)
					m_currentTile->m_interactableOnTile->m_canBeDrawn = true;
				SetCurrentTileOn(tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
			else if (!m_currentMap->IsTileSolid(position) && m_currentMap->IsTileOccupied(tile))
			{
				m_currentMap->ResolveAttack(*this, *tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
		}
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_RIGHTARROW))
	{
		Tile* tile = m_currentMap->GetTilesEasternNeighbor(m_currentTile);
		if (tile != nullptr)
		{
			IntVector2 position = tile->GetPositionInMap();
			if (!m_currentMap->IsTileSolid(position) && !m_currentMap->IsTileOccupied(tile))
			{
				if (m_currentTile->m_interactableOnTile != nullptr)
					m_currentTile->m_interactableOnTile->m_canBeDrawn = true;
				SetCurrentTileOn(tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
			else if (!m_currentMap->IsTileSolid(position) && m_currentMap->IsTileOccupied(tile))
			{
				m_currentMap->ResolveAttack(*this, *tile);
				m_currentMap->m_hasPlayerActedThisFrame = true;
			}
		}
	}

	PlayerCheckForVisibleTiles();
	LookForActorsInView();

	if (!m_currentTile->m_inventory.m_itemList.empty() && m_currentMap->m_hasPlayerActedThisFrame)
	{
		for (unsigned int itemIndex = 0; itemIndex < m_currentTile->m_inventory.m_itemList.size(); ++itemIndex)
		{
			PickupItem(*m_currentTile->m_inventory.m_itemList[itemIndex], m_currentTile->m_inventory);
			--itemIndex;
		}
	}

}

void Player::Render() const
{
	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->SetShaderProgram(m_shaderProgram);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawBitmapFont(m_font, m_bounds.mins + Vector2(-0.09f, -0.1f), m_charToDraw, m_bounds.CalcSize().y, m_charColor, 0.8f);
	g_simpleRenderer->DisableBlend();

	RenderDamageNumbers();
}