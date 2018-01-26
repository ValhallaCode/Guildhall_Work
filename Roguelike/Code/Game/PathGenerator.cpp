#include "Game/PathGenerator.hpp"
#include "Game/Tile.hpp"
#include "Game/Map.hpp"
#include "Game/Character.hpp"
#include "Engine/Math/MathUtils.hpp"

PathGenerator::PathGenerator(IntVector2& start, IntVector2& destination, Map* map, Character* character, float hImportance /*= 1.0f*/) 
	:m_start(start)
	, m_end(destination)
	, m_map(map)
	, m_character(character)
	, m_hImportance(hImportance)
{
	static int nextPathID = 0;
	m_pathID = nextPathID++;
	OpenPathNode(m_start);
}

PathGenerator::~PathGenerator()
{

}

Path PathGenerator::GeneratePath()
{
	Path GeneratedPath;
	bool isComplete = false;
	while(!isComplete) 
	{ 
		isComplete = ContinueSteppedPath(GeneratedPath);
	}
	return GeneratedPath;
}

void PathGenerator::OpenPathNode(const IntVector2& location, PathNode* parent /*= nullptr*/)
{
	float parentTotal = parent ? parent->m_totalG : 0.0f;
	PathNode* newOpen = new PathNode();
	newOpen->m_myTile = m_map->GetTileAtCoords(location);
	newOpen->m_myTile->m_openPathID = m_pathID;
	newOpen->m_parent = parent;
	newOpen->m_hDistanceToGoal = (float)CalculateManhattanDistance(location, m_end);
	newOpen->m_localG = newOpen->m_myTile->m_localG;
	newOpen->m_totalG = parentTotal + newOpen->m_localG;
	newOpen->m_fScore = newOpen->m_totalG + (newOpen->m_hDistanceToGoal * m_hImportance);
	m_openList.push_back(newOpen);
}

bool PathGenerator::Step()
{
	if(m_openList.empty())
		return true;

	// Select and Close Best Node in Open List
	PathNode* selected = GetAndClosedBestNode();
	IntVector2 tileCoord = selected->m_myTile->m_positionInMap;

	// See if Node is Goal, if so then done
	if(tileCoord == m_end)
	{
		BuildFinalPath(selected);
		return true;
	}

	//For each Neighbor
	OpenNeighborIfValid(IntVector2(1, 0) + tileCoord, selected);
	OpenNeighborIfValid(IntVector2(-1, 0) + tileCoord, selected);
	OpenNeighborIfValid(IntVector2(0, 1) + tileCoord, selected);
	OpenNeighborIfValid(IntVector2(0, -1) + tileCoord, selected);
	return false;
}

void PathGenerator::OpenNeighborIfValid(const IntVector2& neighborPosition, PathNode* selected /*= nullptr*/)
{
	// If neighbor out of bounds
	if (m_map->AreCoordsOutOfBounds(neighborPosition))
		return;

	// If neighbor Closed ignore
	Tile* neighbor = m_map->GetTileAtCoords(neighborPosition);
	if (neighbor->m_closedPathID == m_pathID)
		return;

	// If neighbor Solid ignore
	if(neighbor->m_description->m_isSolid)
		return;

	// If neighbor open, do something special (ignore for now)
	if (neighbor->m_openPathID == m_pathID) 
	{
		//FIX Check for better path to already open node
		PathNode* neighborNode = GetPathNodeFromOpenListForTile(neighbor);
		if (neighborNode->m_totalG < selected->m_totalG)
		{
			RemoveNodeFromOpenList(selected->m_parent);
			m_closeList.push_back(selected->m_parent);
			selected->m_parent = neighborNode;
			selected->m_totalG = neighborNode->m_totalG + selected->m_localG;
			selected->m_fScore = selected->m_totalG + (selected->m_hDistanceToGoal * m_hImportance);
			return;
		}
		else
			return;
	}

	OpenPathNode(neighborPosition, selected);
}

void PathGenerator::RemoveNodeFromOpenList(PathNode* node)
{
	for (unsigned int index = 0; index < m_openList.size(); ++index)
	{
		PathNode* iterate = m_openList[index];
		if (iterate == node)
		{
			m_openList[index] = m_openList.back();
			m_openList.pop_back();
			return;
		}
	}
}

PathNode* PathGenerator::GetPathNodeFromOpenListForTile(Tile* tile)
{
	for (PathNode* node : m_openList)
	{
		if (node->m_myTile == tile)
			return node;
	}
	return nullptr;
}


PathNode* PathGenerator::GetAndClosedBestNode()
{
	float bestFScore = FLT_MAX;
	PathNode* bestNode = nullptr;
	unsigned int bestIndex = 0;

	//For loop for lowest F value
	//	If mine lower than best, then set values
	for (unsigned int index = 0; index < m_openList.size(); ++index)
	{
		PathNode* current = m_openList[index];
		if (current->m_fScore < bestFScore)
		{
			bestNode = current;
			bestFScore = current->m_fScore;
			bestIndex = index;
		}
	}

	m_closeList.push_back(m_openList[bestIndex]);

	m_openList[bestIndex] = m_openList.back();
	m_openList.pop_back();
	bestNode->m_myTile->m_closedPathID = m_pathID;
	return bestNode;
}

void PathGenerator::BuildFinalPath(PathNode* endToStart)
{
	PathNode* current = endToStart;

	while (current != nullptr)
	{
		m_final.push_back(current->m_myTile);
		current = current->m_parent;
	}
}

bool PathGenerator::ContinueSteppedPath(Path& path)
{
	bool isComplete = Step();

	if (isComplete)
	{
		path = m_final;
		return true;
	}

	return false;
}
