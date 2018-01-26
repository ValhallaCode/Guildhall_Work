#pragma once
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/IntVector3.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Render/Rgba.hpp"
#include "Engine/Render/SpriteSheet.hpp"
#include "Game/Face.hpp"


enum BlockType {
	AIR,
	GRASS,
	DIRT,
	STONE,
	SAND,
	WATER,
	GLOWSTONE,
	WOOD,
	BRICK,
	STONE_BRICK,
	MOSS_STONE_BRICK,
	HOOKSHOT,
	NUM_BLOCKS
};


class BlockDefinition
{
public:
	IntVector3 m_positionInChunk;
	Rgba m_tint;
	BlockType m_blockType;
	Face m_top;
	Face m_bottom;
	Face m_front;
	Face m_back;
	Face m_left;
	Face m_right;

	BlockDefinition();
	BlockDefinition(unsigned char defIndex);
	~BlockDefinition();
	void GenerateBlockInformationBasedOnType(BlockType blockType);
	void SetBlockAsGraple(SpriteSheet* blockAtlas);
	void SetBlockAsMossStoneBrick(SpriteSheet* blockAtlas);
	void SetBlockAsStoneBrick(SpriteSheet* blockAtlas);
	void SetBlockAsBrick(SpriteSheet* blockAtlas);
	void SetBlockAsWood(SpriteSheet* blockAtlas);
	void SetBlockAsAir(SpriteSheet* blockAtlas);
	void SetBlockAsGrass(SpriteSheet* blockAtlas);
	void SetBlockAsDirt(SpriteSheet* blockAtlas);
	void SetBlockAsStone(SpriteSheet* blockAtlas);
	void SetBlockAsSand(SpriteSheet* blockAtlas);
	void SetBlockAsWater(SpriteSheet* blockAtlas);
	void SetBlockAsGlowstone(SpriteSheet* blockAtlas);
	Face SetTopFace(Texture* texture, Rgba& color, AABB2D& texBox);
	Face SetBottomFace(Texture* texture, Rgba& color, AABB2D& texBox);
	Face SetFrontFace(Texture* texture, Rgba& color, AABB2D& texBox);
	Face SetBackFace(Texture* texture, Rgba& color, AABB2D& texBox);
	Face SetLeftFace(Texture* texture, Rgba& color, AABB2D& texBox);
	Face SetRightFace(Texture* texture, Rgba& color, AABB2D& texBox);
	friend unsigned char GetLightAndFlagsForBlockType(BlockType blockType);
};

unsigned char GetLightAndFlagsForBlockType(BlockType blockType);