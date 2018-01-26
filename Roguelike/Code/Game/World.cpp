#include "Game/World.hpp"
#include "Game/Player.hpp"
#include "Game/NPC.hpp"
#include "Game/Interactable.hpp"
#include "Game/GameCommons.hpp"
#include "Game/PathGenerator.hpp"
#include "Game/MapDescription.hpp"
#include "Game/Game.hpp"
#include "Game/Adventure.hpp"

World::World(Adventure& adventure)
{
	for (auto iterate = adventure.m_mapsList.begin(); iterate != adventure.m_mapsList.end(); ++iterate)
	{
		m_mapsList.insert_or_assign(iterate->first, *iterate->second);
		Map* map = &m_mapsList.find(iterate->first)->second;
		map->m_entities.clear();
		for (unsigned int index = 0; index < iterate->second->m_entities.size(); ++index)
		{
			Interactable* entity = new Interactable(*dynamic_cast<Interactable*>(iterate->second->m_entities[index]));
			entity->m_currentMap = map;
			
			Tile* tile = entity->m_currentMap->GetTileAtCoords(entity->m_location);
			entity->SetCurrentTileOn(tile);
			map->m_entities.push_back(entity);
		}
	}


	m_currentMap = &m_mapsList.find(adventure.m_startMapName)->second;
	CreatePlayer(adventure.m_startTileName, adventure.m_playerName);
	CreateEnemy(adventure);
	CreateInteractable(adventure);
}

World::~World()
{
	//ResetMaps();
}

void World::ResetMaps()
{
	for (auto iterate = m_mapsList.begin(); iterate != m_mapsList.end(); ++iterate)
	{
		for (unsigned int tileIndex = 0; tileIndex < iterate->second.m_tiles.size(); ++tileIndex)
		{
			iterate->second.m_tiles[tileIndex].m_visibleState = HAS_NOT_SEEN;
		}

		for (unsigned int entityIndex = 0; entityIndex < iterate->second.m_entities.size(); ++entityIndex)
		{
			Interactable* feature = dynamic_cast<Interactable*>(iterate->second.m_entities[entityIndex]);
			if (feature != nullptr)
			{
				if (feature->m_isExit)
					continue;
			}
		
			delete iterate->second.m_entities[entityIndex];
			iterate->second.m_entities[entityIndex] = nullptr;
			iterate->second.m_entities.erase(iterate->second.m_entities.begin() + entityIndex);
		}
		iterate->second.m_entities.shrink_to_fit();
	}
}

void World::Render() const
{
	m_currentMap->Render();
	RenderIntroduction();
	RenderVictory();
	RenderFailure();
}

void World::RenderIntroduction() const
{
	static int alpha = 255;
	alpha -= (unsigned int)(g_theGame->m_timeConst.GAME_FRAME_TIME * 80.0f);
	if (alpha < 0)
		alpha = 0;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawTextCenteredOnPosition2D(g_theGame->m_font, Vector2((float)m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_currentPlayer->GetPositionInMap().y + 0.5f), g_theGame->m_currentAdventure->m_title, Rgba(255, 255, 255, (unsigned char)alpha), 0.1f);
	g_simpleRenderer->DisableBlend();

}

void World::RenderVictory() const
{
	if (!g_theGame->m_victory)
		return;

	static int victoryAlpha = 255;
	victoryAlpha -= (unsigned int)(g_theGame->m_timeConst.GAME_FRAME_TIME * 80.0f);
	if (victoryAlpha < 0)
		victoryAlpha = 0;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawTextCenteredOnPosition2D(g_theGame->m_font, Vector2((float)m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_currentPlayer->GetPositionInMap().y + 0.5f), "Victory!", Rgba(255, 255, 255, (unsigned char)victoryAlpha), 0.1f);
	g_simpleRenderer->DisableBlend();
}

void World::RenderFailure() const
{
	if (!g_theGame->m_failure)
		return;

	static int failAlpha = 255;
	failAlpha -= (unsigned int)(g_theGame->m_timeConst.GAME_FRAME_TIME * 80.0f);
	if (failAlpha < 0)
		failAlpha = 0;

	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->DrawTextCenteredOnPosition2D(g_theGame->m_font, Vector2((float)m_currentPlayer->GetPositionInMap().x + 0.5f, (float)m_currentPlayer->GetPositionInMap().y + 0.5f), "You Have Died!", Rgba(255, 255, 255, (unsigned char)failAlpha), 0.1f);
	g_simpleRenderer->DisableBlend();
}

void World::Update(float deltaSeconds)
{
	m_currentMap->Update(deltaSeconds);
	UpdateDebugPath();
}

