#include "Game/Flee.hpp"
#include "Game/Character.hpp"
#include "Game/Map.hpp"
#include "Game/PathGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Engine/EngineConfig.hpp"

Flee::Flee()
	:m_pathGenerator(nullptr)
{

}

Flee::Flee(tinyxml2::XMLElement& element)
	:m_pathGenerator(nullptr)
	, m_turnsTaken(0)
{
	m_name = element.Name();
	m_totalTurns = ParseXmlAttribute(element, "turns", 3);
}

Flee::~Flee()
{

}

Behavior* Flee::Clone()
{
	return new Flee(*this);
}

void Flee::Act()
{
	if (m_actingCharacter->m_target == nullptr)
		return;

	if (m_currentPath.empty())
		ManagePathToTraverse();

	Tile* tile = m_currentPath.back();

	if (!m_actingCharacter->m_currentMap->IsTileOccupied(tile))
	{
		m_currentPath.pop_back();
		m_actingCharacter->SetCurrentTileOn(tile);
	}
	else
	{
		m_actingCharacter->m_currentMap->ResolveAttack(*m_actingCharacter, *tile);
	}

	m_didIAct = false;
	m_turnsTaken++;
}

void Flee::ManagePathToTraverse()
{
	if (m_pathGenerator != nullptr)
	{
		delete m_pathGenerator;
		m_pathGenerator = nullptr;
	}

	if (m_pathGenerator == nullptr)
	{
		while (!m_didIAct)
		{
			Tile furthestTile = GetFurthestRandomTileAwayFromTarget();
			IntVector2 characterPos = m_actingCharacter->GetPositionInMap();
			IntVector2 goalLocation = furthestTile.GetPositionInMap();
			m_pathGenerator = new PathGenerator(characterPos, goalLocation, m_actingCharacter->m_currentMap, m_actingCharacter, 1.01f);
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
}

Tile Flee::GetFurthestRandomTileAwayFromTarget()
{
	std::vector<Tile> tilesToFleeTo;
	tilesToFleeTo.reserve(10);
	for (int index = 0; index < 10; ++index)
	{
		Tile tile = m_actingCharacter->m_currentMap->GetRandomTileThatIsNotSolid();

		if (tile.GetPositionInMap() == m_actingCharacter->GetPositionInMap())
		{
			--index;
			continue;
		}

		tilesToFleeTo.push_back(tile);
	}

	IntVector2 actorPos = m_actingCharacter->GetPositionInMap();
	IntVector2 targetPos = m_actingCharacter->m_target->GetPositionInMap();
	IntVector2 displacementToTarget = targetPos - actorPos;
	Vector2 directionToTarget((float)displacementToTarget.x, (float)displacementToTarget.y);
	directionToTarget.Normalize();

	Tile bestTile = tilesToFleeTo[0];
	int farthestDist = 0;
	for (unsigned int index = 1; index < tilesToFleeTo.size(); ++index)
	{
		Tile currentTile = tilesToFleeTo[index];
		IntVector2 tilePos = currentTile.GetPositionInMap();
		int distance = CalculateManhattanDistance(targetPos, actorPos);

		IntVector2 displacementToTile = tilePos - actorPos;
		Vector2 directionToTile((float)displacementToTile.x, (float)displacementToTile.y);
		directionToTile.Normalize();

		float dot = DotProduct(directionToTile, directionToTarget);

		if (distance > farthestDist && dot < 0.3f)
		{
			farthestDist = distance;
			bestTile = currentTile;
		}
	}

	return bestTile;
}

int Flee::CalculateUtility()
{
	if (m_actingCharacter->m_target == nullptr)
		return 0;

	if (m_turnsTaken >= m_totalTurns)
		return 0;

	std::string name = "Current Health";
	int currentHealth = m_actingCharacter->GetStatByName(name);
	name = "Max Health";
	int maxHealth = m_actingCharacter->GetStatByName(name);
	return maxHealth - currentHealth;
}

void Flee::Render() const
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