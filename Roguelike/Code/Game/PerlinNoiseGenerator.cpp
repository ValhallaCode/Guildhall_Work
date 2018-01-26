#include "Game/PerlinNoiseGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Noise.hpp"

PerlinNoiseGenerator::PerlinNoiseGenerator()
{

}

PerlinNoiseGenerator::PerlinNoiseGenerator(tinyxml2::XMLElement& element, MapDescription* desc)
{
	m_dimensions = desc->m_dimensions;
	m_scale = ParseXmlAttribute(element, "scale", 1.0f);
	m_numOctaves = ParseXmlAttribute(element, "numOctaves", 1);
	m_octavePersitence = ParseXmlAttribute(element, "persistence", 0.5f);
	m_octaveScale = ParseXmlAttribute(element, "octaveScale", 2.0f);

	for (auto tileInfo = element.FirstChildElement(); tileInfo != nullptr; tileInfo = tileInfo->NextSiblingElement())
	{
		std::string tileToCheckAgainst = tileInfo->Name();
		std::string tileToReplaceWith = ParseXmlAttribute(*tileInfo, "replaceWith", std::string("ERROR_TILE_NOT_FOUND!"));
		Vector2 perlinRange = ParseXmlAttribute(*tileInfo, "range", Vector2(0.0f, 0.0f));
		perlinRange.x = ClampWithin(perlinRange.x, 1.0f, 0.0f);
		perlinRange.y = ClampWithin(perlinRange.y, 1.0f, 0.0f);
		float chanceToRun = ParseXmlAttribute(*tileInfo, "chance", 0.0f);
		chanceToRun = ClampWithin(chanceToRun, 1.0f, 0.0f);

		m_chancesToChange.insert_or_assign(tileToCheckAgainst, chanceToRun);
		m_tileToReplaceWith.insert_or_assign(tileToCheckAgainst, tileToReplaceWith);
		m_tileValues.insert_or_assign(tileToCheckAgainst, perlinRange);
	}


	GenerateTiles(desc->m_tiles);

}

PerlinNoiseGenerator::~PerlinNoiseGenerator()
{

}

void PerlinNoiseGenerator::GenerateTiles(std::vector<Tile>& tilesToCheck)
{
	for (unsigned int index = 0; index < tilesToCheck.size(); ++index)
	{
		Tile& currentTile = tilesToCheck[index];
		
		auto iterate = m_tileValues.find(currentTile.m_description->m_name);
		if (iterate == m_tileValues.end())
			continue;

		std::string tileName = iterate->first;
		float chanceToRun = m_chancesToChange.find(tileName)->second;
		float randomChance = GetRandomFloatInRange(0.0f, 1.0f);
		if (chanceToRun < randomChance)
			continue;


		float perlinValue = Compute2dPerlinNoise((float)currentTile.m_positionInMap.x, (float)currentTile.m_positionInMap.y, m_scale, m_numOctaves, m_octavePersitence, m_octaveScale);
		float minPerlin = iterate->second.x;
		float maxPerlin = iterate->second.y;

		if (perlinValue > minPerlin && perlinValue < maxPerlin)
		{
			std::string tileToReplaceWith = m_tileToReplaceWith.find(tileName)->second;
			tilesToCheck[index] = TileDescription::s_tileDefRegistry[tileToReplaceWith];
		}
	}
}
