#include "Game/World.hpp"
#include "Game/Chunk.hpp"
#include "Engine/Math/AABB3D.hpp"
#include "Game/Game.hpp"
#include "Game/BlockDefinition.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Input/FileUtilities.hpp"
#include "Game/Player.hpp"
#include"Game/HookShot.hpp"
#include "Engine/Audio/AudioSystem.hpp"

World::World()
{
}

World::~World()
{
	while (m_activeChunks.begin() != m_activeChunks.end())
	{
		std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.begin();
		ChunkCoords currentActiveChunk = iterate->first;
		RemoveChunkAtCoords(currentActiveChunk);
	}
	
}

void World::Update(float deltaSeconds)
{
	
	
	ChunkManagement();
	PlayerDigOrPlaceBlock();
	ApplyCollisionPhysicsBetweenPlayerAndBlocks();
	//UpdateLighting();
	UpdateChunks();
}

void World::ChunkManagement()
{
	int numberOfActiveChunks = GetNumberOfActiveChunks();
	Vector3 playerPosition = g_theGame->m_player->m_position;

	bool alreadyDidSomethingHeavy = false;
	if (numberOfActiveChunks >= MAX_NUM_CHUNKS)
	{
		alreadyDidSomethingHeavy = DeactivateFarthestChunk(playerPosition);
	}

	if (!alreadyDidSomethingHeavy)
	{
		alreadyDidSomethingHeavy = ActivateNearestMissingChunk(playerPosition);
	}

	if (!alreadyDidSomethingHeavy)
	{
		if(numberOfActiveChunks > DESIRED_NUM_CHUNKS )
			alreadyDidSomethingHeavy = DeactivateFarthestChunk(playerPosition);
	}
}

void World::UpdateChunks()
{
	for (std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.begin(); iterate != m_activeChunks.end(); ++iterate)
	{
		Chunk* chunk = iterate->second;
		if(chunk != nullptr)
		{
			if (chunk->m_isVertexArrayDirty)
				chunk->Update();
		}
	}
}

void World::SetAllChunksAsDirty()
{
	for (std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.begin(); iterate != m_activeChunks.end(); ++iterate)
	{
		Chunk* chunk = iterate->second;
		if (chunk != nullptr)
		{
			chunk->m_isVertexArrayDirty = true;
		}
	}
}

void World::Render() const
{
	RenderActiveChunks();
}

void World::RenderActiveChunks() const
{

	if(!g_IsDebugModeOn)
	{
		Texture* blockAtlas = g_myRenderer->CreateOrGetTexture("Data/Images/SimpleMinerAtlas.png");
		g_myRenderer->BindTexture(blockAtlas);
	}
	else
	{
		Texture* debugBlockAtlas = g_myRenderer->CreateOrGetTexture("Data/Images/DebugBlockAtlas.png");
		g_myRenderer->BindTexture(debugBlockAtlas);
	}

	for (std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.begin(); iterate != m_activeChunks.end(); ++iterate)
	{
		Chunk* chunk = iterate->second;
		if (chunk != nullptr && IsChunkInFrustum(chunk))
			chunk->Render();
		else
			DebuggerPrintf("Chunk not rendered");
	}
}

bool World::IsChunkInFrustum(const Chunk* chunk) const
{
	const float VIEW_CONSTRICTION = 0.0f;
	Vector3 cameraFWD = g_theGame->m_camera.GetForwardXYZ();
	Vector3 cameraPos = g_theGame->m_camera.m_position;
	
	Vector3 bottomMinXMinY = chunk->m_worldBounds.mins;
	Vector3 directionToBottomMinXMinY = cameraPos - bottomMinXMinY;
	directionToBottomMinXMinY.Normalize();
	float dotProductToBottomMinXMinY = DotProduct(cameraFWD, directionToBottomMinXMinY);

	Vector3 bottomMinXMaxY(chunk->m_worldBounds.mins.x, chunk->m_worldBounds.maxs.y, chunk->m_worldBounds.mins.z);
	Vector3 directionToBottomMinXMaxY = cameraPos - bottomMinXMaxY;
	directionToBottomMinXMaxY.Normalize();
	float dotProductToBottomMinXMaxY = DotProduct(cameraFWD, directionToBottomMinXMaxY);

	Vector3 bottomMaxXMaxY(chunk->m_worldBounds.maxs.x, chunk->m_worldBounds.maxs.y, chunk->m_worldBounds.mins.z);
	Vector3 directionToBottomMaxXMaxY = cameraPos - bottomMaxXMaxY;
	directionToBottomMaxXMaxY.Normalize();
	float dotProductToBottomMaxXMaxY = DotProduct(cameraFWD, directionToBottomMaxXMaxY);

	Vector3 bottomMaxXMinY(chunk->m_worldBounds.maxs.x, chunk->m_worldBounds.mins.y, chunk->m_worldBounds.mins.z);
	Vector3 directionToBottomMaxXMinY = cameraPos - bottomMaxXMinY;
	directionToBottomMaxXMinY.Normalize();
	float dotProductToBottomMaxXMinY = DotProduct(cameraFWD, directionToBottomMaxXMinY);

	Vector3 topMinXMinY(chunk->m_worldBounds.mins.x, chunk->m_worldBounds.mins.y, chunk->m_worldBounds.maxs.z);
	Vector3 directionToTopMinXMinY = cameraPos - topMinXMinY;
	directionToTopMinXMinY.Normalize();
	float dotProductToTopMinXMinY = DotProduct(cameraFWD, directionToTopMinXMinY);

	Vector3 topMinXMaxY(chunk->m_worldBounds.mins.x, chunk->m_worldBounds.maxs.y, chunk->m_worldBounds.maxs.z);
	Vector3 directionToTopMinXMaxY = cameraPos - topMinXMaxY;
	directionToTopMinXMaxY.Normalize();
	float dotProductToTopMinXMaxY = DotProduct(cameraFWD, directionToTopMinXMaxY);

	Vector3 topMaxXMaxY = chunk->m_worldBounds.maxs;
	Vector3 directionToTopMaxXMaxY = cameraPos - topMaxXMaxY;
	directionToTopMaxXMaxY.Normalize();
	float dotProductToTopMaxXMaxY = DotProduct(cameraFWD, directionToTopMaxXMaxY);

	Vector3 topMaxXMinY(chunk->m_worldBounds.maxs.x, chunk->m_worldBounds.mins.y, chunk->m_worldBounds.maxs.z);
	Vector3 directionToTopMaxXMinY = cameraPos - topMaxXMinY;
	directionToTopMaxXMinY.Normalize();
	float dotProductToTopMaxXMinY = DotProduct(cameraFWD, directionToTopMaxXMinY);

	
	if (dotProductToBottomMinXMinY >= VIEW_CONSTRICTION || dotProductToBottomMinXMinY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToBottomMinXMaxY >= VIEW_CONSTRICTION || dotProductToBottomMinXMaxY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToBottomMaxXMaxY >= VIEW_CONSTRICTION || dotProductToBottomMaxXMaxY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToBottomMaxXMinY >= VIEW_CONSTRICTION || dotProductToBottomMaxXMinY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToTopMinXMinY >= VIEW_CONSTRICTION || dotProductToTopMinXMinY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToTopMinXMaxY >= VIEW_CONSTRICTION || dotProductToTopMinXMaxY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToTopMaxXMaxY >= VIEW_CONSTRICTION || dotProductToTopMaxXMaxY <= -VIEW_CONSTRICTION)
		return true;
	else if (dotProductToTopMaxXMinY >= VIEW_CONSTRICTION || dotProductToTopMaxXMinY <= -VIEW_CONSTRICTION)
		return true;
	else
		return false;
}

