#pragma once
#include "Engine/Render/Vertex.hpp"
#include "Engine/Render/Texture.hpp"
#include "Engine/Render/Rgba.hpp"



struct Face 
{
	Face()
		:m_faceTexture(nullptr)
		,m_vOne(Vertex3_PCT())
		,m_vTwo(Vertex3_PCT())
		,m_vThree(Vertex3_PCT())
		,m_vFour(Vertex3_PCT())
	{
	};

	Face(Texture* texture, Vertex3_PCT one, Vertex3_PCT two, Vertex3_PCT three, Vertex3_PCT four)
		:m_faceTexture(texture)
		, m_vOne(one)
		, m_vTwo(two)
		, m_vThree(three)
		, m_vFour(four)
	{
	};

	Vertex3_PCT m_vOne;
	Vertex3_PCT m_vTwo;
	Vertex3_PCT m_vThree;
	Vertex3_PCT m_vFour;
	Texture* m_faceTexture;
};