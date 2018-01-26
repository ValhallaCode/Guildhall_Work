#include "Game/Pursue.hpp"
#include "Game/Character.hpp"
#include "Game/PathGenerator.hpp"
#include "Game/Tile.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Game.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"

Pursue::Pursue()
	:m_pathGenerator(nullptr)
{

}

Pursue::Pursue(tinyxml2::XMLElement& element)
	: m_pathGenerator(nullptr)
{
	m_name = element.Name();
}

Pursue::~Pursue()
{

}

Behavior* Pursue::Clone()
{
	return new Pursue(*this);
}

void Pursue::Act()
{
	if (m_actingCharacter->m_target == nullptr)
		return;

	//If in front of goal return
	if (m_currentPath.size() == 1)
	{
		delete m_pathGenerator;
		m_pathGenerator = nullptr;
		m_currentPath.clear();
		return;
	}

	//Clear and Recalculate Path if blocked by another character
	if (!m_currentPath.empty()) 
	{
		Tile* tile = m_currentPath.back();
		if (tile->GetCharacterOnTile() != nullptr)
		{
			delete m_pathGenerator;
			m_pathGenerator = nullptr;
			m_currentPath.clear();
		}
	}

	//If no path generate one
	if (m_pathGenerator == nullptr)
	{
		while(!m_didIAct)
		{
				IntVector2 actorPos = m_actingCharacter->GetPositionInMap();
				IntVector2 targetPos = m_actingCharacter->m_target->GetPositionInMap();
				m_pathGenerator = new PathGenerator(actorPos, targetPos, m_actingCharacter->m_currentMap, m_actingCharacter, 1.01f);
				m_currentPath = m_pathGenerator->GeneratePath();
				if (!m_currentPath.empty())
				{
					m_currentPath.pop_back();
					delete m_pathGenerator;
					m_pathGenerator = nullptr;
					m_didIAct = true;
				}
		}
	}

	//Move along path
	Tile* tileToMoveTo = m_currentPath.back();
	m_currentPath.pop_back();
	if (tileToMoveTo->GetCharacterOnTile() == nullptr)
		m_actingCharacter->SetCurrentTileOn(tileToMoveTo);
	m_didIAct = false;
}

void Pursue::Render() const 
{
	if (!g_debug)
		return;

	Vector2 mouseInWindow = g_theInputSystem->GetCursorNormalizedPosition((float)DEFAULT_WINDOW_WIDTH, (float)DEFAULT_WINDOW_HEIGHT, *g_simpleRenderer);

	float orthoX = RangeMapFloat(0.0f, (float)DEFAULT_WINDOW_WIDTH, -(float)WORLD_WIDTH * 0.5f, (float)WORLD_WIDTH * 0.5f, mouseInWindow.x);
	float orthoY = RangeMapFloat(0.0f, (float)DEFAULT_WINDOW_HEIGHT, -(float)WORLD_HEIGHT * 0.5f, (float)WORLD_HEIGHT * 0.5f, -mouseInWindow.y);
	Vector2 playerPosInWorld((float)m_actingCharacter->m_currentMap->m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_actingCharacter->m_currentMap->m_currentPlayer->GetPositionInMap().y + 0.5f);
	Vector2 mouseInOrtho(orthoX + playerPosInWorld.x, -orthoY + playerPosInWorld.y);

	if (mouseInOrtho.x > m_actingCharacter->m_currentMap->m_dimensions.x / 2 || mouseInOrtho.x < -m_actingCharacter->m_currentMap->m_dimensions.x / 2 || mouseInOrtho.y > m_actingCharacter->m_currentMap->m_dimensions.y / 2 || mouseInOrtho.y < -m_actingCharacter->m_currentMap->m_dimensions.y / 2)
		return;
	IntVector2 tileCoord = m_actingCharacter->m_currentMap->GetTileCoordsForWorldPosition(mouseInOrtho);
	Tile* tileOn = m_actingCharacter->m_currentMap->GetTileAtCoords(tileCoord);

	if (tileOn->GetPositionInMap() != m_actingCharacter->GetPositionInMap())
		return;

	int windowWidth = DEFAULT_WINDOW_WIDTH;
	int orthoWidth = WORLD_WIDTH;
	float pixelRatio = (float)orthoWidth / (float)windowWidth;
	float fontSize = 0.5f;
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	for (unsigned int index = 0; index < m_currentPath.size(); ++index)
	{
		Tile* tile = m_currentPath[index];
		Tile* parent = nullptr;
		if (index + 1 < m_currentPath.size())
			parent = m_currentPath[index + 1];
		IntVector2 nodeLocation = tile->GetPositionInMap();
		IntVector2 parentLocation;
		if (parent != nullptr)
			parentLocation = parent->GetPositionInMap();
		else
			parentLocation = m_actingCharacter->GetPositionInMap();

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

int Pursue::CalculateUtility()
{
	if (m_actingCharacter->m_target == nullptr)
		return 0;

	std::string viewName = "View";
	int view = m_actingCharacter->GetStatByName(viewName);

	IntVector2 actorPos = m_actingCharacter->GetPositionInMap();
	IntVector2 targetPos = m_actingCharacter->m_target->GetPositionInMap();

	int distanceToTarget = CalculateManhattanDistance(targetPos, actorPos);

	if (distanceToTarget > view)
	{
		return 0;
	}

	return distanceToTarget;
}