Chunk* World::CreateChunk( const ChunkCoords& newPosition)
{
	Chunk* newChunk = new Chunk( newPosition );
	newChunk->GenerateChunk();
	newChunk->m_northNeighbor = GetChunkAtCoords(newPosition + ChunkCoords(0,1));
	newChunk->m_eastNeighbor = GetChunkAtCoords(newPosition + ChunkCoords(1,0));
	newChunk->m_southNeighbor = GetChunkAtCoords(newPosition + ChunkCoords(0,-1));
	newChunk->m_westNeighbor = GetChunkAtCoords(newPosition + ChunkCoords(-1,0));
	
	if(newChunk->m_westNeighbor != nullptr)
		newChunk->m_westNeighbor->m_eastNeighbor = newChunk;
	
	if (newChunk->m_eastNeighbor != nullptr)
		newChunk->m_eastNeighbor->m_westNeighbor = newChunk;
	
	if (newChunk->m_northNeighbor != nullptr)
		newChunk->m_northNeighbor->m_southNeighbor = newChunk;
	
	if (newChunk->m_southNeighbor != nullptr)
		newChunk->m_southNeighbor->m_northNeighbor = newChunk;
	
	newChunk->DirtyNeighbors();
	return newChunk;
}

bool World::ActivateNearestMissingChunk( const Vector3& playerPosition) 
{
	bool activatingAChunk = false;
	Vector2 playerXY(playerPosition.x, playerPosition.y);
	ChunkCoords playerChunkCoords = ConvertWorldPositionToChunkPosition(playerPosition);
	int chunkRange = (int)ceil(CHUNK_MAX_RANGE / (float)CHUNK_WIDTH_X);
	ChunkCoords neighborhoodMinCoords = playerChunkCoords - ChunkCoords(chunkRange, chunkRange);
	ChunkCoords neighborhoodMaxCoords = playerChunkCoords + ChunkCoords(chunkRange, chunkRange);
	float winnerDistSquared = CHUNK_MAX_RANGE * CHUNK_MAX_RANGE;
	ChunkCoords winner(0, 0);

	for (int chunkY = neighborhoodMinCoords.y; chunkY < neighborhoodMaxCoords.y; ++chunkY)
	{
		for (int chunkX = neighborhoodMinCoords.x; chunkX < neighborhoodMaxCoords.x; ++chunkX)
		{
			ChunkCoords chunkCoords(chunkX, chunkY);
			Vector3 chunkCenter = GetWorldCenterOfChunkCoords(chunkCoords);
			Vector2 chunkXY(chunkCenter.x, chunkCenter.y);
			Vector2 displacementToChunk = chunkXY - playerXY;
			float currentDistSquared = displacementToChunk.CalcLengthSquared();
			if (currentDistSquared < winnerDistSquared )
			{
				bool isAlreadyActive = m_activeChunks.end() != m_activeChunks.find(chunkCoords);
				if (!isAlreadyActive)
				{
					winnerDistSquared = currentDistSquared;
					winner = chunkCoords;
					activatingAChunk = true;
				}
			}
		}
	}

	if (!activatingAChunk)
		return false;

	std::vector<unsigned char> outBuffer;
	if (!LoadBinaryFileToBuffer(Stringf("Data/Save/Chunk_at_(%i,%i).chocolate", winner.x, winner.y).c_str(), outBuffer))
	{
		Chunk* newChunk = CreateChunk(winner);
		ASSERT_OR_DIE(newChunk != nullptr, "Chunk was null!");
		m_activeChunks[winner] = newChunk;
		//LightingOnChunkActivation(newChunk);
		return true;
	}
	else
	{
		LoadChunkAsActive(outBuffer,winner);
		return true;
	}
}

bool World::DeactivateFarthestChunk(const Vector3& playerPosition)
{
	float farthestChunkDistanceSquared = 0.f;
	Vector2 playerPositionXY(playerPosition.x, playerPosition.y);
	ChunkCoords chunkToRemove(0, 0);

	for (std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.begin(); iterate != m_activeChunks.end(); ++iterate)
	{
		ChunkCoords currentChunkPos = iterate->first;
		Vector3 chunkCenter = GetWorldCenterOfChunkCoords(currentChunkPos);
		Vector2 chunkCenterXY(chunkCenter.x, chunkCenter.y);
		float distanceSquared = CalcDistanceSquared(playerPositionXY, chunkCenterXY);
		if (distanceSquared > farthestChunkDistanceSquared )
		{
			farthestChunkDistanceSquared = distanceSquared;
			chunkToRemove = currentChunkPos;
		}
	}

	RemoveChunkAtCoords(chunkToRemove);
	return true;
}

ChunkCoords World::ConvertWorldPositionToChunkPosition( const Vector3& worldPosition)
{
	int xValue = (int)floor(worldPosition.x);
	int yValue = (int)floor(worldPosition.y);
	xValue >>= CHUNK_BITS_X;
	yValue >>= CHUNK_BITS_Y;
	return ChunkCoords(xValue, yValue);
}

Vector3 World::ConvertChunkPositionToWorldPosition( const ChunkCoords& chunkPosition)
{
	float xValue = (float)( chunkPosition.x * CHUNK_WIDTH_X );
	float yValue = (float)( chunkPosition.y * CHUNK_DEPTH_Y );
	return Vector3(xValue, yValue, 0.f);
}

BlockInfo World::GetBlockInfoAtWorldPosition(const Vector3& worldPosition)
{
	ChunkCoords chunkPos = ConvertWorldPositionToChunkPosition(worldPosition);
	Vector3 chunkToWorld = ConvertChunkPositionToWorldPosition(chunkPos);
	Vector3 relativeLocalBlockPos = worldPosition - chunkToWorld;
	IntVector3 localBlockPos((int)relativeLocalBlockPos.x, (int)relativeLocalBlockPos.y, (int)relativeLocalBlockPos.z);
	Chunk* chunk = nullptr;
	if (!AreLocalCoordsUnreasonable(localBlockPos))
	{
		std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.find(chunkPos);
		if (iterate != m_activeChunks.end())
			chunk = iterate->second;
	}

	int blockIndex = 0; 
	if(chunk != nullptr)
		blockIndex = chunk->GetBlockIndexForLocalCoords(localBlockPos);
	return BlockInfo(chunk, blockIndex);
}

bool World::AreLocalCoordsUnreasonable(IntVector3& localPos)
{
	if (localPos.x < 0 || localPos.x > CHUNK_WIDTH_X)
		return true;
	else if (localPos.y < 0 || localPos.y > CHUNK_DEPTH_Y)
		return true;
	else if (localPos.z < 0 || localPos.z > CHUNK_HEIGHT_Z)
		return true;
	else
		return false;
}

int World::GetNumberOfActiveChunks()
{
	return m_activeChunks.size();
}

