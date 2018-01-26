#include "Game/Chunk.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Render/Renderer.hpp"
#include "Game/BlockDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Noise.hpp"
#include "Game/BlockInfo.hpp"

Chunk::Chunk( const IntVector2& chunkCoords )
	:m_worldBounds( 0.f, 0.f, 0.f, 0.f, 0.f, 0.f )
	, m_chunkCoords(chunkCoords)
	, m_isVertexArrayDirty(true)
	, m_northNeighbor(nullptr)
	, m_eastNeighbor(nullptr)
	, m_southNeighbor(nullptr)
	, m_westNeighbor(nullptr)
	, m_vboID(0)
{

	m_worldBounds.mins.x = (float)chunkCoords.x * (float)CHUNK_WIDTH_X;
	m_worldBounds.mins.y = (float)chunkCoords.y * (float)CHUNK_DEPTH_Y;
	m_worldBounds.mins.z = 0.f;
	m_worldBounds.maxs.x = m_worldBounds.mins.x + (float)CHUNK_WIDTH_X;
	m_worldBounds.maxs.y = m_worldBounds.mins.y + (float)CHUNK_DEPTH_Y;
	m_worldBounds.maxs.z = m_worldBounds.mins.z + (float)CHUNK_HEIGHT_Z;

	memset(&m_blocks[0], 0, sizeof(m_blocks));
	
	m_vboID = g_myRenderer->CreateVBOID();

}

Chunk::~Chunk()
{
	g_myRenderer->DestroyVBO(m_vboID);
}

void Chunk::Update()
{
	if(m_isVertexArrayDirty)
	{	
		PopulateVertexArray();
		m_isVertexArrayDirty = false;
	}
}

void Chunk::Render() const
{
	g_myRenderer->StartManipulatingTheDrawnObject();
	g_myRenderer->TranslateDrawing3D(m_worldBounds.mins);
	if(m_numVertexes > 0)
		g_myRenderer->DrawVBO3D_PCT(m_vboID, m_numVertexes, PRIMITIVE_QUADS);
	g_myRenderer->EndManipulationOfDrawing();
}

void Chunk::GenerateChunk()
{
	for (int z = 0; z < CHUNK_HEIGHT_Z; z++)
	{
		for (int y = 0; y < CHUNK_DEPTH_Y; y++)
		{
			for (int x = 0; x < CHUNK_WIDTH_X; x++)
			{
				int blockIndex = GetBlockIndexForLocalCoords(IntVector3(x, y, z));
				GenerateBlock(blockIndex);
			}
		}
	}
	CreateSandBlocks();
}

void Chunk::GenerateBlock(int blockIndex)
{
	const float MESA_RARE = 0.35f;
	const float CANYON_RARE = 0.25f;

 	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
 
 	Vector3 blockWorldMins = Vector3(m_worldBounds.mins.x + (float)blockCoords.x, m_worldBounds.mins.y + (float)blockCoords.y, m_worldBounds.mins.z);
	float variance = 10.f * Compute2dPerlinNoise(blockWorldMins.x, blockWorldMins.y, 60.f, 5, 0.3f, 2.f, true, 0);
	float mesaNess = Compute2dPerlinNoise(blockWorldMins.x, blockWorldMins.y, 100.f, 3, 0.6f, 2.f, true, 0);
	int groundHeight = SEA_LEVEL_HEIGHT + (int)variance + 20;

	if (mesaNess > MESA_RARE)
	{
		float mesaHeight = RangeMapFloat(MESA_RARE, 1.f, 0.f, 1.f, mesaNess);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		groundHeight += 20.f * (mesaHeight);
	}
	else if (mesaNess < -CANYON_RARE)
	{
		float mesaHeight = RangeMapFloat(CANYON_RARE, 1.f, 0.f, 1.f, -mesaNess);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		mesaHeight = SmoothStop(mesaHeight);
		groundHeight -= 20.f * (mesaHeight);
	}

	int airMinHeight = groundHeight + 1;
	int grassMinHeight = groundHeight;
	int dirtMinHeight = groundHeight - 4;

	if (blockCoords.z >= airMinHeight)
	{
		if (blockCoords.z <= SEA_LEVEL_HEIGHT)
		{
			m_blocks[blockIndex] = Block(WATER, 0b01000000);
		}
		else
		{
			m_blocks[blockIndex] = Block(AIR, 0b01000000);
		} 
	}
	else if (blockCoords.z >= grassMinHeight)
	{
		m_blocks[blockIndex] = Block(GRASS, 0b01110000); 
	}
	else if (blockCoords.z >= dirtMinHeight)
	{
		m_blocks[blockIndex] = Block(DIRT, 0b01110000);
	}
	else 
	{
		m_blocks[blockIndex] = Block(STONE, 0b01110000);
	}

	if (groundHeight == SEA_LEVEL_HEIGHT)
	{
		BlockDefinition blockDef = BlockDefinition(m_blocks[blockIndex].m_blockTypeIndex);
		if (blockDef.m_blockType == DIRT || blockDef.m_blockType == GRASS)
		{
			m_blocks[blockIndex] = Block(SAND, 0b01110000);
		}
	}
}

