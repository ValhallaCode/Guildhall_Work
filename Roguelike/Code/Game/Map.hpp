#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include <vector>

class MapDescription;
class PathGenerator;
typedef std::vector<Tile*> Path;
class Player;

struct ToolTip
{
	std::string m_text;
	Rgba m_color;
	float m_size;
};

class Map
{
public:
	Map();
	Map(MapDescription& desc);
	~Map();
	void Update(float deltaSeconds);
	void AllEntitiesActIfTurn();
	void UpdateAllEntities(float deltaSeconds);
	void Render() const;
	void DebugRenderPath() const;
	void RenderEntities() const;
	void RenderTiles() const;
	IntVector2 GetTileCoordsForWorldPosition(Vector2& worldPosition) const;
	bool IsTileSolid(IntVector2& tileCoords);
	Tile GetTileAtCoords(const IntVector2& tileCoords) const;
	Tile* GetTileAtCoords(const IntVector2& tileCoords);
	int GetTileIndexForTileCoords(const IntVector2& tileCoords) const;
	IntVector2 GetTileCoordsForTileIndex(int tileIndex);
	bool AreCoordsOutOfBounds(const IntVector2& tileCoords);
	Tile* GetTilesNorthernNeighbor(Tile* tile);
	Tile* GetTilesSouthernNeighbor(Tile* tile);
	Tile* GetTilesWesternNeighbor(Tile* tile);
	Tile* GetTilesEasternNeighbor(Tile* tile);
	bool IsTileOccupied(Tile* tile);
	void ResolveAttack(Character& attacker, Tile& target);
	void CheckForDeathOfEntities();
	bool DidRaycastReachTarget(Vector2& start, Vector2& end, bool opaqueHits = false, bool solidHits = false);
	IntVector2 GetPositionInRaycast(Vector2& start, Vector2& end, bool opaqueHits /*= false*/, bool solidHits /*= false*/);
	Character* FindNearestCharacterOfType(Character* center, std::string& typeToCheck);
	Character* FindNearestCharacterNotOfType(Character* center, std::string& typeToCheck);
	Character* FindNearestCharacterOfFaction(Character* center, std::string& factionToCheck);
	Character* FindNearestCharacterNotOfFaction(Character* center, std::string& factionToCheck);
	Tile* FindNearestTileOfType(IntVector2& centerPos, std::string& typeToCheck);
	void StartSteppedPath(IntVector2& start, IntVector2& goal, Character* character, float hImportance = 1.0f);
	Tile GetRandomTileThatIsNotSolid();
	Tile* GetRandomTilePointerThatIsNotSolid();
	Tile* GetRandomTileByName(std::string& name);
	Tile* GetRandomEdgeTile();
	bool RayCastViewUntilTargetForPlayer(Vector2& start, Vector2& end, Entity* character, bool opaqueHits /*= false*/, bool solidHits /*= false*/);
	bool RayCastForActorsInView(Vector2& start, Vector2& end, Entity* character, bool opaqueHits /*= false*/, bool solidHits /*= false*/);
	void SetLightingOnTileBasedOnCenter(const IntVector2& center, int lightValue);
	void RenderToolTip() const;
	void PopulateToolTipVector(Tile& tile, std::vector<ToolTip>& toolTipInfo) const;
public:
	std::string m_name;
	std::vector<Tile> m_tiles;
	std::vector<Entity*> m_entities;
	IntVector2 m_dimensions;
	bool m_hasPlayerActedThisFrame;
	PathGenerator* m_pathGenerator;
	Path m_debugPath;
	Tile m_debugGoal;
	Player* m_currentPlayer;
	int m_defaultLight;
};