void World::RemoveChunkAtCoords(const ChunkCoords& chunkToRemove)//#TODO: Rewrite to use one fwrite and chunk dimensions
{
	std::map<ChunkCoords, Chunk*>::iterator found = m_activeChunks.find(chunkToRemove);
	if (found == m_activeChunks.end())
		return;

	Chunk* currentChunk = found->second;
	ASSERT_OR_DIE(currentChunk != nullptr, "Chunk was null!");

	std::vector<unsigned char> bitBuffer;
	unsigned char xDimension = (unsigned char)CHUNK_WIDTH_X;
	unsigned char yDimension = (unsigned char)CHUNK_DEPTH_Y;
	unsigned char zDimension = (unsigned char)CHUNK_HEIGHT_Z;
	//fwrite(reinterpret_cast<const void*>(&xCoord), sizeof(unsigned char), sizeof(xCoord), chunkFile);
	//fwrite(reinterpret_cast<const void*>(&yCoord), sizeof(unsigned char), sizeof(yCoord), chunkFile);
	bitBuffer.push_back(xDimension);
	bitBuffer.push_back(yDimension);
	bitBuffer.push_back(zDimension);

	unsigned char blockCount = 1;
	for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		int nextIndex = blockIndex + 1;
		if (nextIndex == NUM_BLOCKS_PER_CHUNK)
		{
			nextIndex = blockIndex;
		}

		const Block& currentBlock = currentChunk->m_blocks[blockIndex];
		const Block& nextBlock = currentChunk->m_blocks[nextIndex];
		unsigned char currentType = currentBlock.m_blockTypeIndex;
		unsigned char nextType = nextBlock.m_blockTypeIndex;
			
		if (currentType == nextType && blockIndex != nextIndex && blockCount < 255)
		{
			++blockCount;
		}
		else 
		{
			//fwrite(reinterpret_cast<const void*>(&currentType), sizeof(unsigned char), sizeof(currentType), chunkFile);
			//fwrite(reinterpret_cast<const void*>(&blockCount), sizeof(unsigned char), sizeof(blockCount), chunkFile);
			bitBuffer.push_back(currentType);
			bitBuffer.push_back(blockCount);
			blockCount = 1;
		}
	}

	SaveBinaryFileFromBuffer(Stringf("Data/Save/Chunk_at_(%i,%i).chocolate", chunkToRemove.x, chunkToRemove.y).c_str(), bitBuffer);


	if (currentChunk->m_westNeighbor != nullptr)
		currentChunk->m_westNeighbor->m_eastNeighbor = nullptr;

	if (currentChunk->m_eastNeighbor != nullptr)
		currentChunk->m_eastNeighbor->m_westNeighbor = nullptr;

	if (currentChunk->m_northNeighbor != nullptr)
		currentChunk->m_northNeighbor->m_southNeighbor = nullptr;

	if (currentChunk->m_southNeighbor != nullptr)
		currentChunk->m_southNeighbor->m_northNeighbor = nullptr;

	m_activeChunks.erase(found);
	delete currentChunk;
}

void World::LoadChunkAsActive(std::vector<unsigned char>& outBuffer, const ChunkCoords& chunkCoords)
{
	unsigned char xDimension = outBuffer[0];
	outBuffer.erase(outBuffer.begin());
	unsigned char yDimension = outBuffer[1];
	outBuffer.erase(outBuffer.begin() + 1);
	unsigned char zDimension = outBuffer[2];
	outBuffer.erase(outBuffer.begin() + 2);

	ASSERT_OR_DIE(xDimension != CHUNK_WIDTH_X || yDimension != CHUNK_DEPTH_Y || zDimension != CHUNK_HEIGHT_Z, "Chunk Dimensions do not Match!");

	Chunk* loadedChunk = new Chunk(chunkCoords);
	ASSERT_OR_DIE(loadedChunk != nullptr, "Chunk was null!");
	loadedChunk->m_northNeighbor = GetChunkAtCoords(chunkCoords + ChunkCoords(0, 1));
	loadedChunk->m_eastNeighbor = GetChunkAtCoords(chunkCoords + ChunkCoords(1, 0));
	loadedChunk->m_southNeighbor = GetChunkAtCoords(chunkCoords + ChunkCoords(0, -1));
	loadedChunk->m_westNeighbor = GetChunkAtCoords(chunkCoords + ChunkCoords(-1, 0));

	if (loadedChunk->m_westNeighbor != nullptr)
		loadedChunk->m_westNeighbor->m_eastNeighbor = loadedChunk;

	if (loadedChunk->m_eastNeighbor != nullptr)
		loadedChunk->m_eastNeighbor->m_westNeighbor = loadedChunk;

	if (loadedChunk->m_northNeighbor != nullptr)
		loadedChunk->m_northNeighbor->m_southNeighbor = loadedChunk;

	if (loadedChunk->m_southNeighbor != nullptr)
		loadedChunk->m_southNeighbor->m_northNeighbor = loadedChunk;

	loadedChunk->DirtyNeighbors();

	int blockIndex = 0;
	Block blockList[NUM_BLOCKS_PER_CHUNK];
	for ( ;; )
	{
		unsigned char blockType = outBuffer[0];
		outBuffer.erase(outBuffer.begin());
		unsigned char blockTypeCount = outBuffer[1];
		outBuffer.erase(outBuffer.begin() + 1);
		unsigned char lightAndFlags = GetLightAndFlagsForBlockType((BlockType)blockType);
		for (int numBlockTypes = 0; numBlockTypes < (int)blockTypeCount; ++numBlockTypes)
		{
			ASSERT_OR_DIE(blockIndex < NUM_BLOCKS_PER_CHUNK, "Too many blocks!");
			loadedChunk->m_blocks[blockIndex] = Block(blockType, lightAndFlags);
			++blockIndex;
		}
		if (blockIndex >= NUM_BLOCKS_PER_CHUNK)
			break;
	}

	m_activeChunks[chunkCoords] = loadedChunk;
	//LightingOnChunkActivation(loadedChunk);
}

void World::PlayerDigOrPlaceBlock()
{
	if (!g_playerPlacedBlock && !g_playerDestroyedBlock)
		return;

	const int Num_Steps = 1000;
	Vector3 forwardVec = g_theGame->m_camera.GetForwardXYZ();
	Vector3 displacement = forwardVec * 8.f;
	Vector3 singleStep = displacement / (float)Num_Steps;
	Vector3 currentWorldPos = g_theGame->m_camera.m_position;

	for (int step = 0; step < Num_Steps; step++)
	{
		if(m_activeChunks.size() != 0)
		{
			currentWorldPos = g_theGame->m_camera.m_position + (singleStep * (float)step);
			BlockInfo currentBlockInfo = GetBlockInfoAtWorldPosition(currentWorldPos);
			Chunk* currentChunk = currentBlockInfo.m_chunk;
			int& blockIndex = currentBlockInfo.m_blockIndex;
			
			if(currentChunk != nullptr)
			{
				Block& currentBlock = currentChunk->m_blocks[blockIndex];
				BlockType currentBlockType = BlockDefinition(currentBlock.m_blockTypeIndex).m_blockType;

				if (currentBlockType == AIR || currentBlockType == WATER)
					continue;

				BlockInfo blockToReplace(currentChunk, blockIndex);

				Vector3 blockXPosNorm(1.f, 0.f, 0.f);
				Vector3 blockXNegNorm(-1.f, 0.f, 0.f);
				Vector3 blockYPosNorm(0.f, 1.f, 0.f);
				Vector3 blockYNegNorm(0.f, -1.f, 0.f);
				Vector3 blockZPosNorm(0.f, 0.f, 1.f);
				Vector3 blockZNegNorm(0.f, 0.f, -1.f);

				float dotXPos = DotProduct(-1.f * forwardVec, blockXPosNorm);
				float dotXNeg = DotProduct(-1.f * forwardVec, blockXNegNorm);
				float dotYPos = DotProduct(-1.f * forwardVec, blockYPosNorm);
				float dotYNeg = DotProduct(-1.f * forwardVec, blockYNegNorm);
				float dotZPos = DotProduct(-1.f * forwardVec, blockZPosNorm);

				if (dotXPos > 0.7f)
				{
					blockToReplace = currentBlockInfo.GetEastNeighbor();
				}
				else if (dotXNeg > 0.7f)
				{
					blockToReplace = currentBlockInfo.GetWestNeighbor();
				}
				else if (dotYPos > 0.7f)
				{
					blockToReplace = currentBlockInfo.GetNorthNeighbor();
				}
				else if (dotYNeg > 0.7f)
				{
					blockToReplace = currentBlockInfo.GetSouthNeighbor();
				}
				else if (dotZPos < 1.f)
				{
					blockToReplace = currentBlockInfo.GetTopNeighbor();
				}
				else
				{
					blockToReplace = currentBlockInfo.GetBottomNeighbor();
				}

				if (g_playerPlacedBlock)
				{
					currentChunk->m_isVertexArrayDirty = true;
					Block* blockToBeReplaced = blockToReplace.GetBlockPointer();

					if (g_theGame->m_player->m_blockList[g_selectedBlockIndex].m_blockTypeIndex != BlockType::HOOKSHOT)
						*blockToBeReplaced = g_theGame->m_player->m_blockList[g_selectedBlockIndex];
					
					blockToReplace.SetAllNeighborsDirtyFlagAsTrue();
					g_playerPlacedBlock = false;
				}

				if (g_playerDestroyedBlock)
				{
					if (g_theGame->m_player->m_firedHookShot != nullptr)
					{
						return;
					}

					currentChunk->m_isVertexArrayDirty = true;
					currentChunk->DirtyNeighbors();
					currentBlock = Block(AIR, 0b01000000);
					currentBlockInfo.SetAllNeighborsDirtyFlagAsTrue();
					g_playerDestroyedBlock = false;
				}
			}
		}
	}
}