void Chunk::CreateSandBlocks()
{
	for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
		if (blockCoords.z > SEA_LEVEL_HEIGHT)
			return;

		int blockAbove = GetBlockIndexForLocalCoords(blockCoords + IntVector3(0, 0, 1));
		BlockType currentType = BlockDefinition(m_blocks[blockIndex].m_blockTypeIndex).m_blockType;
		BlockType blockAboveType = BlockDefinition(m_blocks[blockAbove].m_blockTypeIndex).m_blockType;

		if ((currentType == GRASS || currentType == DIRT) && blockAboveType == WATER)
			m_blocks[blockIndex] = Block(SAND, 0b01110000);
	}
}

int Chunk::GetBlockInFrontIndex(int blockIndex)
{
	int frontIndex;
	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
	if (blockCoords.x + 1 > CHUNK_WIDTH_X)
	{
		frontIndex = blockIndex;
	}
	else
	{
		blockCoords.x += 1;
		frontIndex = GetBlockIndexForLocalCoords(blockCoords);
	}
	return frontIndex;
}

int Chunk::GetBlockBehindIndex(int blockIndex)
{
	int behindIndex;
	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
	if (blockCoords.x - 1 < 0)
	{
		behindIndex = blockIndex;
	}
	else
	{
		blockCoords.x -= 1;
		behindIndex = GetBlockIndexForLocalCoords(blockCoords);
	}
	return behindIndex;
}

int Chunk::GetBlockToLeftIndex(int blockIndex)
{
	int leftIndex;
	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
	if (blockCoords.y + 1 > CHUNK_DEPTH_Y)
	{
		leftIndex = blockIndex;
	}
	else
	{
		blockCoords.y += 1;
		leftIndex = GetBlockIndexForLocalCoords(blockCoords);
	}
	return leftIndex;
}

int Chunk::GetBlockToRightIndex(int blockIndex)
{
	int rightIndex;
	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
	if (blockCoords.y - 1 < 0)
	{
		rightIndex = blockIndex;
	}
	else
	{
		blockCoords.y -= 1;
		rightIndex = GetBlockIndexForLocalCoords(blockCoords);
	}
	return rightIndex;
}

int Chunk::GetBlockBelowIndex(int blockIndex)
{
	int belowIndex;
	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
	if (blockCoords.z - 1 < 0)
	{
		belowIndex = blockIndex;
	}
	else
	{
		blockCoords.z -= 1;
		belowIndex = GetBlockIndexForLocalCoords(blockCoords);
	}
	return belowIndex;
}

int Chunk::GetBlockAboveIndex(int blockIndex)
{
	int aboveIndex;
	IntVector3 blockCoords = GetBlockCoordsForIndex(blockIndex);
	if (blockCoords.z + 1 > CHUNK_HEIGHT_Z)
	{
		aboveIndex = blockIndex;
	}
	else
	{
		blockCoords.z += 1;
		aboveIndex = GetBlockIndexForLocalCoords(blockCoords);
	}
	return aboveIndex;
}

