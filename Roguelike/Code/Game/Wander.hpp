#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Game/Behavior.hpp"
#include <vector>

class Tile;
class PathGenerator;
typedef std::vector<Tile*> Path;

class Wander : public Behavior
{
public:
	Wander();
	Wander(tinyxml2::XMLElement& element);
	virtual ~Wander();
	virtual Behavior* Clone() override;
	virtual void Act() override;
	void ManagePathToTraverse();
	virtual int CalculateUtility() override;
	virtual void Render() const override;
public:
	PathGenerator* m_pathGenerator;
	Path m_currentPath;
};