void World::PullPlayerTowardHookShot(float deltaSeconds)
{
	Player*& player = g_theGame->m_player;
	HookShot*& hookShot = player->m_firedHookShot;
	Vector3& hookFiredPos = hookShot->m_firedPosition;
	Vector3& hookEndPos = hookShot->m_endPosition;
	if (hookFiredPos != hookEndPos)
		return;

	Vector3 directionToEnd = hookEndPos - player->m_position;
	directionToEnd.Normalize();
	player->m_velocity += 12.f * directionToEnd * deltaSeconds;

}

void World::FireHookShot()
{
	const int Num_Steps = 1000;
	Vector3 forwardVec = g_theGame->m_camera.GetForwardXYZ();
	Vector3 displacement = forwardVec * 50.f;
	Vector3 singleStep = displacement / (float)Num_Steps;
	Vector3 currentWorldPos = g_theGame->m_player->m_position;
	BlockInfo currentBlockInfo = GetBlockInfoAtWorldPosition(currentWorldPos);

	g_theGame->m_player->m_firedHookShot = new HookShot(currentBlockInfo, currentWorldPos, forwardVec);

	for (int step = 0; step < Num_Steps; step++)
	{
		if (m_activeChunks.size() != 0)
		{
			currentWorldPos = g_theGame->m_player->m_position + (singleStep * (float)step);
			currentBlockInfo = GetBlockInfoAtWorldPosition(currentWorldPos);
			Chunk* currentChunk = currentBlockInfo.m_chunk;

			if (currentChunk != nullptr)
			{
				if (!currentBlockInfo.IsBlockSolid())
					continue;

				g_theGame->m_player->m_firedHookShot->m_anchorBlock = currentBlockInfo;
				g_theGame->m_player->m_firedHookShot->m_endPosition = currentWorldPos;
				return;
			}
		}
	}
}

Chunk* World::GetChunkAtCoords(const ChunkCoords& chunkCoords)
{
	std::map<ChunkCoords, Chunk*>::iterator iterate = m_activeChunks.find(chunkCoords);
	if (iterate != m_activeChunks.end())
	{
		return iterate->second;
	}
	return nullptr;
}

void World::UpdateLighting()
{
	AddAnyDirtyBlocksToQueue();
	while (!m_dirtyLightBlocksQueue.empty())
	{
		BlockInfo dirtyBlockInfo = m_dirtyLightBlocksQueue.front();
		m_dirtyLightBlocksQueue.pop_front();
		dirtyBlockInfo.SetDirtyFlagAsFalse();
		UpdateLightForBlock(dirtyBlockInfo);
	}
	int x = 5;
}

void World::UpdateLightForBlock(BlockInfo dirtyBlockInfo)
{
	int idealLight = CalculateIdealLight(dirtyBlockInfo);
	int currentLight = (int)dirtyBlockInfo.GetLightValueForBlock();
	if (idealLight != currentLight)
	{
		dirtyBlockInfo.SetLightValueForBlock((unsigned char)idealLight);
		MarkNonOpaqueNeighborsDirty(dirtyBlockInfo);
	}
}

void World::MarkNonOpaqueNeighborsDirty(BlockInfo blockUpdated)
{
	BlockInfo northNeighbor = blockUpdated.GetNorthNeighbor();
	if (!northNeighbor.IsBlockOpaque())
	{
		northNeighbor.SetDirtyFlagAsTrue();
	}

	BlockInfo southNeighbor = blockUpdated.GetSouthNeighbor();
	if (!southNeighbor.IsBlockOpaque())
	{
		southNeighbor.SetDirtyFlagAsTrue();
	}

	BlockInfo eastNeighbor = blockUpdated.GetEastNeighbor();
	if (!eastNeighbor.IsBlockOpaque())
	{
		eastNeighbor.SetDirtyFlagAsTrue();
	}

	BlockInfo westNeighbor = blockUpdated.GetWestNeighbor();
	if (!westNeighbor.IsBlockOpaque())
	{
		westNeighbor.SetDirtyFlagAsTrue();
	}

	BlockInfo topNeighbor = blockUpdated.GetTopNeighbor();
	if (!topNeighbor.IsBlockOpaque())
	{
		topNeighbor.SetDirtyFlagAsTrue();
	}

	BlockInfo bottomNeighbor = blockUpdated.GetBottomNeighbor();
	if (!bottomNeighbor.IsBlockOpaque())
	{
		bottomNeighbor.SetDirtyFlagAsTrue();
	}
}

int World::CalculateIdealLight(BlockInfo dirtyBlock)
{
	unsigned char dirtyBlockLight = dirtyBlock.GetLightValueForBlock();
	unsigned char dirtyBlockTypeLightAndFlags = GetLightAndFlagsForBlockType(dirtyBlock.GetBlockType());
	unsigned char dirtyBlockTypeLight = dirtyBlockTypeLightAndFlags & ~BLOCK_LIGHT_MASK;
	if (!(dirtyBlockLight >= dirtyBlockTypeLight))
	{
		dirtyBlockLight = dirtyBlockTypeLight;
	}
	
	if (!dirtyBlock.IsBlockOpaque())
	{
		unsigned char northNeighborLight = dirtyBlock.GetNorthNeighbor().GetLightValueForBlock();
		unsigned char southNeighborLight = dirtyBlock.GetSouthNeighbor().GetLightValueForBlock();
		unsigned char eastNeighborLight = dirtyBlock.GetEastNeighbor().GetLightValueForBlock();
		unsigned char westNeighborLight = dirtyBlock.GetWestNeighbor().GetLightValueForBlock();
		unsigned char topNeighborLight = dirtyBlock.GetTopNeighbor().GetLightValueForBlock();
		unsigned char bottomNeighborLight = dirtyBlock.GetBottomNeighbor().GetLightValueForBlock();

		if (!(dirtyBlockLight >= (northNeighborLight - 1)))
		{
			dirtyBlockLight = northNeighborLight - 1;
		}
		
		if (!(dirtyBlockLight >= (southNeighborLight - 1)))
		{
			dirtyBlockLight = southNeighborLight - 1;
		}
		
		if (!(dirtyBlockLight >= (eastNeighborLight - 1)))
		{
			dirtyBlockLight = eastNeighborLight - 1;
		}
		
		if (!(dirtyBlockLight >= (westNeighborLight - 1)))
		{
			dirtyBlockLight = westNeighborLight - 1;
		}
		
		if (!(dirtyBlockLight >= (topNeighborLight - 1)))
		{
			dirtyBlockLight = topNeighborLight - 1;
		}
		
		if (!(dirtyBlockLight >= (bottomNeighborLight - 1)))
		{
			dirtyBlockLight = bottomNeighborLight - 1;
		}

		if (!(dirtyBlockLight >= SKY_LIGHT) && dirtyBlock.IsBlockSkyBlock())
		{
			dirtyBlockLight = SKY_LIGHT;
		}
	}
	return (int)dirtyBlockLight;
}

