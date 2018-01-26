#include "Game/RoomAndCorridor.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Math2D.hpp"

RoomAndCorridor::RoomAndCorridor()
{

}

RoomAndCorridor::RoomAndCorridor(tinyxml2::XMLElement& element, MapDescription* desc)
{
	m_dimensions = desc->m_dimensions;

	m_maxNumRooms = ParseXmlAttribute(element, "maxRooms", 0);
	m_roomWidthRange = ParseXmlAttribute(element, "widthRange", IntVector2(0, 0));
	m_roomHeightRange  = ParseXmlAttribute(element, "heightRange", IntVector2(0, 0));
	
	m_canOverlap = ParseXmlAttribute(element, "overlap", false);
	m_loopAttempts = ParseXmlAttribute(element, "roomTries", 100);


	tinyxml2::XMLElement* tiles = element.FirstChildElement("TileInfo");

	m_wallTile = ParseXmlAttribute(*tiles, "wallTile", std::string("stone"));
	m_floorTile = ParseXmlAttribute(*tiles, "floorTile", std::string("ground"));
	m_pathTile = ParseXmlAttribute(*tiles, "pathTile", std::string("ground"));


	m_tiles = desc->m_tiles;

	GenerateRoomsList();
	GenerateWalls();
	GenerateCorridors();
	GenerateFloors();

	desc->m_tiles = m_tiles;
}


RoomAndCorridor::~RoomAndCorridor()
{

}

IntVector2 RoomAndCorridor::GetTileCoordsForTileIndex(int tileIndex)
{
	int xValue = (tileIndex % m_dimensions.x) - (m_dimensions.x / 2);
	int yValue = (tileIndex / m_dimensions.x) - (m_dimensions.y / 2);
	return IntVector2(xValue, yValue);
}

int RoomAndCorridor::GetTileIndexForTileCoords(const IntVector2& tileCoords) const
{
	return ((tileCoords.y * m_dimensions.x) + tileCoords.x) + (((m_dimensions.y / 2) * m_dimensions.x) + (m_dimensions.x / 2));
}

void RoomAndCorridor::GenerateRoomsList()
{
	m_rooms.reserve(m_maxNumRooms);
	for (int index = 0; index < m_maxNumRooms; ++index)
	{
		int roomWidth = GetRandomIntInRange(m_roomWidthRange.x, m_roomWidthRange.y);
		int roomHeight = GetRandomIntInRange(m_roomHeightRange.x, m_roomHeightRange.y);

		bool wasRoomSet = false;
		int loopCount = 0;

		while(!wasRoomSet && loopCount < m_loopAttempts)
		{
			int randomCenterWidth = GetRandomIntInRange(-(m_dimensions.x - roomWidth) / 2, (m_dimensions.x - roomWidth - 1) / 2);
			int randomCenterHeight = GetRandomIntInRange(-(m_dimensions.y - roomHeight) / 2, (m_dimensions.y - roomHeight) / 2);

			Room possibleRoom;
			possibleRoom.width = roomWidth;
			possibleRoom.height = roomHeight;
			possibleRoom.centerTilePosition = IntVector2(randomCenterWidth, randomCenterHeight);

			if (!DoAnyRoomsOverlap(possibleRoom) && !m_canOverlap)
			{
				m_rooms.push_back(possibleRoom);
				wasRoomSet = true;
			}
			else if (m_canOverlap)
			{
				m_rooms.push_back(possibleRoom);
				wasRoomSet = true;
			}

			++loopCount;
		}
	}
}

bool RoomAndCorridor::DoAnyRoomsOverlap(Room& roomToCheck)
{
	if (m_rooms.empty())
		return false;

	for (unsigned int index = 0; index < m_rooms.size(); ++index)
	{
		Room roomToCheckAgainst = m_rooms[index];
		AABB2D roomToCheckAgainstBox(Vector2((float)roomToCheckAgainst.centerTilePosition.x, (float)roomToCheckAgainst.centerTilePosition.y), (float)(roomToCheckAgainst.width / 2), (float)(roomToCheckAgainst.height / 2));
		AABB2D roomToCheckBox(Vector2((float)roomToCheck.centerTilePosition.x, (float)roomToCheck.centerTilePosition.y), (float)(roomToCheck.width / 2), (float)(roomToCheck.height / 2));

		if (DoAABBsOverlap(roomToCheckBox, roomToCheckAgainstBox))
			return true;
	}

	return false;
}

void RoomAndCorridor::GenerateWalls()
{
	for (unsigned int index = 0; index < m_rooms.size(); ++index)
	{
		Room& currentRoom = m_rooms[index];

		int roomMinHeight = -(currentRoom.height / 2) + currentRoom.centerTilePosition.y;
		int roomMaxHeight = (currentRoom.height / 2) + currentRoom.centerTilePosition.y;
		int roomMinWidth = -(currentRoom.width / 2) + currentRoom.centerTilePosition.x;
		int roomMaxWidth = (currentRoom.width / 2) + currentRoom.centerTilePosition.x;

		for (int rowIndex = roomMinHeight; rowIndex <= roomMaxHeight; rowIndex++)
		{
			for (int columnIndex = roomMinWidth; columnIndex <= roomMaxWidth; columnIndex++)
			{
				if (rowIndex == roomMinHeight || rowIndex == (roomMaxHeight))
				{
					Tile tile = Tile(TileDescription::s_tileDefRegistry[m_wallTile]);
					IntVector2 tileCoords(columnIndex, rowIndex);
					tile.SetPositionInMap(tileCoords);
					int tileIndex = GetTileIndexForTileCoords(tileCoords);

					m_tiles[tileIndex] = tile;
					continue;
				}

				if (columnIndex == roomMinWidth || columnIndex == (roomMaxWidth))
				{
					Tile tile = Tile(TileDescription::s_tileDefRegistry[m_wallTile]);
					IntVector2 tileCoords(columnIndex, rowIndex);
					tile.SetPositionInMap(tileCoords);
					int tileIndex = GetTileIndexForTileCoords(tileCoords);

					m_tiles[tileIndex] = tile;
					continue;
				}

				Tile tile = Tile(TileDescription::s_tileDefRegistry[m_floorTile]);
				IntVector2 tileCoords(columnIndex, rowIndex);
				tile.SetPositionInMap(tileCoords);
				int tileIndex = GetTileIndexForTileCoords(tileCoords);

				m_tiles[tileIndex] = tile;
			}
		}
	}
}

