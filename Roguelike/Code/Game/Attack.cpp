#include "Game/Attack.hpp"
#include "Game/Character.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommons.hpp"
#include "Game/Game.hpp"
#include "Engine/EngineConfig.hpp"
#include "Game/Player.hpp"

Attack::Attack()
{

}

Attack::Attack(tinyxml2::XMLElement& element)
{
	m_name = element.Name();
}

Attack::~Attack()
{

}

Behavior* Attack::Clone()
{
	return new Attack(*this);
}

void Attack::Act()
{
	if (m_actingCharacter->m_target == nullptr)
		return;

	IntVector2 actorPos = m_actingCharacter->GetPositionInMap();
	IntVector2 targetPos = m_actingCharacter->m_target->GetPositionInMap();

	int distanceToTarget = CalculateManhattanDistance(targetPos, actorPos);
	std::string rangeName = "Range";
	int range = m_actingCharacter->GetStatByName(rangeName);

	if (distanceToTarget > range)
		return;

	Tile* targetTile = m_actingCharacter->m_currentMap->GetTileAtCoords(targetPos);

	m_actingCharacter->m_currentMap->ResolveAttack(*m_actingCharacter, *targetTile);
}

int Attack::CalculateUtility()
{
	if (m_actingCharacter->m_target == nullptr)
		return 0;

	IntVector2 actorPos = m_actingCharacter->GetPositionInMap();
	IntVector2 targetPos = m_actingCharacter->m_target->GetPositionInMap();

	int distanceToTarget = CalculateManhattanDistance(targetPos, actorPos);
	std::string rangeName = "Range";
	int range = m_actingCharacter->GetStatByName(rangeName);

	if (distanceToTarget >= range)
		return 0;

	std::string name = "Strength";
	int strength = m_actingCharacter->GetStatByName(name);
	name = "Defense";
	int defense = m_actingCharacter->GetStatByName(name);
	name = "Toughness";
	int toughness = m_actingCharacter->GetStatByName(name);

	return strength + defense + toughness;
}

void Attack::Render() const
{
	if (m_actingCharacter->m_target == nullptr || !g_debug)
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

	IntVector2 actorPos = m_actingCharacter->GetPositionInMap();
	Vector2 actorPosInWorld((float)actorPos.x + 0.5f, (float)actorPos.y + 0.5f);

	IntVector2 targetPos = m_actingCharacter->m_target->GetPositionInMap();
	Vector2 targetPosInWorld((float)targetPos.x + 0.5f, (float)targetPos.y + 0.5f);

	Vector2 arrowBufferStart(0.0f, 0.0f);
	Vector2 arrowBufferEnd(0.0f, 0.0f);
	float buffer = 0.3f;
	if (actorPosInWorld.x > targetPosInWorld.x)
	{
		arrowBufferStart.x = -buffer;
		arrowBufferEnd.x = buffer;
	}
	else if (actorPosInWorld.x < targetPosInWorld.x)
	{
		arrowBufferStart.x = buffer;
		arrowBufferEnd.x = -buffer;
	}

	if (actorPosInWorld.y > targetPosInWorld.y)
	{
		arrowBufferStart.y = -buffer;
		arrowBufferEnd.y = buffer;
	}
	else if (actorPosInWorld.y < targetPosInWorld.y)
	{
		arrowBufferStart.y = buffer;
		arrowBufferEnd.y = -buffer;
	}

	Vector2 arrowStart = actorPosInWorld + arrowBufferStart;
	Vector2 arrowEnd = targetPosInWorld + arrowBufferEnd;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawTextWithFont(g_theGame->m_font, targetPosInWorld.x - 0.13f, targetPosInWorld.y + 0.45f, "x", Rgba(255, 0, 0, 255), (fontSize + 0.1f) * pixelRatio);
	Rgba color(255, 0, 0, 255);
	g_simpleRenderer->DrawArrow2D(arrowStart, arrowEnd, color, color, 0.15f);
	g_simpleRenderer->DisableBlend();

}