void World::LightingOnChunkActivation(Chunk* chunkActivated)
{
	InitializeLightForAllBlocks(chunkActivated);
	DescendColumnAndMarkSky(chunkActivated);
	DescendWhileSkyAndMarkDirty(chunkActivated);
	DirtySelfIlluminateBlocks(chunkActivated);
}

void World::InitializeLightForAllBlocks(Chunk* chunkActivated)
{
	for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		BlockInfo currentBlock(chunkActivated, blockIndex);
		currentBlock.SetLightValueForBlock((unsigned char)0);
	}
}

void World::DescendColumnAndMarkSky(Chunk* chunkActivated)
{
	int startingZIndex = NUM_BLOCKS_PER_CHUNK - BLOCKS_PER_LAYER;
	for (int columnIndex = startingZIndex; columnIndex < NUM_BLOCKS_PER_CHUNK; ++columnIndex)
	{
		int blockIndex = columnIndex;
		for (;;)
		{
			BlockInfo currentBlock(chunkActivated, blockIndex);
			if (!currentBlock.IsBlockOpaque())
			{
				currentBlock.SetSkyFlagAsTrue();
				currentBlock.SetLightValueForBlock((unsigned char)SKY_LIGHT);
			}
			blockIndex -= BLOCKS_PER_LAYER;

			if(currentBlock.IsBlockOpaque() || blockIndex < 0)
				break;
		}
	}
}

void World::DescendWhileSkyAndMarkDirty(Chunk* chunkActivated)
{
	int startingZIndex = NUM_BLOCKS_PER_CHUNK - BLOCKS_PER_LAYER;
	for (int columnIndex = startingZIndex; columnIndex < NUM_BLOCKS_PER_CHUNK; ++columnIndex)
	{
		int blockIndex = columnIndex;
		for (;;)
		{
			BlockInfo currentBlock(chunkActivated, blockIndex);
			if(!currentBlock.IsBlockSkyBlock() || currentBlock.IsBlockOpaque())
				break;

			BlockInfo northNeighbor = currentBlock.GetNorthNeighbor();
			if (!northNeighbor.IsBlockOpaque() && !northNeighbor.IsBlockSkyBlock())
			{
				northNeighbor.SetDirtyFlagAsTrue();
			}

			BlockInfo southNeighbor = currentBlock.GetSouthNeighbor();
			if (!southNeighbor.IsBlockOpaque() && !southNeighbor.IsBlockSkyBlock())
			{
				southNeighbor.SetDirtyFlagAsTrue();
			}

			BlockInfo eastNeighbor = currentBlock.GetEastNeighbor();
			if (!eastNeighbor.IsBlockOpaque() && !eastNeighbor.IsBlockSkyBlock())
			{
				eastNeighbor.SetDirtyFlagAsTrue();
			}

			BlockInfo westNeighbor = currentBlock.GetWestNeighbor();
			if (!westNeighbor.IsBlockOpaque() && !westNeighbor.IsBlockSkyBlock())
			{
				westNeighbor.SetDirtyFlagAsTrue();
			}

			blockIndex -= BLOCKS_PER_LAYER;
			if (blockIndex < 0)
				break;
		}
	}
}

void World::DirtySelfIlluminateBlocks(Chunk* chunkActivated)
{
	for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		BlockInfo currentBlock(chunkActivated, blockIndex);
		if (currentBlock.GetLightValueForBlock() > 0)
			currentBlock.SetDirtyFlagAsTrue();
	}
}

void World::AddAnyDirtyBlocksToQueue()
{
	for (std::map<ChunkCoords, Chunk*>::const_iterator iterate = m_activeChunks.begin(); iterate != m_activeChunks.end(); ++iterate)
	{
		Chunk* chunk = iterate->second;
		for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
		{
			BlockInfo currentBlock(chunk, blockIndex);
			if (currentBlock.IsBlockDirty())
				m_dirtyLightBlocksQueue.push_back(currentBlock);
		}
	}
}

void World::ApplyCollisionPhysicsBetweenPlayerAndBlocks()
{
	PhysicsForPlayerFaces();
	PhysicsForPlayerEdges();
	PhysicsForPlayerCorners();
	if(g_IsPlayerWalking)
		AddHorizontalFrictionOnGround();
	else
		AddHorizontalAndVerticalFrictionWhileFlying();
}

