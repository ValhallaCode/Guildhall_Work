#pragma once
#include "Game/MapGenerator.hpp"
#include "Game/MapDescription.hpp"
#include <vector>
#include <string>
#include <map>


class CellularAutomata : public MapGenerator
{
public:
	CellularAutomata();
	CellularAutomata(tinyxml2::XMLElement& element, MapDescription* desc);
	virtual ~CellularAutomata();
	void GenerateTiles(std::vector<Tile>& tilesToCheck);
	int GetTileIndexForTileCoords(const IntVector2& tileCoords) const;
	int GetNumberOfSurroundingTiles(IntVector2& position, std::string& tileToCompare, std::vector<Tile>& tilesToCheck);
	bool AreCoordsOutOfBounds(const IntVector2& tileCoords);
public:
	std::map<std::string, IntVector2> m_tileMinMaxs;
	std::map<std::string, std::string> m_tilesToCheckAgainst;
	std::map<std::string, std::string> m_tilesToReplace;
	std::map<std::string, float> m_chancesToRun;
	unsigned int m_numberOfPasses;
};