#pragma once
#include "Game/MapGenerator.hpp"
#include "Game/MapDescription.hpp"
#include <map>


class PerlinNoiseGenerator : public MapGenerator
{
public:
	PerlinNoiseGenerator();
	PerlinNoiseGenerator(tinyxml2::XMLElement& element, MapDescription* desc);
	virtual ~PerlinNoiseGenerator();
	void GenerateTiles(std::vector<Tile>& tilesToCheck);
public:
	std::map<std::string, float> m_chancesToChange;
	std::map<std::string, Vector2> m_tileValues;
	std::map<std::string, std::string> m_tileToReplaceWith;
	float m_scale;
	unsigned int m_numOctaves;
	float m_octavePersitence;
	float m_octaveScale;
};