#pragma once
#include "Engine/Core/XMLUtils.hpp"
#include <map>
#include <string>
#include <vector>

class Map;
class Interactable;

struct ReturnExit {
	std::string m_name;
	std::string m_prevMapName;
	Interactable* m_prevFeat;
};

struct EntitiesToAdd
{
	std::string m_name;
	std::string m_tileName;
	int m_number;
};

class Adventure
{
public:
	Adventure();
	Adventure(tinyxml2::XMLElement& element);
	~Adventure();
public:
	std::string m_name;
	std::string m_playerName;
	std::string m_title;
	std::string m_startMapName;
	std::string m_startTileName;
	std::vector<std::string> m_goalList;
	std::vector<std::string> m_killList;
	std::map<std::string, std::vector<EntitiesToAdd>> m_npcsToAdd;
	std::map<std::string, std::vector<EntitiesToAdd>> m_featsToAdd;
	std::map<std::string, Map*> m_mapsList;
	int m_adventureID;
	static std::map<std::string, Adventure*> s_adventureList;
};