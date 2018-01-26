#pragma once
#include "Engine/Math/IntVector2.hpp"
#include <vector>

class Tile;
class Character;
class Map;

typedef std::vector<Tile*> Path;

class PathNode 
{
public:
	Tile* m_myTile;
	PathNode* m_parent;
	float m_localG;
	float m_totalG;
	float m_hDistanceToGoal;
	float m_fScore;
};


class PathGenerator
{
public:
	PathGenerator(IntVector2& start, IntVector2& destination, Map* map, Character* character, float hImportance = 1.0f);
	~PathGenerator();
	Path GeneratePath();
	bool ContinueSteppedPath(Path& path);
private:
	void OpenPathNode(const IntVector2& location, PathNode* parent = nullptr);
	bool Step();
	void OpenNeighborIfValid(const IntVector2& neighborPosition, PathNode* selected = nullptr);
	void RemoveNodeFromOpenList(PathNode* node);
	PathNode* GetPathNodeFromOpenListForTile(Tile* tile);
	PathNode* GetAndClosedBestNode();
	void BuildFinalPath(PathNode* endToStart);
public:
	IntVector2 m_start;
	IntVector2 m_end;
	std::vector<PathNode*> m_openList;
	std::vector<PathNode*> m_closeList;
private:
	Path m_final;
	Map* m_map;
	Character* m_character;
	int m_pathID = 0;
	float m_hImportance;
};