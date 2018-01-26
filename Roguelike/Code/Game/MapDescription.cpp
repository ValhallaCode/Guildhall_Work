#include "Game/MapDescription.hpp"
#include "Game/MapGenerator.hpp"
#include "Game/MapFromFile.hpp"
#include "Game/RoomAndCorridor.hpp"
#include "Game/CellularAutomata.hpp"
#include "Game/PerlinNoiseGenerator.hpp"

MapDescription::MapDescription()
{

}

MapDescription::MapDescription(tinyxml2::XMLElement& element)
{
	ValidateXmlElement(element, "Map", "name,dimensions,initTile");
	m_name = ParseXmlAttribute(element, "name", std::string("ERROR_NAME_NOT_FOUND!"));
	m_dimensions = ParseXmlAttribute(element, "dimensions", IntVector2(4,4));
	std::string baseTile = ParseXmlAttribute(element, "initTile", std::string("Grass"));
	std::string edgeTile = ParseXmlAttribute(element, "edgeTile", baseTile);

	GenerateInitialTileSet(baseTile, edgeTile);
	m_tiles.shrink_to_fit();

	for (auto genType = element.FirstChildElement(); genType != nullptr; genType = genType->NextSiblingElement())
	{
		std::string mapType = genType->Name();

		if (mapType == "FromFile")
		{
			MapFromFile(*genType, this);
		}
		else if (mapType == "RoomAndCorridor")
		{
			RoomAndCorridor(*genType, this);
		}
		else if (mapType == "CellularAutomata")
		{
			CellularAutomata(*genType, this);
		}
		else if (mapType == "PerlinNoise")
		{
			PerlinNoiseGenerator(*genType, this);
		}
	}
}

MapDescription::~MapDescription()
{

}

void MapDescription::GenerateInitialTileSet(std::string& baseTile, std::string& edgeTile)
{
	int numTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(numTiles);

	for (int rowIndex = -(m_dimensions.y / 2); rowIndex < (m_dimensions.y / 2); ++rowIndex)
	{
		for (int columnIndex = -(m_dimensions.x / 2); columnIndex < (m_dimensions.x / 2); ++columnIndex)
		{
			if (rowIndex == -(m_dimensions.y / 2) || rowIndex == ((m_dimensions.y / 2)-1))
			{
				Tile tile = Tile(TileDescription::s_tileDefRegistry[edgeTile]);
				tile.SetPositionInMap(columnIndex, rowIndex);
				m_tiles.push_back(tile);
				continue;
			}

			if (columnIndex == -(m_dimensions.x / 2) || columnIndex == ((m_dimensions.x / 2)-1))
			{
				Tile tile = Tile(TileDescription::s_tileDefRegistry[edgeTile]);
				tile.SetPositionInMap(columnIndex, rowIndex);
				m_tiles.push_back(tile);
				continue;
			}

			Tile tile = Tile(TileDescription::s_tileDefRegistry[baseTile]);
			tile.SetPositionInMap(columnIndex, rowIndex);
			m_tiles.push_back(tile);
		}
	}
}

std::map<std::string, MapDescription*> MapDescription::s_mapList;
