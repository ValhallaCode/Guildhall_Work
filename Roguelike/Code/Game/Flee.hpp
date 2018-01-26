#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Game/Behavior.hpp"
#include <vector>

class Tile;
class PathGenerator;
typedef std::vector<Tile*> Path;

class Flee : public Behavior
{
public:
	Flee();
	Flee(tinyxml2::XMLElement& element);
	virtual ~Flee();
	virtual Behavior* Clone() override;
	virtual void Act() override;
	void ManagePathToTraverse();
	Tile GetFurthestRandomTileAwayFromTarget();
	virtual int CalculateUtility() override;
	virtual void Render() const override;
public:
	int m_totalTurns;
	int m_turnsTaken;
	PathGenerator* m_pathGenerator;
	Path m_currentPath;
};