void World::UpdateDebugPath()
{
	if (!g_debug)
	{
		if (m_currentMap->m_pathGenerator != nullptr) 
		{
			delete m_currentMap->m_pathGenerator;
			m_currentMap->m_pathGenerator = nullptr;

			if (!m_currentMap->m_debugPath.empty())
				m_currentMap->m_debugPath.clear();
		}

		return;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_ENTER))
	{
		IntVector2 start = m_currentPlayer->GetPositionInMap();
		IntVector2 goal = m_currentMap->m_debugGoal.GetPositionInMap();
		m_currentMap->StartSteppedPath(start, goal, dynamic_cast<Character*>(m_currentPlayer), 1.01f);
		m_currentMap->m_debugPath = m_currentMap->m_pathGenerator->GeneratePath();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEY_SPACEBAR))
	{
		IntVector2 start = m_currentPlayer->GetPositionInMap();
		Tile goalTile = m_currentMap->m_debugGoal;
		IntVector2 goal = goalTile.m_positionInMap;
		if(m_currentMap->m_pathGenerator == nullptr)
			m_currentMap->StartSteppedPath(start, goal, dynamic_cast<Character*>(m_currentPlayer), 1.01f);
		m_currentMap->m_pathGenerator->ContinueSteppedPath(m_currentMap->m_debugPath);
	}
}

void World::AddMaps()
{
	//for (auto desc : MapDescription::s_mapList)
	//{
	//	Map map = Map(*(desc.second));
	//	m_mapsList.insert_or_assign(desc.first, map);
	//}
	//
	//m_currentMap = m_mapsList.find("test")->second;
}

void World::CreatePlayer(std::string& tileName, std::string& playerName)
{
	m_currentPlayer = new Player(CharacterDescription::s_characterDefRegistry[playerName]);

	Tile* tileToSpawn = m_currentMap->GetRandomTileByName(tileName);

	m_currentPlayer->SetCurrentTileOn(tileToSpawn);
	m_currentPlayer->SetCurrentMap(m_currentMap);
	m_currentMap->m_entities.push_back(m_currentPlayer);
}

void World::CreateEnemy(Adventure& adventure)
{
	for(auto iterate = adventure.m_npcsToAdd.begin(); iterate != adventure.m_npcsToAdd.end(); ++iterate)
	{
		std::vector<EntitiesToAdd> npcList = iterate->second;
		for(unsigned int index = 0; index < npcList.size(); ++index)
		{
			EntitiesToAdd& npcInfo = npcList[index];
			for(int numAdded = 0; numAdded < npcInfo.m_number; ++numAdded)
			{
				NPC* npc = new NPC(CharacterDescription::s_characterDefRegistry[npcInfo.m_name]);
				Map& mapOn = m_mapsList.find(iterate->first)->second;

				Tile* tileToSpawn = mapOn.GetRandomTileByName(npcInfo.m_tileName);

				npc->SetCurrentTileOn(tileToSpawn);
				npc->SetCurrentMap(&mapOn);
				mapOn.m_entities.push_back(npc);
			}
		}
	}
}

void World::CreateInteractable(Adventure& adventure)
{
	for (auto iterate = adventure.m_featsToAdd.begin(); iterate != adventure.m_featsToAdd.end(); ++iterate)
	{
		std::vector<EntitiesToAdd> featList = iterate->second;
		for (unsigned int index = 0; index < featList.size(); ++index)
		{
			EntitiesToAdd& featInfo = featList[index];
			for (int numAdded = 0; numAdded < featInfo.m_number; ++numAdded)
			{
				Interactable* interactable = new Interactable(InteractableDescription::s_interactableDefRegistry[featInfo.m_name]);
				Map& mapOn = m_mapsList.find(iterate->first)->second;

				if (!interactable->m_isLight)
					interactable->m_lightValue = mapOn.m_defaultLight;

				Tile* tileToSpawn;
				if(interactable->m_isExit)
					tileToSpawn = mapOn.GetRandomTileByName(interactable->m_tileNameToPlace);
				else
					tileToSpawn = mapOn.GetRandomTileByName(featInfo.m_tileName);

				interactable->SetCurrentTileOn(tileToSpawn);
				interactable->SetCurrentMap(&mapOn);
				mapOn.m_entities.push_back(interactable);
			}
		}
	}
}

void World::MoveToNextMap(std::string& mapName)
{
	m_currentMap = &m_mapsList.find(mapName)->second;
}

Map* World::GetNextMapInList(std::string& mapName)
{
	return &m_mapsList.find(mapName)->second;
}

void World::TransferCharacterToNewMap(Character* character, std::string& newMapName)
{
	for (unsigned int index = 0; index < m_currentMap->m_entities.size(); ++index)
	{
		Player* listObj = dynamic_cast<Player*>(m_currentMap->m_entities[index]);
		if (listObj == nullptr)
			continue;

		//if (listObj != character)
		//	continue;
		

		m_currentMap->m_entities.erase(m_currentMap->m_entities.begin() + index);
		m_mapsList.find(newMapName)->second.m_entities.push_back(character);
		//newMap->m_entities.push_back(character);
		return;
	}
}