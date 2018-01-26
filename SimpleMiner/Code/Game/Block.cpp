#include "Game/Block.hpp"
#include "Game/BlockDefinition.hpp"
#include "Engine/Render/Renderer.hpp"


Block::Block(unsigned char typeIndex, unsigned char lightAndFlags)
	:m_blockTypeIndex(typeIndex)
	,m_lightAndFlags(lightAndFlags)
{
}

Block::Block()
{
}

Block::~Block()
{
}


