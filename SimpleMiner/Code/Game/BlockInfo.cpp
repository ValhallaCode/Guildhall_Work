#include "Game/BlockInfo.hpp"
#include "Game/GameCommons.hpp"

BlockInfo::BlockInfo(Chunk* chunk, int blockIndex)
	:m_chunk(chunk)
	, m_blockIndex(blockIndex)
{
}

BlockInfo::~BlockInfo()
{
}

BlockInfo BlockInfo::GetEastNeighbor() //+X Direction
{
	if ((m_blockIndex & CHUNK_X_MASK) == CHUNK_X_MASK)
	{
		return BlockInfo(m_chunk->m_eastNeighbor, m_blockIndex & ~CHUNK_X_MASK);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex + 1);
	}
}

BlockInfo BlockInfo::GetWestNeighbor() //-X Direction
{
	if ((m_blockIndex & ~CHUNK_X_MASK) == m_blockIndex)
	{
		return BlockInfo(m_chunk->m_westNeighbor, m_blockIndex | CHUNK_X_MASK);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex - 1);
	}
}

BlockInfo BlockInfo::GetNorthNeighbor() //+Y Direction
{
	if ((m_blockIndex & CHUNK_Y_MASK) == CHUNK_Y_MASK)
	{
		return BlockInfo(m_chunk->m_northNeighbor, m_blockIndex & ~CHUNK_Y_MASK);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex + CHUNK_WIDTH_X);
	}
}

BlockInfo BlockInfo::GetSouthNeighbor() //-Y Direction
{
	if ((m_blockIndex & ~CHUNK_Y_MASK) == m_blockIndex)
	{
		return BlockInfo(m_chunk->m_southNeighbor, m_blockIndex | CHUNK_Y_MASK);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex - CHUNK_WIDTH_X);
	}
}

BlockInfo BlockInfo::GetTopNeighbor() //+Z Direction
{
	if ((m_blockIndex & CHUNK_Z_MASK) == CHUNK_Z_MASK)
	{
		return BlockInfo(nullptr, 0);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex + BLOCKS_PER_LAYER);
	}
}

BlockInfo BlockInfo::GetBottomNeighbor() //-Z Direction
{
	if ((m_blockIndex & ~CHUNK_Z_MASK) == m_blockIndex)
	{
		return BlockInfo(nullptr, 0);
	}
	else
	{
		return BlockInfo(m_chunk, m_blockIndex - BLOCKS_PER_LAYER);
	}
}

bool BlockInfo::IsBlockOpaque() // 0b00X00000
{
	if (m_chunk == nullptr)
		return false;

	unsigned char lightAndFlags = m_chunk->m_blocks[m_blockIndex].m_lightAndFlags;
	return (lightAndFlags & BLOCK_OPAQUE_MASK) == BLOCK_OPAQUE_MASK;
}

bool BlockInfo::IsBlockSkyBlock() // 0bX0000000
{
	if (m_chunk == nullptr)
		return false;

	unsigned char lightAndFlags = m_chunk->m_blocks[m_blockIndex].m_lightAndFlags;
	return (lightAndFlags & BLOCK_SKY_MASK) == BLOCK_SKY_MASK;
}

bool BlockInfo::IsBlockSolid() // 0b000X0000
{
	if (m_chunk == nullptr)
		return false;

	unsigned char lightAndFlags = m_chunk->m_blocks[m_blockIndex].m_lightAndFlags;
	return (lightAndFlags & BLOCK_SOLID_MASK) == BLOCK_SOLID_MASK;
}

bool BlockInfo::IsBlockDirty() // 0b0X000000
{
	if (m_chunk == nullptr)
		return false;

	unsigned char lightAndFlags = m_chunk->m_blocks[m_blockIndex].m_lightAndFlags;
	return (lightAndFlags & BLOCK_DIRTY_MASK) == BLOCK_DIRTY_MASK;
}