int Chunk::GetBlockIndexForLocalCoords(const IntVector3& blockCoords)
{
	return blockCoords.x | (blockCoords.y << CHUNK_BITS_Y) | (blockCoords.z << CHUNK_BITS_XY);
}

IntVector3 Chunk::GetBlockCoordsForIndex(int blockIndex)
{
	int x = blockIndex & CHUNK_X_MASK;
	int y = (blockIndex & CHUNK_Y_MASK) >> CHUNK_BITS_X;
	int z = (blockIndex & CHUNK_Z_MASK) >> CHUNK_BITS_XY;
	return IntVector3(x, y, z);
}

void Chunk::PopulateVertexArray()
{
	std::vector<Vertex3_PCT> vertexes;
	for (int blockIndex = 0; blockIndex < NUM_BLOCKS_PER_CHUNK; ++blockIndex)
	{
		AddBlockVertexes(blockIndex, vertexes);
	}
	g_myRenderer->UpdateVBO(m_vboID, &vertexes[0], vertexes.size());
	m_numVertexes = vertexes.size();
}

void Chunk::DirtyNeighbors()
{
	if (m_northNeighbor != nullptr)
		m_northNeighbor->m_isVertexArrayDirty = true;

	if (m_southNeighbor != nullptr)
		m_southNeighbor->m_isVertexArrayDirty = true;

	if (m_westNeighbor != nullptr)
		m_westNeighbor->m_isVertexArrayDirty = true;

	if (m_eastNeighbor != nullptr)
		m_eastNeighbor->m_isVertexArrayDirty = true;
}

Rgba Chunk::GetVertexColorForLightLevel(int lightLevel)
{
	unsigned char lightColorBytes[MAX_LEVEL] = { 20, 35, 50, 70, 95, 115, 128, 142, 160, 178, 192, 210, 224, 235, 255 };
	unsigned char colorByte = lightColorBytes[lightLevel - 1];
	return Rgba(colorByte, colorByte, colorByte, 255);
}

