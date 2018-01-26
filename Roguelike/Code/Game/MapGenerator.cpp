#include "Game/MapGenerator.hpp"
#include "Game/MapFromFile.hpp"
#include "Game/RoomAndCorridor.hpp"
#include "Game/CellularAutomata.hpp"
#include "Game/PerlinNoiseGenerator.hpp"
#include <string>

MapGenerator::MapGenerator()
{

}

MapGenerator::MapGenerator(tinyxml2::XMLElement& element, MapDescription& desc)
{
	for(auto genType = element.FirstChildElement(); genType != nullptr; genType = genType->NextSiblingElement())
	{
		std::string mapType = genType->Name();

		if (mapType == "FromFile")
		{
			*this = MapFromFile(*genType, &desc);
		}
		else if (mapType == "RoomAndCorridor")
		{
			m_dimensions = desc.m_dimensions;
			*this = RoomAndCorridor(*genType, &desc);
		}
		else if (mapType == "CellularAutomata")
		{
			m_dimensions = desc.m_dimensions;
			*this = CellularAutomata(*genType, &desc);
		}
		else if (mapType == "PerlinNoise")
		{
			m_dimensions = desc.m_dimensions;
			*this = PerlinNoiseGenerator(*genType, &desc);
		}
	}
}

MapGenerator::~MapGenerator()
{

}

