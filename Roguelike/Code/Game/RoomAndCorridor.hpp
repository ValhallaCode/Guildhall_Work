#pragma once
#include "Game/MapGenerator.hpp"
#include "Engine/Math/IntVector2.hpp"
#include <vector>

struct Room
{
	int width;
	int height;
	IntVector2 centerTilePosition;
};

class RoomAndCorridor : public MapGenerator
{
public:
	RoomAndCorridor();
	RoomAndCorridor(tinyxml2::XMLElement& element, MapDescription* desc);
	virtual ~RoomAndCorridor();
	IntVector2 GetTileCoordsForTileIndex(int tileIndex);
	int GetTileIndexForTileCoords(const IntVector2& tileCoords) const;
	void GenerateRoomsList();
	bool DoAnyRoomsOverlap(Room& roomToCheck);
	void GenerateWalls();
	void GenerateFloors();
	Room* GetLeftMostRoom();
	Room* GetRightMostRoom();
	Room* GetNextRoomToTheRight(Room& currentRoom);
	void GenerateCorridors();
public:
	int m_maxNumRooms;
	int m_loopAttempts;
	IntVector2 m_roomWidthRange;
	IntVector2 m_roomHeightRange;
	std::string m_wallTile;
	std::string m_floorTile;
	std::string m_pathTile;
	std::vector<Room> m_rooms;
	bool m_canOverlap;
};