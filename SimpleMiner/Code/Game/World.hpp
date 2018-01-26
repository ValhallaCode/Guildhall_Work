#pragma once
#include "Game/GameCommons.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockInfo.hpp"
#include "Engine/Math/Vector3.hpp"
#include <map>
#include <stdio.h>
#include <deque>

const int DAY_LIGHT = MAX_LEVEL;
const int MOON_LIGHT = 6;
const int SKY_LIGHT = MOON_LIGHT;

class World
{
public:
	std::map<ChunkCoords, Chunk*> m_activeChunks;
	std::deque<BlockInfo> m_dirtyLightBlocksQueue;

	World();
	~World();
	void Update(float deltaSeconds);
	void ChunkManagement();
	void UpdateChunks();
	void SetAllChunksAsDirty();
	void Render() const;
	void RenderActiveChunks() const;
	bool IsChunkInFrustum(const Chunk* chunk) const;
	Chunk* CreateChunk(const ChunkCoords& newPosition);
	bool ActivateNearestMissingChunk( const Vector3& playerPosition);
	bool DeactivateFarthestChunk(const Vector3& playerPosition);
	ChunkCoords ConvertWorldPositionToChunkPosition(const Vector3& worldPosition);
	Vector3 ConvertChunkPositionToWorldPosition( const ChunkCoords& chunkPosition);
	BlockInfo GetBlockInfoAtWorldPosition(const Vector3& worldPosition);
	bool AreLocalCoordsUnreasonable(IntVector3& localPos);
	int GetNumberOfActiveChunks();
	void RemoveChunkAtCoords(const ChunkCoords& chunkToRemove);
	void LoadChunkAsActive(std::vector<unsigned char>& outBuffer, const ChunkCoords& chunkCoords);
	void PlayerDigOrPlaceBlock();
	void PullPlayerTowardHookShot(float deltaSeconds);
	void FireHookShot();
	Chunk* GetChunkAtCoords(const ChunkCoords& chunkCoords);
	void UpdateLighting();
	void UpdateLightForBlock(BlockInfo dirtyBlockInfo);
	void MarkNonOpaqueNeighborsDirty(BlockInfo blockUpdated);
	int CalculateIdealLight(BlockInfo dirtyBlock);
	void LightingOnChunkActivation(Chunk* chunkActivated);
	void InitializeLightForAllBlocks(Chunk* chunkActivated);
	void DescendColumnAndMarkSky(Chunk* chunkActivated);
	void DescendWhileSkyAndMarkDirty(Chunk* chunkActivated);
	void DirtySelfIlluminateBlocks(Chunk* chunkActivated);
	void AddAnyDirtyBlocksToQueue();
	void ApplyCollisionPhysicsBetweenPlayerAndBlocks();
	void PhysicsForPlayerFaces();
	void PhysicsForPlayerEdges();
	void PhysicsForPlayerCorners();
	void AddHorizontalFrictionOnGround();
	void AddHorizontalAndVerticalFrictionWhileFlying();
	float RaycastDistanceCanTravel(const Vector3& startingPosition, const Vector3& directionTotravel);
};