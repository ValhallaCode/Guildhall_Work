#include "Game/Adventure.hpp"
#include "Game/Map.hpp"
#include "Game/Item.hpp"
#include "Game/MapDescription.hpp"
#include "Game/ItemDescription.hpp"
#include "Game/Interactable.hpp"
#include "Engine/Math/MathUtils.hpp"

Adventure::Adventure()
{

}

Adventure::Adventure(tinyxml2::XMLElement& element)
{
	m_name = ParseXmlAttribute(element, "name", std::string("NO_NAME_GIVEN!"));
	m_playerName = ParseXmlAttribute(element, "player", std::string("NO_NAME_GIVEN!"));
	m_title = ParseXmlAttribute(element, "title", std::string("NO_TITLE_GIVEN!"));

	tinyxml2::XMLElement* startInfo = element.FirstChildElement("StartConditions");
	m_startMapName = ParseXmlAttribute(*startInfo, "startMap", std::string("NO_START_MAP_FOUND!"));
	m_startTileName = ParseXmlAttribute(*startInfo, "startTileType", std::string("NO_START_TILE_FOUND!"));

	tinyxml2::XMLElement* goalInfo = element.FirstChildElement("VictoryConditions");
	m_goalList = ParseXmlCommaDelimitedAttribute(*goalInfo, "haveItems", std::string("NO_START_TILE_FOUND!"));

	m_killList = ParseXmlCommaDelimitedAttribute(*goalInfo, "killTarget", std::string("NO_START_TILE_FOUND!"));


	std::multimap<std::string, ReturnExit> exitsToAdd; // list of previous exits to add to next maps -- MapName, FeatName
	for (auto mapInfo = element.FirstChildElement("Map"); mapInfo != nullptr; mapInfo = mapInfo->NextSiblingElement("Map"))
	{
		std::string name = ParseXmlAttribute(*mapInfo, "name", std::string("NO_NAME_GIVEN!"));
		std::string desc = ParseXmlAttribute(*mapInfo, "mapDescription", std::string("NO_DESC_GIVEN!"));
		int lightValue = ParseXmlAttribute(*mapInfo, "light", 10);
		lightValue = ClampWithin(lightValue, 10, 0);

		Map* map = new Map(*MapDescription::s_mapList.find(desc)->second);
		map->m_name = name;
		map->m_defaultLight = lightValue;

		for (unsigned int index = 0; index < map->m_tiles.size(); ++index)
		{
			map->m_tiles[index].m_lightValue = lightValue;
		}

		for (auto itemInfo = mapInfo->FirstChildElement("Item"); itemInfo != nullptr; itemInfo = itemInfo->NextSiblingElement("Item"))
		{
			std::string itemName = ParseXmlAttribute(*itemInfo, "name", std::string("NO_NAME_GIVEN!"));
			std::string tileName = ParseXmlAttribute(*itemInfo, "onTileType", std::string("NO_NAME_GIVEN!"));
			Item* item = new Item(ItemDescription::s_itemDefRegistry.find(itemName)->second);
			Tile* tile = map->GetRandomTileByName(tileName);
			tile->m_inventory.m_itemList.push_back(item);
		}

		std::vector<EntitiesToAdd> npcCollect;
		for (auto npcInfo = mapInfo->FirstChildElement("NPC"); npcInfo != nullptr; npcInfo = npcInfo->NextSiblingElement("NPC"))
		{
			EntitiesToAdd npc;
			npc.m_name = ParseXmlAttribute(*npcInfo, "name", std::string("NO_NAME_GIVEN!"));
			npc.m_tileName = ParseXmlAttribute(*npcInfo, "onTileType", std::string("NO_NAME_GIVEN!"));
			npc.m_number = ParseXmlAttribute(*npcInfo, "number", 0);
			npcCollect.push_back(npc);
		}
		if (!npcCollect.empty())
			m_npcsToAdd.insert_or_assign(name, npcCollect);

		std::vector<EntitiesToAdd> featCollect;
		for (auto featInfo = mapInfo->FirstChildElement("Interactable"); featInfo != nullptr; featInfo = featInfo->NextSiblingElement("Interactable"))
		{
			EntitiesToAdd feat;
			feat.m_name = ParseXmlAttribute(*featInfo, "name", std::string("NO_NAME_GIVEN!"));
			feat.m_tileName = ParseXmlAttribute(*featInfo, "onTileType", std::string("NO_NAME_GIVEN!"));
			feat.m_number = ParseXmlAttribute(*featInfo, "number", 0);
			featCollect.push_back(feat);
		}
		if (!featCollect.empty())
			m_featsToAdd.insert_or_assign(name, featCollect);

		for (auto exitInfo = mapInfo->FirstChildElement("Exit"); exitInfo != nullptr; exitInfo = exitInfo->NextSiblingElement("Exit"))
		{
			std::string exitName = ParseXmlAttribute(*exitInfo, "type", std::string("NO_TYPE_GIVEN!"));						//adds to this map
			std::string destName = ParseXmlAttribute(*exitInfo, "destinationMap", std::string("NO_DESTINATION_GIVEN!"));	//adds to this feat & above list
			std::string featureToAdd = ParseXmlAttribute(*exitInfo, "createDestinationFeature", std::string("NO_CREATE_FEATURE_GIVEN!")); //adds to above list for later

			Interactable* feature = new Interactable(InteractableDescription::s_interactableDefRegistry[exitName]);
			Tile* tile = map->GetRandomTileByName(feature->m_tileNameToPlace);

			if (tile == nullptr)
				tile = map->GetRandomTilePointerThatIsNotSolid();

			feature->m_destinationMap = destName;
			feature->m_destinationFeature = featureToAdd;
			feature->m_location = tile->m_positionInMap;
			//feature->SetCurrentMap(map);
			map->m_entities.push_back(feature);

			ReturnExit returnExit;
			returnExit.m_name = featureToAdd;
			returnExit.m_prevFeat = feature;
			returnExit.m_prevMapName = name;

			exitsToAdd.insert(std::multimap<std::string, ReturnExit>::value_type(destName, returnExit));

		}

		m_mapsList.insert_or_assign(name, map);
	}

	for(auto iterate = m_mapsList.begin(); iterate != m_mapsList.end(); ++iterate)
	{
		std::pair <std::multimap<std::string, ReturnExit>::iterator, std::multimap<std::string, ReturnExit>::iterator> ret;
		ret = exitsToAdd.equal_range(iterate->first);
		for (std::multimap<std::string, ReturnExit>::iterator it = ret.first; it != ret.second; ++it)
		{
			std::string featName = it->second.m_name;
			Interactable* returnFeat = new Interactable(InteractableDescription::s_interactableDefRegistry[featName]);
			IntVector2 prevFeatLocation = it->second.m_prevFeat->m_location;


			returnFeat->m_destinationMap = it->second.m_prevMapName;
			returnFeat->m_destinationFeature = it->second.m_prevFeat->m_name;
			returnFeat->m_exitLocation = prevFeatLocation;

			Tile* featTile = iterate->second->FindNearestTileOfType(prevFeatLocation, returnFeat->m_tileNameToPlace);

			it->second.m_prevFeat->m_exitLocation = featTile->m_positionInMap;
			returnFeat->m_location = featTile->m_positionInMap;
			//returnFeat->SetCurrentMap(iterate->second);
			iterate->second->m_entities.push_back(returnFeat);
		}
	}


	static int adventureID = 1;
	m_adventureID = adventureID;
	++adventureID;

}

Adventure::~Adventure()
{

}

std::map<std::string, Adventure*> Adventure::s_adventureList;
