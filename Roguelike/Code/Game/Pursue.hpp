#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Game/Behavior.hpp"
#include <vector>

class Tile;
class PathGenerator;
typedef std::vector<Tile*> Path;

class Pursue : public Behavior
{
public:
	Pursue();
	Pursue(tinyxml2::XMLElement& element);
	virtual ~Pursue();
	virtual Behavior* Clone() override;
	virtual void Act() override;
	virtual void Render() const override;
	virtual int CalculateUtility() override;
public:
	PathGenerator* m_pathGenerator;
	Path m_currentPath;
};