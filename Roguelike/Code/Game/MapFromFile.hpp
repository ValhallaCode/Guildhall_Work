#pragma once
#include "Game/MapGenerator.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <vector>


class MapFromFile : public MapGenerator
{
public:
	MapFromFile();
	MapFromFile(tinyxml2::XMLElement& element, MapDescription* desc);
	virtual ~MapFromFile();
	void GenerateTiles(MapDescription* desc);
	IntVector2 GetTexelCoordsForTexelIndex(int index);
	std::string GetTileNameFromLegendWithRGBA(Rgba& color);
	int GetTileIndexForTileCoords(const IntVector2& tileCoords, MapDescription* desc) const;
public:
	std::vector< std::pair<std::string, Rgba>> m_legend;
	std::string m_filePath;
	bool m_isMirrored;
	int m_xOffset;
	int m_yOffset;
	int m_rotation;
};