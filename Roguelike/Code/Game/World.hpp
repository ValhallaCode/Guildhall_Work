#pragma once
#include "Game/Map.hpp"
#include <map>

class Adventure;

class World
{
public:
	World(Adventure& adventure);
	~World();
	void ResetMaps();
	void Render() const;
	void RenderIntroduction() const;
	void RenderVictory() const;
	void RenderFailure() const;
	void Update(float deltaSeconds);
	void UpdateDebugPath();
	void AddMaps();
	void CreatePlayer(std::string& tileName, std::string& playerName);
	void CreateEnemy(Adventure& adventure);
	void CreateInteractable(Adventure& adventure);
	void MoveToNextMap(std::string& mapName);
	Map* GetNextMapInList(std::string& mapName);
	void TransferCharacterToNewMap(Character* character, std::string& newMapName);
public:
	std::map<std::string, Map> m_mapsList;
	Map* m_currentMap;
	Entity* m_currentPlayer;
};