#include "Game/Interactable.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Tile.hpp"
#include "Game/World.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"

Interactable::Interactable()
	:m_interacted(false)
	, m_canBeDrawn(true)
{

}

Interactable::Interactable(InteractableDescription* interacDesc)
	:m_interacted(false)
	, m_canBeDrawn(true)
	, m_destinationMap("")
	, m_destinationFeature("")
{
	m_currentTile = nullptr;
	m_currentMap = nullptr;

	m_name = interacDesc->m_name;
	m_charToDraw = interacDesc->m_charToDraw;
	m_charColor = interacDesc->m_charColor;
	m_color = interacDesc->m_color;
	m_isSolid = interacDesc->m_isSolid;
	m_isOpaque = interacDesc->m_isOpaque; 
	m_isExit = interacDesc->m_isExit;
	m_lightValue = interacDesc->m_lightValue;
	m_tileNameToPlace = interacDesc->m_tileNameToPlace;
	m_isLight = interacDesc->m_isLIght;

	m_stats = new Stat(interacDesc->m_statRanges); 

	m_inventory = new Inventory(200);

	m_font = g_simpleRenderer->CreateOrGetBitmapFont("PixelFont");
	m_sampler = g_simpleRenderer->CreateOrGetSampler("PointToPoint", eFilterMode::FILTER_POINT, eFilterMode::FILTER_POINT);
	m_shaderProgram = g_simpleRenderer->CreateOrGetShaderProgram("Data/HLSL/nop_textured.hlsl");

	m_mesh = new Mesh();
}

Interactable::~Interactable()
{

}

void Interactable::UpdateBoxPosition(IntVector2& position)
{
	m_mesh->CreateOneSidedQuad(Vector2((float)position.x, (float)position.y), Vector2((float)position.x + 1.0f, (float)position.y + 1.0f), Vector2(0, 0), Vector2(1, 1), m_color);
	m_bounds = AABB2D(Vector2((float)position.x, (float)position.y), Vector2((float)position.x + 1.0f, (float)position.y + 1.0f));
}

void Interactable::Update(float deltaSeconds)
{
	deltaSeconds;

	if (m_interacted && m_isExit)
	{
		Character* character = dynamic_cast<Character*>(m_currentTile->m_characterOnTile);
		if (character != nullptr)
		{
			Map* map = g_theGame->m_world->GetNextMapInList(m_destinationMap);
			g_theGame->m_world->TransferCharacterToNewMap(character, m_destinationMap);


			//SetToEntryway
			Tile* tile = map->GetTileAtCoords(m_exitLocation);

			character->m_currentMap = map;
			character->SetCurrentTileOn(tile);

			g_theGame->m_world->MoveToNextMap(m_destinationMap);

			Player* player = dynamic_cast<Player*>(character);
			if (player != nullptr)
			{
				g_theGame->m_world->m_currentPlayer = player;
			}

			m_interacted = false;
		}
	}

	if (m_isLight)
	{
		m_currentMap->SetLightingOnTileBasedOnCenter(GetPositionInMap(), m_lightValue);
	}
}

void Interactable::Render() const
{
	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->SetShaderProgram(m_shaderProgram);
	g_simpleRenderer->SetTexture(g_simpleRenderer->m_whiteTexture);

	if (m_canBeDrawn && (m_currentTile->m_visibleState == CAN_SEE || m_currentTile->m_visibleState == HAS_SEEN))
	{
		Rgba glyphColor;
		if (m_currentTile->m_visibleState == HAS_SEEN)
		{
			if (m_charColor.a >= 128)
				glyphColor = Rgba(m_charColor.r, m_charColor.g, m_charColor.b, 128);
			else
				glyphColor = m_charColor;

			Rgba color; 
			if (m_color.a >= 128)
				color = Rgba(m_color.r, m_color.g, m_color.b, 128);
			else
				color = m_color;

			m_mesh->m_indices.clear();
			m_mesh->m_vertices.clear();
			m_mesh->CreateOneSidedQuad(Vector2((float)m_currentTile->m_positionInMap.x, (float)m_currentTile->m_positionInMap.y), Vector2((float)m_currentTile->m_positionInMap.x + 1.0f, (float)m_currentTile->m_positionInMap.y + 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), color);
		}
		else
		{
			glyphColor = m_charColor;
			m_mesh->m_indices.clear();
			m_mesh->m_vertices.clear();
			m_mesh->CreateOneSidedQuad(Vector2((float)m_currentTile->m_positionInMap.x, (float)m_currentTile->m_positionInMap.y), Vector2((float)m_currentTile->m_positionInMap.x + 1.0f, (float)m_currentTile->m_positionInMap.y + 1.0f), Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f), m_color);
		}


		g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
		
		g_simpleRenderer->DrawMesh(*m_mesh);
	
		g_simpleRenderer->DrawBitmapFont(m_font, m_bounds.mins + Vector2(-0.09f, -0.1f), m_charToDraw, m_bounds.CalcSize().y, glyphColor, 0.8f);
		g_simpleRenderer->DisableBlend();
	}
}
