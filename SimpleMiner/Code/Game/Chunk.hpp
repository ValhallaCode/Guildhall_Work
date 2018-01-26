#pragma once
#include "Engine/Math/AABB3D.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Game/Block.hpp"
#include "Game/GameCommons.hpp"
#include "Engine/Render/Vertex.hpp"
#include <vector>


const int MAX_LEVEL = 15;
const int NUM_BLOCKS_PER_CHUNK = BLOCKS_PER_LAYER * CHUNK_HEIGHT_Z;
const int SEA_LEVEL_HEIGHT = CHUNK_HEIGHT_Z / 4;

class Chunk  
{
public:
	AABB3D m_worldBounds;
	IntVector2 m_chunkCoords;
	Block m_blocks[NUM_BLOCKS_PER_CHUNK];
	unsigned int m_vboID;
	int m_numVertexes;
	bool m_isVertexArrayDirty;
	Chunk* m_northNeighbor;
	Chunk* m_eastNeighbor;
	Chunk* m_southNeighbor;
	Chunk* m_westNeighbor;

	Chunk( const IntVector2& chunkCoords);
	~Chunk();
	void Update();
	void Render() const;
	void GenerateChunk();
	void GenerateBlock(int blockIndex);
	void CreateSandBlocks();
	int GetBlockInFrontIndex(int blockIndex);
	int GetBlockBehindIndex(int blockIndex);
	int GetBlockToLeftIndex(int blockIndex);
	int GetBlockToRightIndex(int blockIndex);
	int GetBlockBelowIndex(int blockIndex);
	int GetBlockAboveIndex(int blockIndex);
	int GetBlockIndexForLocalCoords(const IntVector3& blockCoords);
	IntVector3 GetBlockCoordsForIndex(int blockIndex);
	int GetBlockIndexForLocalCoords(const IntVector3& blockCoords) const;
	IntVector3 GetBlockCoordsForIndex(int blockIndex) const;
	void PopulateVertexArray();
	void DirtyNeighbors();
	Rgba GetVertexColorForLightLevel(int lightLevel);
	void AddBlockVertexes(int blockIndex, std::vector<Vertex3_PCT>& vertexes);
};