void World::PhysicsForPlayerFaces()
{
	Vector3 playerPosition = g_theGame->m_player->m_position;
	//block Below Player
	Vector3 playerBottomCenter = playerPosition - Vector3(0.f, 0.f, 0.93f);
	BlockInfo blockAtPlayersBottomCenter = GetBlockInfoAtWorldPosition(playerBottomCenter);
	if (blockAtPlayersBottomCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersBottomCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		AudioChannelHandle channel = g_theAudioSystem->GetChannelForChannelID(3);
		SoundID pauserSound = g_theAudioSystem->CreateOrGetSound("Data/Audio/Walk.wav");
		g_theAudioSystem->PlaySound(pauserSound, 0.05f, channel);

		if (blockBounds.maxs.z > playerBottomCenter.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerBottomCenter.z;
			g_theGame->m_player->m_position.z += overlapInZ;
			g_theGame->m_player->m_velocity.z = 0.f;
		}
	}

	//block Above Player
	Vector3 playerTopCenter = playerPosition + Vector3(0.f, 0.f, 0.93f);
	BlockInfo blockAtPlayersTopCenter = GetBlockInfoAtWorldPosition(playerTopCenter);
	if (blockAtPlayersTopCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersTopCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.z < playerTopCenter.z)
		{
			float overlapInZ = playerTopCenter.z - blockBounds.mins.z;
			g_theGame->m_player->m_position.z -= overlapInZ;
			g_theGame->m_player->m_velocity.z = 0.f;
		}
	}

	//block +X Bottom Player
	Vector3 playerForwardBottom = playerPosition + Vector3(0.31f, 0.f, -0.47f);
	BlockInfo blockAtPlayerForwardBottom = GetBlockInfoAtWorldPosition(playerForwardBottom);
	if (blockAtPlayerForwardBottom.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerForwardBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.x < playerForwardBottom.x)
		{
			float overlapInX = playerForwardBottom.x - blockBounds.mins.x;
			g_theGame->m_player->m_position.x -= overlapInX;
			g_theGame->m_player->m_velocity.x = 0.f;
		}
	}

	//block -X Bottom Player
	Vector3 playerBackwardBottom = playerPosition - Vector3(0.31f, 0.f, 0.47f);
	BlockInfo blockAtPlayerBackwardBottom = GetBlockInfoAtWorldPosition(playerBackwardBottom);
	if (blockAtPlayerBackwardBottom.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerBackwardBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.x > playerBackwardBottom.x)
		{
			float overlapInX = blockBounds.maxs.x - playerBackwardBottom.x;
			g_theGame->m_player->m_position.x += overlapInX;
			g_theGame->m_player->m_velocity.x = 0.f;
		}
	}

	//block +X Top Player
	Vector3 playerForwardTop = playerPosition + Vector3(0.31f, 0.f, 0.47f);
	BlockInfo blockAtPlayerForwardTop = GetBlockInfoAtWorldPosition(playerForwardTop);
	if (blockAtPlayerForwardTop.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerForwardTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.x < playerForwardTop.x)
		{
			float overlapInX = playerForwardTop.x - blockBounds.mins.x;
			g_theGame->m_player->m_position.x -= overlapInX;
			g_theGame->m_player->m_velocity.x = 0.f;
		}
	}

	//block -X Top Player
	Vector3 playerBackwardTop = playerPosition - Vector3(0.31f, 0.f, -0.47f);
	BlockInfo blockAtPlayerBackwardTop = GetBlockInfoAtWorldPosition(playerBackwardTop);
	if (blockAtPlayerBackwardTop.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerBackwardTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.x > playerBackwardTop.x)
		{
			float overlapInX = blockBounds.maxs.x - playerBackwardTop.x;
			g_theGame->m_player->m_position.x += overlapInX;
			g_theGame->m_player->m_velocity.x = 0.f;
		}
	}

	//block +Y Bottom Player
	Vector3 playerYPosBottom = playerPosition + Vector3(0.f, 0.31f, -0.47f);
	BlockInfo blockAtPlayerYPosBottom = GetBlockInfoAtWorldPosition(playerYPosBottom);
	if (blockAtPlayerYPosBottom.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerForwardBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.y < playerYPosBottom.y)
		{
			float overlapInY = playerYPosBottom.y - blockBounds.maxs.y;
			g_theGame->m_player->m_position.y -= overlapInY;
			g_theGame->m_player->m_velocity.y = 0.f;
		}
	}

	//block -Y Bottom Player
	Vector3 playerYNegBottom = playerPosition - Vector3(0.f, 0.31f, 0.47f);
	BlockInfo blockAtPlayerYNegBottom = GetBlockInfoAtWorldPosition(playerYNegBottom);
	if (blockAtPlayerYNegBottom.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerYNegBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.y > playerYNegBottom.y)
		{
			float overlapInY = blockBounds.maxs.y - playerYNegBottom.y;
			g_theGame->m_player->m_position.y += overlapInY;
			g_theGame->m_player->m_velocity.y = 0.f;
		}
	}

	//block +Y Top Player
	Vector3 playerYPosTop = playerPosition + Vector3(0.f, 0.31f, 0.47f);
	BlockInfo blockAtPlayerYPosTop = GetBlockInfoAtWorldPosition(playerYPosTop);
	if (blockAtPlayerYPosTop.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerYPosTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.y < playerYPosTop.y)
		{
			float overlapInY = playerYPosTop.y - blockBounds.maxs.y;
			g_theGame->m_player->m_position.y -= overlapInY;
			g_theGame->m_player->m_velocity.y = 0.f;
		}
	}

	//block -Y Top Player
	Vector3 playerYNegTop = playerPosition - Vector3(0.f, 0.31f, -0.47f);
	BlockInfo blockAtPlayerYNegTop = GetBlockInfoAtWorldPosition(playerYNegTop);
	if (blockAtPlayerYNegTop.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerYNegTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.y > playerYNegTop.y)
		{
			float overlapInY = blockBounds.maxs.y - playerYNegTop.y;
			g_theGame->m_player->m_position.y += overlapInY;
			g_theGame->m_player->m_velocity.y = 0.f;
		}
	}
}