unsigned char BlockInfo::GetLightValueForBlock()
{
	if (m_chunk == nullptr)
		return 0;

	return m_chunk->m_blocks[m_blockIndex].m_lightAndFlags & BLOCK_LIGHT_MASK;
}

void BlockInfo::SetLightValueForBlock(unsigned char new_LightValue)
{
	unsigned char valueToAdd = new_LightValue & BLOCK_LIGHT_MASK;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags |= valueToAdd;
}

void BlockInfo::SetDirtyFlagAsTrue()
{
	if (IsBlockDirty() || m_chunk == nullptr)
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags |= BLOCK_DIRTY_MASK;
}

void BlockInfo::SetDirtyFlagAsFalse()
{
	if (!IsBlockDirty())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags &= ~BLOCK_DIRTY_MASK;
}

void BlockInfo::SetSkyFlagAsTrue()
{
	if (IsBlockSkyBlock())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags |= BLOCK_SKY_MASK;
}

void BlockInfo::SetSkyFlagAsFalse()
{
	if (!IsBlockSkyBlock())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags &= ~BLOCK_SKY_MASK;
}

void BlockInfo::SetSolidFlagAsTrue()
{
	if (IsBlockSolid())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags |= BLOCK_SOLID_MASK;
}

void BlockInfo::SetSolidFlagAsFalse()
{
	if (!IsBlockSolid())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags &= ~BLOCK_SOLID_MASK;
}

void BlockInfo::SetOpaqueFlagAsTrue()
{
	if (IsBlockOpaque())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags |= BLOCK_OPAQUE_MASK;
}

void BlockInfo::SetOpaqueFlagAsFalse()
{
	if (!IsBlockOpaque())
		return;
	m_chunk->m_blocks[m_blockIndex].m_lightAndFlags &= ~BLOCK_OPAQUE_MASK;
}

BlockType BlockInfo::GetBlockType()
{
	unsigned char blockType = m_chunk->m_blocks[m_blockIndex].m_blockTypeIndex;
	return BlockDefinition(blockType).m_blockType;
}

BlockDefinition BlockInfo::GetBlockDefinition()
{
	unsigned char blockType = m_chunk->m_blocks[m_blockIndex].m_blockTypeIndex;
	return BlockDefinition(blockType);
}

IntVector3 BlockInfo::GetBlockLocalPosition()
{
	return m_chunk->GetBlockCoordsForIndex(m_blockIndex);
}

Vector3 BlockInfo::GetBlockWorldPosition()
{
	IntVector2 chunkTranslate = m_chunk->m_chunkCoords * IntVector2(CHUNK_WIDTH_X, CHUNK_DEPTH_Y);
	Vector3 chunkToWorld((float)chunkTranslate.x, (float)chunkTranslate.y, 0.f);
	IntVector3 blockLocalPos = GetBlockLocalPosition();
	return chunkToWorld + Vector3((float)blockLocalPos.x, (float)blockLocalPos.y, (float)blockLocalPos.z);
}

Block* BlockInfo::GetBlockPointer()
{
	return &m_chunk->m_blocks[m_blockIndex];
}

void BlockInfo::SetAllNeighborsDirtyFlagAsTrue()
{
	BlockInfo northNeighbor = this->GetNorthNeighbor();
	northNeighbor.SetDirtyFlagAsTrue();
	BlockInfo southNeighbor = this->GetSouthNeighbor();
	southNeighbor.SetDirtyFlagAsTrue();
	BlockInfo eastNeighbor = this->GetEastNeighbor();
	eastNeighbor.SetDirtyFlagAsTrue();
	BlockInfo westNeighbor = this->GetWestNeighbor();
	westNeighbor.SetDirtyFlagAsTrue();
	BlockInfo topNeighbor = this->GetTopNeighbor();
	topNeighbor.SetDirtyFlagAsTrue();
	BlockInfo bottomNeighbor = this->GetBottomNeighbor();
	bottomNeighbor.SetDirtyFlagAsTrue();
}