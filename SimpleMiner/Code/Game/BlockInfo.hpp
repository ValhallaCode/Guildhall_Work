#pragma once
#include "Game/Chunk.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/Block.hpp"



class BlockInfo
{
public:
	Chunk* m_chunk;
	int m_blockIndex;

	BlockInfo(Chunk* chunk, int blockIndex);
	~BlockInfo();
	BlockInfo GetEastNeighbor();
	BlockInfo GetWestNeighbor();
	BlockInfo GetNorthNeighbor();
	BlockInfo GetSouthNeighbor();
	BlockInfo GetTopNeighbor();
	BlockInfo GetBottomNeighbor();
	bool IsBlockOpaque();
	bool IsBlockSkyBlock();
	bool IsBlockSolid();
	bool IsBlockDirty();
	unsigned char GetLightValueForBlock();
	void SetLightValueForBlock(unsigned char new_LightValue);
	void SetDirtyFlagAsTrue();
	void SetDirtyFlagAsFalse();
	void SetSkyFlagAsTrue();
	void SetSkyFlagAsFalse();
	void SetSolidFlagAsTrue();
	void SetSolidFlagAsFalse();
	void SetOpaqueFlagAsTrue();
	void SetOpaqueFlagAsFalse();
	BlockType GetBlockType();
	Vector3 GetBlockWorldPosition();
	Block* GetBlockPointer();
	void SetAllNeighborsDirtyFlagAsTrue();
	BlockDefinition GetBlockDefinition();
	IntVector3 GetBlockLocalPosition();
};