void World::PhysicsForPlayerEdges()
{
	Vector3& playerVelocity = g_theGame->m_player->m_velocity;
	Vector3& playerPosition = g_theGame->m_player->m_position;
	//+X +Y Bottom
	Vector3 playerXPosYPosBottom = playerPosition + Vector3(0.31f, 0.31f, -0.47f);
	BlockInfo blockAtPlayerXPosYPosBottom = GetBlockInfoAtWorldPosition(playerXPosYPosBottom);
	if (blockAtPlayerXPosYPosBottom.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXPosYPosBottom.x, playerXPosYPosBottom.y);
		Vector3 blockPositionInWorld = blockAtPlayerXPosYPosBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x > blockBounds.mins.x && pointInsideBlock.y > blockBounds.mins.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.mins.x, blockBounds.mins.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x -= overlapInX;
			playerPosition.y -= overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//+X +Y Top
	Vector3 playerXPosYPosTop = playerPosition + Vector3(0.31f, 0.31f, 0.47f);
	BlockInfo blockAtPlayerXPosYPosTop = GetBlockInfoAtWorldPosition(playerXPosYPosTop);
	if (blockAtPlayerXPosYPosTop.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXPosYPosTop.x, playerXPosYPosTop.y);
		Vector3 blockPositionInWorld = blockAtPlayerXPosYPosTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x > blockBounds.mins.x && pointInsideBlock.y > blockBounds.mins.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.mins.x, blockBounds.mins.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x -= overlapInX;
			playerPosition.y -= overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//-X +Y Bottom
	Vector3 playerXNegYPosBottom = playerPosition + Vector3(-0.31f, 0.31f, -0.47f);
	BlockInfo blockAtPlayerXNegYPosBottom = GetBlockInfoAtWorldPosition(playerXNegYPosBottom);
	if (blockAtPlayerXNegYPosBottom.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXNegYPosBottom.x, playerXNegYPosBottom.y);
		Vector3 blockPositionInWorld = blockAtPlayerXNegYPosBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x < blockBounds.maxs.x && pointInsideBlock.y > blockBounds.mins.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.maxs.x, blockBounds.mins.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x += overlapInX;
			playerPosition.y -= overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//-X +Y Top
	Vector3 playerXNegYPosTop = playerPosition + Vector3(-0.31f, 0.31f, 0.47f);
	BlockInfo blockAtPlayerXNegYPosTop = GetBlockInfoAtWorldPosition(playerXNegYPosTop);
	if (blockAtPlayerXNegYPosTop.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXNegYPosTop.x, playerXNegYPosTop.y);
		Vector3 blockPositionInWorld = blockAtPlayerXNegYPosTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x < blockBounds.maxs.x && pointInsideBlock.y > blockBounds.mins.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.maxs.x, blockBounds.mins.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x += overlapInX;
			playerPosition.y -= overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//-X -Y Bottom
	Vector3 playerXNegYNegBottom = playerPosition + Vector3(-0.31f, -0.31f, -0.47f);
	BlockInfo blockAtPlayerXNegYNegBottom = GetBlockInfoAtWorldPosition(playerXNegYNegBottom);
	if (blockAtPlayerXNegYNegBottom.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXNegYNegBottom.x, playerXNegYNegBottom.y);
		Vector3 blockPositionInWorld = blockAtPlayerXNegYNegBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x < blockBounds.maxs.x && pointInsideBlock.y < blockBounds.maxs.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.maxs.x, blockBounds.maxs.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x += overlapInX;
			playerPosition.y += overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//-X -Y Top
	Vector3 playerXNegYNegTop = playerPosition + Vector3(-0.31f, -0.31f, 0.47f);
	BlockInfo blockAtPlayerXNegYNegTop = GetBlockInfoAtWorldPosition(playerXNegYNegTop);
	if (blockAtPlayerXNegYNegTop.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXNegYNegTop.x, playerXNegYNegTop.y);
		Vector3 blockPositionInWorld = blockAtPlayerXNegYNegTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x < blockBounds.maxs.x && pointInsideBlock.y < blockBounds.maxs.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.maxs.x, blockBounds.maxs.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x += overlapInX;
			playerPosition.y += overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//+X -Y Bottom
	Vector3 playerXPosYNegBottom = playerPosition + Vector3(0.31f, -0.31f, -0.47f);
	BlockInfo blockAtPlayerXPosYNegBottom = GetBlockInfoAtWorldPosition(playerXPosYNegBottom);
	if (blockAtPlayerXPosYNegBottom.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXPosYNegBottom.x, playerXPosYNegBottom.y);
		Vector3 blockPositionInWorld = blockAtPlayerXPosYNegBottom.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x > blockBounds.mins.x && pointInsideBlock.y < blockBounds.maxs.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.mins.x, blockBounds.maxs.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x -= overlapInX;
			playerPosition.y += overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//+X -Y Top
	Vector3 playerXPosYNegTop = playerPosition + Vector3(0.31f, -0.31f, 0.47f);
	BlockInfo blockAtPlayerXPosYNegTop = GetBlockInfoAtWorldPosition(playerXPosYNegTop);
	if (blockAtPlayerXPosYNegTop.IsBlockSolid())
	{
		Vector2 pointInsideBlock(playerXPosYNegTop.x, playerXPosYNegTop.y);
		Vector3 blockPositionInWorld = blockAtPlayerXPosYNegTop.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (pointInsideBlock.x > blockBounds.mins.x && pointInsideBlock.y < blockBounds.maxs.y)
		{
			Vector2 overlap = pointInsideBlock - Vector2(blockBounds.mins.x, blockBounds.maxs.y);
			Vector2 normVelocity(playerVelocity.x, playerVelocity.y);
			normVelocity.Normalize();
			float overlapInX = normVelocity.x * overlap.x;
			float overlapInY = normVelocity.y * overlap.y;
			playerPosition.x -= overlapInX;
			playerPosition.y += overlapInY;
			playerVelocity.x = 0.f;
			playerVelocity.y = 0.f;
		}
	}

	//+X Center bottom edge
	Vector3 playerXPosBottomCenter = playerPosition + Vector3(0.31f, 0.f, -0.93f);
	BlockInfo blockAtPlayersXPosBottomCenter = GetBlockInfoAtWorldPosition(playerXPosBottomCenter);
	if (blockAtPlayersXPosBottomCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersXPosBottomCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.z > playerXPosBottomCenter.z && blockBounds.mins.z + 0.5f < playerXPosBottomCenter.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerXPosBottomCenter.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//-X Center bottom edge
	Vector3 playerXNegBottomCenter = playerPosition + Vector3(-0.31f, 0.f, -0.93f);
	BlockInfo blockAtPlayersXNegBottomCenter = GetBlockInfoAtWorldPosition(playerXNegBottomCenter);
	if (blockAtPlayersXNegBottomCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersXNegBottomCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.z > playerXNegBottomCenter.z && blockBounds.mins.z + 0.5f < playerXNegBottomCenter.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerXNegBottomCenter.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//+Y Center bottom edge
	Vector3 playerYPosBottomCenter = playerPosition + Vector3(0.f, 0.31f, -0.93f);
	BlockInfo blockAtPlayersYPosBottomCenter = GetBlockInfoAtWorldPosition(playerYPosBottomCenter);
	if (blockAtPlayersYPosBottomCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersYPosBottomCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.z > playerYPosBottomCenter.z && blockBounds.mins.z + 0.5f < playerYPosBottomCenter.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerYPosBottomCenter.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//-Y Center bottom edge
	Vector3 playerYNegBottomCenter = playerPosition + Vector3(0.f, -0.31f, -0.93f);
	BlockInfo blockAtPlayersYNegBottomCenter = GetBlockInfoAtWorldPosition(playerYNegBottomCenter);
	if (blockAtPlayersYNegBottomCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersYNegBottomCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.z > playerYNegBottomCenter.z && blockBounds.mins.z + 0.5f < playerYNegBottomCenter.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerYNegBottomCenter.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}
	//-------------------------------------------
	//+X Center top edge
	Vector3 playerXPosTopCenter = playerPosition + Vector3(0.31f, 0.f, 0.93f);
	BlockInfo blockAtPlayersXPosTopCenter = GetBlockInfoAtWorldPosition(playerXPosTopCenter);
	if (blockAtPlayersXPosTopCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersXPosTopCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.z < playerXPosTopCenter.z)
		{
			float overlapInZ = playerXPosTopCenter.z - blockBounds.mins.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//-X Center top edge
	Vector3 playerXNegTopCenter = playerPosition + Vector3(-0.31f, 0.f, 0.93f);
	BlockInfo blockAtPlayersXNegTopCenter = GetBlockInfoAtWorldPosition(playerXNegTopCenter);
	if (blockAtPlayersXNegTopCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersXNegTopCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.z < playerXNegTopCenter.z && blockBounds.maxs.z - 0.5f > playerXNegTopCenter.z)
		{
			float overlapInZ = playerXPosTopCenter.z - blockBounds.mins.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//+Y Center top edge
	Vector3 playerYPosTopCenter = playerPosition + Vector3(0.f, 0.31f, 0.93f);
	BlockInfo blockAtPlayersYPosTopCenter = GetBlockInfoAtWorldPosition(playerYPosTopCenter);
	if (blockAtPlayersYPosTopCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersYPosTopCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.z < playerYPosTopCenter.z && blockBounds.maxs.z - 0.5f > playerYPosTopCenter.z)
		{
			float overlapInZ = playerXPosTopCenter.z - blockBounds.mins.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//-Y Center top edge
	Vector3 playerYNegTopCenter = playerPosition + Vector3(0.f, -0.31f, 0.93f);
	BlockInfo blockAtPlayersYNegTopCenter = GetBlockInfoAtWorldPosition(playerYNegTopCenter);
	if (blockAtPlayersYNegTopCenter.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayersYNegTopCenter.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.z < playerYNegTopCenter.z)
		{
			float overlapInZ = playerXPosTopCenter.z - blockBounds.mins.z;
			if(overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}
}

void World::PhysicsForPlayerCorners()
{
	Vector3 playerPosition = g_theGame->m_player->m_position;
	//Min, Min Bottom
	Vector3 playerMinCorner = g_theGame->m_player->m_body.mins;
	BlockInfo blockAtPlayerMinCorner = GetBlockInfoAtWorldPosition(playerMinCorner);
	if (blockAtPlayerMinCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMinCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.x < playerMinCorner.x && blockBounds.maxs.y < playerMinCorner.y && blockBounds.maxs.z < playerMinCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMinCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Min, Max Bottom
	Vector3 playerMinMaxBottomCorner(g_theGame->m_player->m_body.maxs.x, g_theGame->m_player->m_body.mins.y, g_theGame->m_player->m_body.mins.z);
	BlockInfo blockAtPlayerMinMaxsBottomCorner = GetBlockInfoAtWorldPosition(playerMinMaxBottomCorner);
	if (blockAtPlayerMinMaxsBottomCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMinMaxsBottomCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.x > playerMinCorner.x && blockBounds.maxs.y < playerMinCorner.y && blockBounds.maxs.z < playerMinCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMinMaxBottomCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Max, Max Bottom
	Vector3 playerMaxBottomCorner(g_theGame->m_player->m_body.maxs.x, g_theGame->m_player->m_body.maxs.y, g_theGame->m_player->m_body.mins.z);
	BlockInfo blockAtPlayerMaxsBottomCorner = GetBlockInfoAtWorldPosition(playerMaxBottomCorner);
	if (blockAtPlayerMaxsBottomCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMaxsBottomCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.x > playerMinCorner.x && blockBounds.mins.y > playerMinCorner.y && blockBounds.maxs.z < playerMinCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMaxBottomCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Max, Min Bottom
	Vector3 playerMaxMinBottomCorner(g_theGame->m_player->m_body.mins.x, g_theGame->m_player->m_body.maxs.y, g_theGame->m_player->m_body.mins.z);
	BlockInfo blockAtPlayerMaxsMinsBottomCorner = GetBlockInfoAtWorldPosition(playerMaxMinBottomCorner);
	if (blockAtPlayerMaxsMinsBottomCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMaxsMinsBottomCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.x < playerMinCorner.x && blockBounds.mins.y > playerMinCorner.y && blockBounds.maxs.z < playerMinCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMaxMinBottomCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z += overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Min, Min Top
	Vector3 playerMinTopCorner(g_theGame->m_player->m_body.mins.x, g_theGame->m_player->m_body.mins.y, g_theGame->m_player->m_body.maxs.z);
	BlockInfo blockAtPlayerMinTopCorner = GetBlockInfoAtWorldPosition(playerMinTopCorner);
	if (blockAtPlayerMinTopCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMinTopCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.x < playerMinTopCorner.x && blockBounds.maxs.y < playerMinTopCorner.y && blockBounds.mins.z > playerMinTopCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMinTopCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z -= overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Min, Max Top
	Vector3 playerMinMaxTopCorner(g_theGame->m_player->m_body.maxs.x, g_theGame->m_player->m_body.mins.y, g_theGame->m_player->m_body.maxs.z);
	BlockInfo blockAtPlayerMinMaxsTopCorner = GetBlockInfoAtWorldPosition(playerMinMaxTopCorner);
	if (blockAtPlayerMinMaxsTopCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMinMaxsTopCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.x > playerMinMaxTopCorner.x && blockBounds.maxs.y < playerMinMaxTopCorner.y && blockBounds.mins.z > playerMinMaxTopCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMinMaxTopCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z -= overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Max, Max Top
	Vector3 playerMaxTopCorner(g_theGame->m_player->m_body.maxs.x, g_theGame->m_player->m_body.maxs.y, g_theGame->m_player->m_body.maxs.z);
	BlockInfo blockAtPlayerMaxsTopCorner = GetBlockInfoAtWorldPosition(playerMaxTopCorner);
	if (blockAtPlayerMaxsTopCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMaxsTopCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.mins.x > playerMaxTopCorner.x && blockBounds.mins.y > playerMaxTopCorner.y && blockBounds.mins.z > playerMaxTopCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMaxTopCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z -= overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}

	//Max, Min Top
	Vector3 playerMaxMinTopCorner(g_theGame->m_player->m_body.mins.x, g_theGame->m_player->m_body.maxs.y, g_theGame->m_player->m_body.maxs.z);
	BlockInfo blockAtPlayerMaxsMinsTopCorner = GetBlockInfoAtWorldPosition(playerMaxMinTopCorner);
	if (blockAtPlayerMaxsMinsTopCorner.IsBlockSolid())
	{
		Vector3 blockPositionInWorld = blockAtPlayerMaxsMinsTopCorner.GetBlockWorldPosition();
		AABB3D blockBounds(blockPositionInWorld, blockPositionInWorld + Vector3(1.f, 1.f, 1.f));

		if (blockBounds.maxs.x < playerMaxMinTopCorner.x && blockBounds.mins.y > playerMaxMinTopCorner.y && blockBounds.mins.z > playerMaxMinTopCorner.z)
		{
			float overlapInZ = blockBounds.maxs.z - playerMaxMinTopCorner.z;
			if (overlapInZ < 0.1f)
			{
				g_theGame->m_player->m_position.z -= overlapInZ;
				g_theGame->m_player->m_velocity.z = 0.f;
			}
		}
	}
}

void World::AddHorizontalFrictionOnGround()
{
	if (!g_IsPlayerWalking)
		return;

	Vector3& playerVelocity = g_theGame->m_player->m_velocity;
	Vector3& playerPosition = g_theGame->m_player->m_position;
	Vector3 playerBottomCenter = playerPosition - Vector3(0.f, 0.f, 0.98f);
	BlockInfo blockAtPlayersBottomCenter = GetBlockInfoAtWorldPosition(playerBottomCenter);
	BlockInfo blockBelowPlayer = blockAtPlayersBottomCenter.GetBottomNeighbor();
	if(blockBelowPlayer.IsBlockSolid())
	{
		if (playerVelocity.x > 0.0005f)
		{	
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.012f * SmoothStep(direction.x);

			if (playerVelocity.x - amountToStep < 0.f)
				playerVelocity.x = 0.f;
			else
				playerVelocity.x -= amountToStep;
		}

		if (playerVelocity.x < -0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.012f * SmoothStep(direction.x);

			if (playerVelocity.x + amountToStep > 0.f)
				playerVelocity.x = 0.f;
			else
				playerVelocity.x += amountToStep;
		}

		if (playerVelocity.y > 0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.012f * SmoothStep(direction.x);

			if (playerVelocity.y - amountToStep < 0.f)
				playerVelocity.y = 0.f;
			else
				playerVelocity.y -= amountToStep;
		}

		if (playerVelocity.y < -0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.012f * SmoothStep(direction.x);

			if (playerVelocity.y + amountToStep > 0.f)
				playerVelocity.y = 0.f;
			else
				playerVelocity.y += amountToStep;
		}
	}
}

void World::AddHorizontalAndVerticalFrictionWhileFlying()
{
	if (g_IsPlayerWalking)
		return;
	
		Vector3& playerVelocity = g_theGame->m_player->m_velocity;
		Vector3& playerPosition = g_theGame->m_player->m_position;
		if (playerVelocity.x > 0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.1f * SmoothStep(direction.x);

			if (playerVelocity.x - amountToStep < 0.f)
				playerVelocity.x = 0.f;
			else
				playerVelocity.x -= amountToStep;
		}

		if (playerVelocity.x < -0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.1f * SmoothStep(direction.x);

			if (playerVelocity.x + amountToStep > 0.f)
				playerVelocity.x = 0.f;
			else
				playerVelocity.x += amountToStep;
		}

		if (playerVelocity.y > 0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.1f * SmoothStep(direction.y);

			if (playerVelocity.y - amountToStep < 0.f)
				playerVelocity.y = 0.f;
			else
				playerVelocity.y -= amountToStep;
		}

		if (playerVelocity.y < -0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.1f * SmoothStep(direction.y);

			if (playerVelocity.y + amountToStep > 0.f)
				playerVelocity.y = 0.f;
			else
				playerVelocity.y += amountToStep;
		}

		if (playerVelocity.z > 0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.1f * SmoothStep(direction.z);

			if (playerVelocity.z - amountToStep < 0.f)
				playerVelocity.z = 0.f;
			else
				playerVelocity.z -= amountToStep;
		}

		if (playerVelocity.z < -0.0005f)
		{
			Vector3 direction = playerVelocity - playerPosition;
			direction.Normalize();
			float amountToStep = 0.1f * SmoothStep(direction.z);

			if (playerVelocity.z + amountToStep > 0.f)
				playerVelocity.z = 0.f;
			else
				playerVelocity.z += amountToStep;
		}
	
}

float World::RaycastDistanceCanTravel(const Vector3& startingPosition, const Vector3& directionTotravel)
{
	const int Num_Steps = 1000;
	Vector3 displacement = directionTotravel * 8.f;
	Vector3 singleStep = displacement / (float)Num_Steps;
	Vector3 currentWorldPos = startingPosition;

	for (int step = 0; step < Num_Steps; step++)
	{
		if (m_activeChunks.size() != 0)
		{
			currentWorldPos = g_theGame->m_camera.m_position + (singleStep * (float)step);
			BlockInfo currentBlockInfo = GetBlockInfoAtWorldPosition(currentWorldPos);
			Chunk* currentChunk = currentBlockInfo.m_chunk;
			int& blockIndex = currentBlockInfo.m_blockIndex;

			if (currentChunk != nullptr)
			{
				if (currentBlockInfo.IsBlockSolid())
					break;
			}
		}
	}

	return CalcDistance(startingPosition, currentWorldPos) - 3.f;
}