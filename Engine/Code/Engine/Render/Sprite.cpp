#include "Engine/Render/Sprite.hpp"
#include "Engine/RHI/Texture2D.hpp"

Sprite::Sprite()
	: m_texture(nullptr)
	, m_bounds(AABB2D(0.0f, 0.0f, 1.0f, 1.0f))
{

}

Sprite::~Sprite()
{

}

void Sprite::SetTexture(Texture2D* tex)
{
	m_texture = tex;
}

void Sprite::SetBounds(const AABB2D& bounds)
{
	m_bounds = bounds;
}
