#include "Engine/UI/UICanvas.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"


UICanvas::UICanvas()
	:m_texture(nullptr)
{
	m_tint = Rgba(0, 0, 0, 0);
}

UICanvas::~UICanvas()
{

}

void UICanvas::SetTexture(Texture2D* tex)
{
	m_texture = tex;
}

void UICanvas::SetTexture(const char* file_path)
{
	m_texture = CreateOrGetTexture2D(file_path, g_simpleRenderer, file_path);
}

void UICanvas::SetTargetResolution(float target)
{
	float aspect_ratio = (float)DEFAULT_WINDOW_WIDTH / (float)DEFAULT_WINDOW_HEIGHT;

	if (aspect_ratio > 1.0f)
	{
		m_resolution = Vector2(aspect_ratio * target, target);
	}
	else
	{
		m_resolution = Vector2(target, (1.0f / aspect_ratio) * target);
	}

	SetPivot(Vector2(0.0f, 0.0f));
	SetSize(m_resolution);
}

void UICanvas::Render()
{
	if (!g_simpleRenderer)
		return;

	g_simpleRenderer->SetOrthoProjection(Vector2(0.0f, 0.0f), m_resolution);
	g_simpleRenderer->EnableBlend(BLEND_SRC_ALPHA, BLEND_INV_SRC_ALPHA);
	g_simpleRenderer->SetSampler(m_sampler);
	g_simpleRenderer->SetShaderProgram(m_shader);
	g_simpleRenderer->MakeModelMatrixIdentity();

	Matrix4 transform = GetWorldTransform();
	g_simpleRenderer->SetModelMatrix(transform);

	#if defined(_DEBUG)
		DebugRender();
	#endif


	if (m_texture)
	{
		g_simpleRenderer->Draw2DQuad(m_bounds.CalcCenter(), m_bounds.GetExtends(), m_texture, m_tint);
	}

	for (UIElement* child : m_children)
	{
		child->Render();
	}

	g_simpleRenderer->DisableBlend();
}
