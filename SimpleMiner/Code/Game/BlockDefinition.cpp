#include "Game/BlockDefinition.hpp"

BlockDefinition::BlockDefinition()
	:m_blockType(AIR)
	, m_tint((unsigned char)255, (unsigned char)255, (unsigned char)255, (unsigned char)255)
	, m_positionInChunk(0,0,0)
	, m_top(Face())
	, m_bottom(Face())
	, m_front(Face())
	, m_back(Face())
	, m_left(Face())
	, m_right(Face())
{
}

BlockDefinition::BlockDefinition(unsigned char defIndex)
	:m_blockType(static_cast<BlockType>(defIndex))
{
	GenerateBlockInformationBasedOnType(m_blockType);
}

BlockDefinition::~BlockDefinition()
{
}

void BlockDefinition::GenerateBlockInformationBasedOnType(BlockType blockType)
{
	static SpriteSheet* blockAtlas = new SpriteSheet("Data/Images/SimpleMinerAtlas.png", 16, 16);

	if (blockType == AIR)
		SetBlockAsAir(blockAtlas);

	if (blockType == GRASS)
		SetBlockAsGrass(blockAtlas);

	if (blockType == DIRT)
		SetBlockAsDirt(blockAtlas);

	if (blockType == STONE)
		SetBlockAsStone(blockAtlas);

	if (blockType == SAND)
		SetBlockAsSand(blockAtlas);

	if (blockType == WATER)
		SetBlockAsWater(blockAtlas);

	if(blockType == GLOWSTONE)
		SetBlockAsGlowstone(blockAtlas);

	if (blockType == WOOD)
		SetBlockAsWood(blockAtlas);

	if (blockType == BRICK)
		SetBlockAsBrick(blockAtlas);

	if (blockType == STONE_BRICK)
		SetBlockAsStoneBrick(blockAtlas);

	if (blockType == MOSS_STONE_BRICK)
		SetBlockAsMossStoneBrick(blockAtlas);

	if (blockType == HOOKSHOT)
		SetBlockAsGraple(blockAtlas);
}

void BlockDefinition::SetBlockAsGraple(SpriteSheet* blockAtlas)
{
	AABB2D grapleBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(0,7));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, grapleBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, grapleBox);
	m_front = SetFrontFace(spriteSheet, m_tint, grapleBox);
	m_back = SetBackFace(spriteSheet, m_tint, grapleBox);
	m_left = SetLeftFace(spriteSheet, m_tint, grapleBox);
	m_right = SetRightFace(spriteSheet, m_tint, grapleBox);
}


void BlockDefinition::SetBlockAsMossStoneBrick(SpriteSheet* blockAtlas)
{
	AABB2D stoneMossBrickBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(7, 10));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, stoneMossBrickBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, stoneMossBrickBox);
	m_front = SetFrontFace(spriteSheet, m_tint, stoneMossBrickBox);
	m_back = SetBackFace(spriteSheet, m_tint, stoneMossBrickBox);
	m_left = SetLeftFace(spriteSheet, m_tint, stoneMossBrickBox);
	m_right = SetRightFace(spriteSheet, m_tint, stoneMossBrickBox);
}

void BlockDefinition::SetBlockAsStoneBrick(SpriteSheet* blockAtlas)
{
	AABB2D stoneBrickBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(8, 10));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, stoneBrickBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, stoneBrickBox);
	m_front = SetFrontFace(spriteSheet, m_tint, stoneBrickBox);
	m_back = SetBackFace(spriteSheet, m_tint, stoneBrickBox);
	m_left = SetLeftFace(spriteSheet, m_tint, stoneBrickBox);
	m_right = SetRightFace(spriteSheet, m_tint, stoneBrickBox);
}


void BlockDefinition::SetBlockAsBrick(SpriteSheet* blockAtlas)
{
	AABB2D brickBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(3, 11));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, brickBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, brickBox);
	m_front = SetFrontFace(spriteSheet, m_tint, brickBox);
	m_back = SetBackFace(spriteSheet, m_tint, brickBox);
	m_left = SetLeftFace(spriteSheet, m_tint, brickBox);
	m_right = SetRightFace(spriteSheet, m_tint, brickBox);
}

