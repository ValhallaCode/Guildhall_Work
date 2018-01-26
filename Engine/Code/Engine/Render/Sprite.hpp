#pragma once
#include "Engine/Math/AABB2D.hpp"

class Texture2D;

class Sprite
{
public:
	Sprite();
	~Sprite();
	void SetTexture(Texture2D* tex);
	void SetBounds(const AABB2D& bounds);
public:
	Texture2D* m_texture;
	AABB2D m_bounds;
};