void Chunk::AddBlockVertexes(int blockIndex, std::vector<Vertex3_PCT>& vertexes)
{
	Block& block = m_blocks[blockIndex];
	if (block.m_blockTypeIndex == AIR)
		return;

	BlockDefinition blockDef(block.m_blockTypeIndex);
	IntVector3 blockLocalCoords = GetBlockCoordsForIndex(blockIndex);
	Vector3 blockLocalMins((float)blockLocalCoords.x, (float)blockLocalCoords.y, (float)blockLocalCoords.z);
	Vector3 blockLocalMaxs = blockLocalMins + Vector3(1.f, 1.f, 1.f);
	BlockInfo currentBlockInfo(this, blockIndex);

	BlockInfo blockAbove = currentBlockInfo.GetTopNeighbor();
	if(!blockAbove.IsBlockOpaque() && blockAbove.m_chunk != nullptr)
	{
		Face top;
		top.m_vOne.m_position = Vector3(blockLocalMins.x, blockLocalMins.y, blockLocalMaxs.z);
		top.m_vTwo.m_position = Vector3(blockLocalMaxs.x, blockLocalMins.y, blockLocalMaxs.z);
		top.m_vThree.m_position = Vector3(blockLocalMaxs.x, blockLocalMaxs.y, blockLocalMaxs.z);
		top.m_vFour.m_position = Vector3(blockLocalMins.x, blockLocalMaxs.y, blockLocalMaxs.z);
		top.m_vOne.m_color = GetVertexColorForLightLevel(MAX_LEVEL);								// (int)blockAbove.GetLightValueForBlock()
		top.m_vTwo.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		top.m_vThree.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		top.m_vFour.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		top.m_vOne.m_texCoords = blockDef.m_top.m_vOne.m_texCoords;
		top.m_vTwo.m_texCoords = blockDef.m_top.m_vTwo.m_texCoords;
		top.m_vThree.m_texCoords = blockDef.m_top.m_vThree.m_texCoords;
		top.m_vFour.m_texCoords = blockDef.m_top.m_vFour.m_texCoords;

		vertexes.push_back(top.m_vOne);
		vertexes.push_back(top.m_vTwo);
		vertexes.push_back(top.m_vThree);
		vertexes.push_back(top.m_vFour);
	}

	BlockInfo blockBelow = currentBlockInfo.GetBottomNeighbor();
	if (!blockBelow.IsBlockOpaque() && blockBelow.m_chunk != nullptr)
	{
		Face bottom;
		bottom.m_vOne.m_position = Vector3(blockLocalMaxs.x, blockLocalMins.y, blockLocalMins.z);
		bottom.m_vTwo.m_position = Vector3(blockLocalMins.x, blockLocalMins.y, blockLocalMins.z);
		bottom.m_vThree.m_position = Vector3(blockLocalMins.x, blockLocalMaxs.y, blockLocalMins.z);
		bottom.m_vFour.m_position = Vector3(blockLocalMaxs.x, blockLocalMaxs.y, blockLocalMins.z);
		bottom.m_vOne.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		bottom.m_vTwo.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		bottom.m_vThree.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		bottom.m_vFour.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		bottom.m_vOne.m_texCoords = blockDef.m_bottom.m_vOne.m_texCoords;
		bottom.m_vTwo.m_texCoords = blockDef.m_bottom.m_vTwo.m_texCoords;
		bottom.m_vThree.m_texCoords = blockDef.m_bottom.m_vThree.m_texCoords;
		bottom.m_vFour.m_texCoords = blockDef.m_bottom.m_vFour.m_texCoords;

		vertexes.push_back(bottom.m_vOne);
		vertexes.push_back(bottom.m_vTwo);
		vertexes.push_back(bottom.m_vThree);
		vertexes.push_back(bottom.m_vFour);
	}

	BlockInfo blockBehind = currentBlockInfo.GetWestNeighbor();
	if (!blockBehind.IsBlockOpaque() && blockBehind.m_chunk != nullptr)
	{
		Face front;
		front.m_vOne.m_position = Vector3(blockLocalMins.x, blockLocalMins.y, blockLocalMins.z);
		front.m_vTwo.m_position = Vector3(blockLocalMins.x, blockLocalMins.y, blockLocalMaxs.z);
		front.m_vThree.m_position = Vector3(blockLocalMins.x, blockLocalMaxs.y, blockLocalMaxs.z);
		front.m_vFour.m_position = Vector3(blockLocalMins.x, blockLocalMaxs.y, blockLocalMins.z);
		front.m_vOne.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		front.m_vTwo.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		front.m_vThree.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		front.m_vFour.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		front.m_vOne.m_texCoords = blockDef.m_front.m_vOne.m_texCoords;
		front.m_vTwo.m_texCoords = blockDef.m_front.m_vTwo.m_texCoords;
		front.m_vThree.m_texCoords = blockDef.m_front.m_vThree.m_texCoords;
		front.m_vFour.m_texCoords = blockDef.m_front.m_vFour.m_texCoords;

		vertexes.push_back(front.m_vOne);
		vertexes.push_back(front.m_vTwo);
		vertexes.push_back(front.m_vThree);
		vertexes.push_back(front.m_vFour);
	}

	BlockInfo blockInFront = currentBlockInfo.GetEastNeighbor();					
	if (!blockInFront.IsBlockOpaque() && blockInFront.m_chunk != nullptr)
	{
		Face back;
		back.m_vOne.m_position = Vector3(blockLocalMaxs.x, blockLocalMaxs.y, blockLocalMins.z);
		back.m_vTwo.m_position = Vector3(blockLocalMaxs.x, blockLocalMaxs.y, blockLocalMaxs.z);
		back.m_vThree.m_position = Vector3(blockLocalMaxs.x, blockLocalMins.y, blockLocalMaxs.z);
		back.m_vFour.m_position = Vector3(blockLocalMaxs.x, blockLocalMins.y, blockLocalMins.z);
		back.m_vOne.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		back.m_vTwo.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		back.m_vThree.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		back.m_vFour.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		back.m_vOne.m_texCoords = blockDef.m_back.m_vOne.m_texCoords;
		back.m_vTwo.m_texCoords = blockDef.m_back.m_vTwo.m_texCoords;
		back.m_vThree.m_texCoords = blockDef.m_back.m_vThree.m_texCoords;
		back.m_vFour.m_texCoords = blockDef.m_back.m_vFour.m_texCoords;

		vertexes.push_back(back.m_vOne);
		vertexes.push_back(back.m_vTwo);
		vertexes.push_back(back.m_vThree);
		vertexes.push_back(back.m_vFour);
	}

	BlockInfo blockNorth = currentBlockInfo.GetNorthNeighbor();
	if (!blockNorth.IsBlockOpaque() && blockNorth.m_chunk != nullptr)
	{
		Face left;
		left.m_vOne.m_position = Vector3(blockLocalMins.x, blockLocalMaxs.y, blockLocalMins.z);
		left.m_vTwo.m_position = Vector3(blockLocalMins.x, blockLocalMaxs.y, blockLocalMaxs.z);
		left.m_vThree.m_position = Vector3(blockLocalMaxs.x, blockLocalMaxs.y, blockLocalMaxs.z);
		left.m_vFour.m_position = Vector3(blockLocalMaxs.x, blockLocalMaxs.y, blockLocalMins.z);
		left.m_vOne.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		left.m_vTwo.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		left.m_vThree.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		left.m_vFour.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		left.m_vOne.m_texCoords = blockDef.m_left.m_vOne.m_texCoords;
		left.m_vTwo.m_texCoords = blockDef.m_left.m_vTwo.m_texCoords;
		left.m_vThree.m_texCoords = blockDef.m_left.m_vThree.m_texCoords;
		left.m_vFour.m_texCoords = blockDef.m_left.m_vFour.m_texCoords;

		vertexes.push_back(left.m_vOne);
		vertexes.push_back(left.m_vTwo);
		vertexes.push_back(left.m_vThree);
		vertexes.push_back(left.m_vFour);
	}

	BlockInfo blockSouth = currentBlockInfo.GetSouthNeighbor();
	if (!blockSouth.IsBlockOpaque() && blockSouth.m_chunk != nullptr)
	{
		Face right;
		right.m_vOne.m_position = Vector3(blockLocalMaxs.x, blockLocalMins.y, blockLocalMins.z);
		right.m_vTwo.m_position = Vector3(blockLocalMaxs.x, blockLocalMins.y, blockLocalMaxs.z);
		right.m_vThree.m_position = Vector3(blockLocalMins.x, blockLocalMins.y, blockLocalMaxs.z);
		right.m_vFour.m_position = Vector3(blockLocalMins.x, blockLocalMins.y, blockLocalMins.z);
		right.m_vOne.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		right.m_vTwo.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		right.m_vThree.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		right.m_vFour.m_color = GetVertexColorForLightLevel(MAX_LEVEL);
		right.m_vOne.m_texCoords = blockDef.m_right.m_vOne.m_texCoords;
		right.m_vTwo.m_texCoords = blockDef.m_right.m_vTwo.m_texCoords;
		right.m_vThree.m_texCoords = blockDef.m_right.m_vThree.m_texCoords;
		right.m_vFour.m_texCoords = blockDef.m_right.m_vFour.m_texCoords;

		vertexes.push_back(right.m_vOne);
		vertexes.push_back(right.m_vTwo);
		vertexes.push_back(right.m_vThree);
		vertexes.push_back(right.m_vFour);
	}
}

int Chunk::GetBlockIndexForLocalCoords(const IntVector3& blockCoords) const
{
	return blockCoords.x | (blockCoords.y << CHUNK_BITS_Y) | (blockCoords.z << CHUNK_BITS_XY);
}

IntVector3 Chunk::GetBlockCoordsForIndex(int blockIndex) const
{
	int x = blockIndex & CHUNK_X_MASK;
	int y = (blockIndex & CHUNK_Y_MASK) >> CHUNK_BITS_X;
	int z = (blockIndex & CHUNK_Z_MASK) >> CHUNK_BITS_XY;
	return IntVector3(x, y, z);
}

