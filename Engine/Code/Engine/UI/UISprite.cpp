#include "Engine/UI/UISprite.hpp"
#include "Engine/Render/Sprite.hpp"
#include "Engine/RHI/Texture2D.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"


UISprite::UISprite()
{
	m_sprite = new Sprite();
}

UISprite::~UISprite()
{

}

void UISprite::SetTexture(Texture2D* tex)
{
	m_sprite->SetTexture(tex);
}

void UISprite::SetTexture(const char* file_path)
{
	m_sprite->m_texture = CreateOrGetTexture2D(file_path, g_simpleRenderer, file_path);
}

void UISprite::SetBounds(const AABB2D& bounds)
{
	m_sprite->SetBounds(bounds);
}

void UISprite::Render()
{
	g_simpleRenderer->MakeModelMatrixIdentity();

	Matrix4 transform = GetWorldTransform();
	g_simpleRenderer->SetModelMatrix(transform);

	g_simpleRenderer->SetShaderProgram(m_shader);

	#if defined(_DEBUG)
		DebugRender();
	#endif

	if (m_sprite->m_texture)
	{
		AABB2D local_bounds = GetBounds();
		g_simpleRenderer->DrawTexturedSprite(local_bounds, *m_sprite->m_texture, m_sprite->m_bounds, m_tint, BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	}

	for (UIElement* child : m_children)
	{
		child->Render();
	}
}
