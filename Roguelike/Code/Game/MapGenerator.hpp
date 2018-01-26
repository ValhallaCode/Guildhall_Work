#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Game/MapDescription.hpp"
#include "Game/Tile.hpp"
#include <vector>
#include <string>

class MapGenerator
{
public:
	MapGenerator();
	MapGenerator(tinyxml2::XMLElement& element, MapDescription& desc);
	virtual ~MapGenerator();
public:
	std::vector<Tile> m_tiles;
	IntVector2 m_dimensions;
};