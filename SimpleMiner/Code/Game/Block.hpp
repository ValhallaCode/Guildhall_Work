#pragma once


class Block
{
public:
	unsigned char m_blockTypeIndex;
	unsigned char m_lightAndFlags;

	Block(unsigned char typeIndex, unsigned char lightAndFlags);
	Block();
	~Block();
};