void RoomAndCorridor::GenerateFloors()
{
	for (unsigned int index = 0; index < m_rooms.size(); ++index)
	{
		Room& currentRoom = m_rooms[index];

		int roomMinHeight = -(currentRoom.height / 2) + currentRoom.centerTilePosition.y;
		int roomMaxHeight = (currentRoom.height / 2) + currentRoom.centerTilePosition.y;
		int roomMinWidth = -(currentRoom.width / 2) + currentRoom.centerTilePosition.x;
		int roomMaxWidth = (currentRoom.width / 2) + currentRoom.centerTilePosition.x;

		for (int rowIndex = roomMinHeight + 1; rowIndex <= roomMaxHeight - 1; rowIndex++)
		{
			for (int columnIndex = roomMinWidth + 1; columnIndex <= roomMaxWidth - 1; columnIndex++)
			{
				Tile tile = Tile(TileDescription::s_tileDefRegistry[m_floorTile]);
				IntVector2 tileCoords(columnIndex, rowIndex);
				tile.SetPositionInMap(tileCoords);
				int tileIndex = GetTileIndexForTileCoords(tileCoords);

				m_tiles[tileIndex] = tile;
			}
		}
	}
}

Room* RoomAndCorridor::GetLeftMostRoom()
{
	Room* leftRoom = nullptr;
	for (unsigned int index = 0; index < m_rooms.size(); ++index)
	{
		if (leftRoom == nullptr)
			leftRoom = &m_rooms[index];

		Room& currentRoom = m_rooms[index];

		if (leftRoom->centerTilePosition.x > currentRoom.centerTilePosition.x)
			leftRoom = &currentRoom;
	}

	return leftRoom;
}

Room* RoomAndCorridor::GetRightMostRoom()
{
	Room* rightRoom = nullptr;
	for (unsigned int index = 0; index < m_rooms.size(); ++index)
	{
		if (rightRoom == nullptr)
			rightRoom = &m_rooms[index];

		Room& currentRoom = m_rooms[index];

		if (rightRoom->centerTilePosition.x < currentRoom.centerTilePosition.x)
			rightRoom = &currentRoom;
	}

	return rightRoom;
}

Room* RoomAndCorridor::GetNextRoomToTheRight(Room& currentRoom)
{
	Room* nextRoom = GetRightMostRoom();
	for (unsigned int index = 0; index < m_rooms.size(); ++index)
	{
		if (nextRoom == nullptr)
			nextRoom = &m_rooms[index];

		Room& indexRoom = m_rooms[index];

		if (nextRoom->centerTilePosition.x > indexRoom.centerTilePosition.x && indexRoom.centerTilePosition.x > currentRoom.centerTilePosition.x)
			nextRoom = &indexRoom;
	}

	return nextRoom;
}

void RoomAndCorridor::GenerateCorridors()
{
	Room* firstRoom = GetLeftMostRoom();
	Room* nextRoom = GetNextRoomToTheRight(*firstRoom);

	for(unsigned int index = 0; index < m_rooms.size() - 1; ++index)
	{
		for (int xDiff = firstRoom->centerTilePosition.x; xDiff <= nextRoom->centerTilePosition.x; ++xDiff)
		{
			Tile tile = Tile(TileDescription::s_tileDefRegistry[m_pathTile]);
			IntVector2 tileCoords(xDiff, firstRoom->centerTilePosition.y);
			tile.SetPositionInMap(tileCoords);
			int tileIndex = GetTileIndexForTileCoords(tileCoords);

			m_tiles[tileIndex] = tile;
		}

		for (int yDiff = firstRoom->centerTilePosition.y; yDiff <= nextRoom->centerTilePosition.y; ++yDiff)
		{
			Tile tile = Tile(TileDescription::s_tileDefRegistry[m_pathTile]);
			IntVector2 tileCoords(nextRoom->centerTilePosition.x, yDiff);
			tile.SetPositionInMap(tileCoords);
			int tileIndex = GetTileIndexForTileCoords(tileCoords);

			m_tiles[tileIndex] = tile;
		}

		for (int yDiff = firstRoom->centerTilePosition.y; yDiff >= nextRoom->centerTilePosition.y; --yDiff)
		{
			Tile tile = Tile(TileDescription::s_tileDefRegistry[m_pathTile]);
			IntVector2 tileCoords(nextRoom->centerTilePosition.x, yDiff);
			tile.SetPositionInMap(tileCoords);
			int tileIndex = GetTileIndexForTileCoords(tileCoords);

			m_tiles[tileIndex] = tile;
		}

		firstRoom = nextRoom;
		nextRoom = GetNextRoomToTheRight(*firstRoom);
	}
}