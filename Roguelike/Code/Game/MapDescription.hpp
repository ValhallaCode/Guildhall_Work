#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include "Game/Tile.hpp"
#include <vector>
#include <map>

class MapDescription
{
public:
	MapDescription();
	MapDescription(tinyxml2::XMLElement& element);
	~MapDescription();
	void GenerateInitialTileSet(std::string& baseTile, std::string& edgeTile);
public:
	std::string m_name;
	IntVector2 m_dimensions;
	std::vector<Tile> m_tiles;
	static std::map<std::string, MapDescription*> s_mapList;
};