void BlockDefinition::SetBlockAsWood(SpriteSheet* blockAtlas)
{
	AABB2D woodBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(13, 8));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, woodBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, woodBox);
	m_front = SetFrontFace(spriteSheet, m_tint, woodBox);
	m_back = SetBackFace(spriteSheet, m_tint, woodBox);
	m_left = SetLeftFace(spriteSheet, m_tint, woodBox);
	m_right = SetRightFace(spriteSheet, m_tint, woodBox);
}

void BlockDefinition::SetBlockAsAir(SpriteSheet* blockAtlas)
{
	AABB2D airBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(2, 5));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, airBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, airBox);
	m_front = SetFrontFace(spriteSheet, m_tint, airBox);
	m_back = SetBackFace(spriteSheet, m_tint, airBox);
	m_left = SetLeftFace(spriteSheet, m_tint, airBox);
	m_right = SetRightFace(spriteSheet, m_tint, airBox);
}

void BlockDefinition::SetBlockAsGrass(SpriteSheet* blockAtlas)
{
	AABB2D dirtBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(7, 8));
	AABB2D dirtWithGrassBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(8, 8));
	AABB2D grassBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(9, 8));

	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, grassBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, dirtBox);
	m_front = SetFrontFace(spriteSheet, m_tint, dirtWithGrassBox);
	m_back = SetBackFace(spriteSheet, m_tint, dirtWithGrassBox);
	m_left = SetLeftFace(spriteSheet, m_tint, dirtWithGrassBox);
	m_right = SetRightFace(spriteSheet, m_tint, dirtWithGrassBox);
}

void BlockDefinition::SetBlockAsDirt(SpriteSheet* blockAtlas)
{
	AABB2D dirtBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(7, 8));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, dirtBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, dirtBox);
	m_front = SetFrontFace(spriteSheet, m_tint, dirtBox);
	m_back = SetBackFace(spriteSheet, m_tint, dirtBox);
	m_left = SetLeftFace(spriteSheet, m_tint, dirtBox);
	m_right = SetRightFace(spriteSheet, m_tint, dirtBox);
}

void BlockDefinition::SetBlockAsStone(SpriteSheet* blockAtlas)
{
	AABB2D stoneBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(2, 10));
	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, stoneBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, stoneBox);
	m_front = SetFrontFace(spriteSheet, m_tint, stoneBox);
	m_back = SetBackFace(spriteSheet, m_tint, stoneBox);
	m_left = SetLeftFace(spriteSheet, m_tint, stoneBox);
	m_right = SetRightFace(spriteSheet, m_tint, stoneBox);
}

void BlockDefinition::SetBlockAsSand(SpriteSheet* blockAtlas)
{
	AABB2D sandBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(1, 8));

	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, sandBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, sandBox);
	m_front = SetFrontFace(spriteSheet, m_tint, sandBox);
	m_back = SetBackFace(spriteSheet, m_tint, sandBox);
	m_left = SetLeftFace(spriteSheet, m_tint, sandBox);
	m_right = SetRightFace(spriteSheet, m_tint, sandBox);
}

void BlockDefinition::SetBlockAsWater(SpriteSheet* blockAtlas)
{
	AABB2D waterBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(6, 0));

	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, waterBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, waterBox);
	m_front = SetFrontFace(spriteSheet, m_tint, waterBox);
	m_back = SetBackFace(spriteSheet, m_tint, waterBox);
	m_left = SetLeftFace(spriteSheet, m_tint, waterBox);
	m_right = SetRightFace(spriteSheet, m_tint, waterBox);
}

