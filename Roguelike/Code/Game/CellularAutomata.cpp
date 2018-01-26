#include "Game/CellularAutomata.hpp"
#include "Engine/Math/MathUtils.hpp"

CellularAutomata::CellularAutomata()
{

}

CellularAutomata::CellularAutomata(tinyxml2::XMLElement& element, MapDescription* desc)
{
	m_numberOfPasses = ParseXmlAttribute(element, "passes", 3);
	m_dimensions = desc->m_dimensions;
	m_tiles = desc->m_tiles;

	for (auto tileInfo = element.FirstChildElement(); tileInfo != nullptr; tileInfo = tileInfo->NextSiblingElement())
	{
		std::string tileToCheckAgainst = tileInfo->Name();
		std::string tileToCompare = ParseXmlAttribute(*tileInfo, "compareTile", std::string("ERROR_TILE_NOT_FOUND!"));
		std::string tileToReplaceWith = ParseXmlAttribute(*tileInfo, "replaceWith", std::string("ERROR_TILE_NOT_FOUND!"));
		int minVal = ParseXmlAttribute(*tileInfo, "min", 0);
		int maxVal = ParseXmlAttribute(*tileInfo, "max", 1);
		IntVector2 compareValues(minVal, maxVal);
		float chanceToRun = ParseXmlAttribute(*tileInfo, "chance", 0.0f);
		chanceToRun = ClampWithin(chanceToRun, 1.0f, 0.0f);

		m_tileMinMaxs.insert_or_assign(tileToCheckAgainst, compareValues);
		m_tilesToCheckAgainst.insert_or_assign(tileToCheckAgainst, tileToCompare);
		m_tilesToReplace.insert_or_assign(tileToCheckAgainst, tileToReplaceWith);
		m_chancesToRun.insert_or_assign(tileToCheckAgainst, chanceToRun);
	}

	GenerateTiles(desc->m_tiles);
}

CellularAutomata::~CellularAutomata()
{

}

void CellularAutomata::GenerateTiles(std::vector<Tile>& tilesToCheck)
{
	for(unsigned int iterationCount = 0; iterationCount < m_numberOfPasses; ++iterationCount)
	{
		for (int rowIndex = -(m_dimensions.y / 2); rowIndex < (m_dimensions.y / 2); ++rowIndex)
		{
			for (int columnIndex = -(m_dimensions.x / 2); columnIndex < (m_dimensions.x / 2); ++columnIndex)
			{
				IntVector2 pos(columnIndex, rowIndex);
				int index = GetTileIndexForTileCoords(pos);

				Tile& currentTile = tilesToCheck[index];

				auto iterate = m_tilesToCheckAgainst.find(currentTile.m_description->m_name);
				if (iterate == m_tilesToCheckAgainst.end())
					continue;

				std::string tileToCompareName = iterate->first;
				float random = GetRandomFloatInRange(0.0f, 1.0f);
				float chance = m_chancesToRun.find(tileToCompareName)->second;
				if (chance < random)
					break;

				int neighborCount = GetNumberOfSurroundingTiles(pos, iterate->second, tilesToCheck);
				int min = m_tileMinMaxs.find(tileToCompareName)->second.x;
				int max = m_tileMinMaxs.find(tileToCompareName)->second.y;

				if (neighborCount >= min && neighborCount < max)
				{
					std::string newTileName = m_tilesToReplace.find(tileToCompareName)->second;
					m_tiles[index] = TileDescription::s_tileDefRegistry[newTileName];
					m_tiles[index].SetPositionInMap(pos);
				}
			}
		}

		tilesToCheck = m_tiles;
	}
}

int CellularAutomata::GetTileIndexForTileCoords(const IntVector2& tileCoords) const
{
	return ((tileCoords.y * m_dimensions.x) + tileCoords.x) + (((m_dimensions.y / 2) * m_dimensions.x) + (m_dimensions.x / 2));
}

int CellularAutomata::GetNumberOfSurroundingTiles(IntVector2& position, std::string& tileToCompare, std::vector<Tile>& tilesToCheck)
{
	int count = 0;
	for (int rowIndex = -1; rowIndex < 2; ++rowIndex)
	{
		for (int columnIndex = -1; columnIndex < 2; ++columnIndex)
		{
			if (rowIndex == 0 && columnIndex == 0)
				continue;

			IntVector2 tileCoords(columnIndex + position.x, rowIndex + position.y);
			if (AreCoordsOutOfBounds(tileCoords))
				continue;

			int index = GetTileIndexForTileCoords(tileCoords);
			Tile& currentNeighbor = tilesToCheck[index];
			if (currentNeighbor.m_description->m_name == tileToCompare)
				++count;
		}
	}
	return count;
}

bool CellularAutomata::AreCoordsOutOfBounds(const IntVector2& tileCoords)
{
	if (tileCoords.x < (-m_dimensions.x * 0.5f) || tileCoords.y < (-m_dimensions.y * 0.5f))
		return true;
	if (tileCoords.x >= (m_dimensions.x * 0.5f) || tileCoords.y >= (m_dimensions.y * 0.5f))
		return true;
	return false;
}