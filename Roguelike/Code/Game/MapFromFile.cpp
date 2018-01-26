#include "Game/MapFromFile.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Math/MathUtils.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"

MapFromFile::MapFromFile()
{

}

MapFromFile::MapFromFile(tinyxml2::XMLElement& element, MapDescription* desc)
{
	ValidateXmlElement(element, "FromFile", "filePath");
	m_dimensions = ParseXmlAttribute(element, "dimensions", IntVector2(0, 0));
	m_filePath = ParseXmlAttribute(element, "filePath", std::string("ERROR_MAP_FILEPATH_NOT_FOUND!"));
	m_isMirrored = ParseXmlAttribute(element, "isMirrored", false);

	IntVector2 xOffsetRange = ParseXmlAttribute(element, "xOffset", IntVector2(0, 0));
	IntVector2 yOffsetRange = ParseXmlAttribute(element, "yOffset", IntVector2(0, 0));

	m_xOffset = GetRandomIntInRange(xOffsetRange.x, xOffsetRange.y);
	m_yOffset = GetRandomIntInRange(yOffsetRange.x, yOffsetRange.y);


	IntVector2 rotation = ParseXmlAttribute(element, "turn90", IntVector2(0, 0));
	m_rotation = GetRandomIntInRange(rotation.x, rotation.y);

	m_rotation = ClampWithin(m_rotation, 3, 0);

	for (auto legendIter = element.FirstChildElement("Legend"); legendIter != nullptr; legendIter = legendIter->NextSiblingElement("Legend"))
	{
		Rgba colorForTile = ParseXmlAttribute(*legendIter, "color", Rgba(100,100,100,255));
		std::string tileName = ParseXmlAttribute(*legendIter, "tile", std::string("ERROR_TILE_NAME_NOT_FOUND!"));
		std::pair<std::string, Rgba> pair(tileName, colorForTile);
		m_legend.push_back(pair);
	}
	m_legend.shrink_to_fit();

	m_tiles = desc->m_tiles;

	GenerateTiles(desc);

	desc->m_tiles = m_tiles;

}

MapFromFile::~MapFromFile()
{

}

void MapFromFile::GenerateTiles(MapDescription* desc)
{
	int numTiles = m_dimensions.x * m_dimensions.y;

	Rgba texelColor;

	int byteComponents = 4;
	int numBytes = numTiles * byteComponents;
	unsigned char* texelBytes = stbi_load(m_filePath.c_str(), &m_dimensions.x, &m_dimensions.y, &byteComponents, 4);
	for (int byteIndex = 0; byteIndex < numBytes; byteIndex += byteComponents)
	{
		//int maxTileY = m_dimensions.y - 1;
		int texelIndex = byteIndex / byteComponents;
		IntVector2 texelCoords = GetTexelCoordsForTexelIndex(texelIndex);

		if (m_isMirrored)
			texelCoords.x *= -1;

		IntVector2 tileCoords(texelCoords.x, -texelCoords.y);


		tileCoords += IntVector2(m_xOffset, m_yOffset);

		if (m_rotation == 1)
		{
			tileCoords = IntVector2(tileCoords.y, -tileCoords.x);
		}
		else if (m_rotation == 2)
		{
			tileCoords = IntVector2(-tileCoords.x, -tileCoords.y);
		}
		else if (m_rotation == 3)
		{
			tileCoords = IntVector2(-tileCoords.y, tileCoords.x);
		}


		unsigned char red = texelBytes[byteIndex + 0];
		unsigned char green = texelBytes[byteIndex + 1];
		unsigned char blue = texelBytes[byteIndex + 2];
		unsigned char alpha = texelBytes[byteIndex + 3];

		if (alpha == 0)
			continue;

		texelColor = Rgba(red, green, blue, alpha);

		std::string tileName = GetTileNameFromLegendWithRGBA(texelColor);

		Tile tile = Tile(TileDescription::s_tileDefRegistry[tileName]);
		tile.SetPositionInMap(tileCoords.x, tileCoords.y);

		int index = GetTileIndexForTileCoords(tileCoords, desc);


		m_tiles[index] = tile;
	}
}

IntVector2 MapFromFile::GetTexelCoordsForTexelIndex(int index)
{
	int xValue = (index % m_dimensions.x) - (m_dimensions.x / 2);
	int yValue = (index / m_dimensions.x) - (m_dimensions.y / 2);
	return IntVector2(xValue, yValue);
}

std::string MapFromFile::GetTileNameFromLegendWithRGBA(Rgba& color)
{
	for (unsigned int index = 0; index < m_legend.size(); ++index)
	{
		std::pair<std::string, Rgba> couple = m_legend[index];
		if (couple.second == color)
			return couple.first;
	}

	return "FAILED_TO_FIND_TILENAME!";
}

int MapFromFile::GetTileIndexForTileCoords(const IntVector2& tileCoords, MapDescription* desc) const
{
	return ((tileCoords.y * desc->m_dimensions.x) + tileCoords.x) + (((desc->m_dimensions.y / 2) * desc->m_dimensions.x) + (desc->m_dimensions.x / 2));
}