void BlockDefinition::SetBlockAsGlowstone(SpriteSheet* blockAtlas)
{
	AABB2D glowStoneBox = blockAtlas->GetTexCoordsForSpriteCoords(IntVector2(4, 11));

	Texture* spriteSheet = blockAtlas->GetSpritesheetTexture();

	m_top = SetTopFace(spriteSheet, m_tint, glowStoneBox);
	m_bottom = SetBottomFace(spriteSheet, m_tint, glowStoneBox);
	m_front = SetFrontFace(spriteSheet, m_tint, glowStoneBox);
	m_back = SetBackFace(spriteSheet, m_tint, glowStoneBox);
	m_left = SetLeftFace(spriteSheet, m_tint, glowStoneBox);
	m_right = SetRightFace(spriteSheet, m_tint, glowStoneBox);
}

Face BlockDefinition::SetTopFace(Texture* texture, Rgba& color, AABB2D& texBox)
{
	Face top;
	top.m_faceTexture = texture;
	top.m_vOne = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.mins.x, texBox.maxs.y));
	top.m_vTwo = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.mins.x, texBox.mins.y));
	top.m_vThree = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.mins.y));
	top.m_vFour = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.maxs.y));
	return top;
}

Face BlockDefinition::SetBottomFace(Texture* texture, Rgba& color, AABB2D& texBox)
{
	Face bottom;
	bottom.m_faceTexture = texture;
	bottom.m_vOne = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.maxs.y));
	bottom.m_vTwo = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.mins.y));
	bottom.m_vThree = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y, (float)m_positionInChunk.z), color, Vector2(texBox.maxs.x, texBox.mins.y));
	bottom.m_vFour = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z), color, Vector2(texBox.maxs.x, texBox.maxs.y));
	return bottom;
}

Face BlockDefinition::SetFrontFace(Texture* texture, Rgba& color, AABB2D& texBox)
{
	Face front;
	front.m_faceTexture = texture;
	front.m_vOne = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.maxs.y));
	front.m_vTwo = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.mins.y));
	front.m_vThree = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.mins.y));
	front.m_vFour = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.maxs.y));
	return front;
}

Face BlockDefinition::SetBackFace(Texture* texture, Rgba& color, AABB2D& texBox)
{
	Face back;
	back.m_faceTexture = texture;
	back.m_vOne = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.maxs.y));
	back.m_vTwo = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.mins.y));
	back.m_vThree = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.mins.y));
	back.m_vFour = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.maxs.y));
	return back;
}

Face BlockDefinition::SetLeftFace(Texture* texture, Rgba& color, AABB2D& texBox)
{
	Face left;
	left.m_faceTexture = texture;
	left.m_vOne = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.maxs.y));
	left.m_vTwo = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.mins.y));
	left.m_vThree = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.mins.y));
	left.m_vFour = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y + 1.f, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.maxs.y));
	return left;
}

Face BlockDefinition::SetRightFace(Texture* texture, Rgba& color, AABB2D& texBox)
{
	Face right;
	right.m_faceTexture = texture;
	right.m_vOne = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.maxs.y));
	right.m_vTwo = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y, (float)m_positionInChunk.z), color, Vector2(texBox.mins.x, texBox.mins.y));
	right.m_vThree = Vertex3_PCT(Vector3((float)m_positionInChunk.x + 1.f, (float)m_positionInChunk.y, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.mins.y));
	right.m_vFour = Vertex3_PCT(Vector3((float)m_positionInChunk.x, (float)m_positionInChunk.y, (float)m_positionInChunk.z + 1.f), color, Vector2(texBox.maxs.x, texBox.maxs.y));
	return right;
}

unsigned char GetLightAndFlagsForBlockType(BlockType blockType)
{
	if (blockType == AIR)
		return 0b01000000;

	if (blockType == GRASS)
		return 0b01110000;

	if (blockType == DIRT)
		return 0b01110000;

	if (blockType == WATER)
		return 0b01100000;

	if (blockType == STONE)
		return 0b01110000;

	if (blockType == SAND)
		return 0b01110000;

	if (blockType == GLOWSTONE)
		return 0b01111100;

	if (blockType == WOOD)
		return 0b01110000;

	if (blockType == BRICK)
		return 0b01110000;

	if (blockType == STONE_BRICK)
		return 0b01110000;

	if (blockType == MOSS_STONE_BRICK)
		return 0b01110000;
	
	return 0b01110000;
}