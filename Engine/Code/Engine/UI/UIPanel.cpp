#include "Engine/UI/UIPanel.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"

UIPanel::UIPanel()
{

}

UIPanel::~UIPanel()
{

}

void UIPanel::Render()
{
	g_simpleRenderer->MakeModelMatrixIdentity();
	
	Matrix4 transform = GetWorldTransform();
	g_simpleRenderer->SetModelMatrix(transform);

	g_simpleRenderer->SetShaderProgram(m_shader);
	
	#if defined(_DEBUG)
		DebugRender();
	#endif

	g_simpleRenderer->Draw2DQuad(m_bounds.CalcCenter(), m_bounds.GetExtends(), g_simpleRenderer->m_whiteTexture, m_tint);

	for (UIElement* child : m_children)
	{
		child->Render();
	}
}

