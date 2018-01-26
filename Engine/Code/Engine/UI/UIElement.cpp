#include "Engine/UI/UIElement.hpp"
#include "Engine/EngineConfig.hpp"
#include "Engine/Render/SimpleRenderer.hpp"

UIElement::UIElement()
	:m_parent(nullptr)
	, m_debug(Rgba(255, 255, 255, 255))
	, m_tint(Rgba(255, 255, 255, 255))
	, m_isDebug(false)
{
	m_sampler = new Sampler(g_simpleRenderer->m_device, FILTER_LINEAR, FILTER_LINEAR);
	m_shader = CreateOrGetShaderProgram("Default", "Data/HLSL/nop_textured.hlsl", g_simpleRenderer);
}

UIElement::~UIElement()
{
	delete m_sampler;

	Filicide();
}

void UIElement::SetPosition(Vector2 ratio, Vector2 unit)
{
	m_position.ratio = ratio;
	m_position.unit = unit;

	CalcBoundsAndChildBounds();
}

void UIElement::SetSize(Vector2 size)
{
	m_size.unit = size;

	CalcBoundsAndChildBounds();
}

void UIElement::SetSizeRatio(Vector2 size_ratio)
{
	m_size.ratio = size_ratio;

	CalcBoundsAndChildBounds();
}

void UIElement::SetPivot(Vector2 ratio_in_respect_to_my_bounds)
{
	m_pivot = ratio_in_respect_to_my_bounds;

	CalcBoundsAndChildBounds();
}

UIElement* UIElement::AddChild(UIElement* child)
{
	m_children.push_back(child);
	child->m_parent = this;

	CalcBoundsAndChildBounds();

	return child;
}

void UIElement::RemoveChild(UIElement* child)
{
	for (unsigned int index = 0; index < m_children.size(); ++index)
	{
		if (m_children[index] == child)
		{
			std::swap(m_children.back(), m_children[index]);
			child->m_parent = nullptr;
			m_children.pop_back();
			break;
		}
	}

	CalcBoundsAndChildBounds();
}

Vector2 UIElement::GetSize() const
{
	if(!m_parent)
		return m_size.unit;

	return m_size.ratio * m_parent->GetSize() + m_size.unit;
}

AABB2D UIElement::GetBounds() const
{
	Vector2 size = GetSize();
	Vector2 pivot_position = size * m_pivot;

	auto bounds = AABB2D(Vector2(0.0f, 0.0f), size);
	bounds.Translate(-1.0f * pivot_position);
	return bounds;
}

void UIElement::DebugRender() const
{
	if (!g_simpleRenderer || !m_isDebug)
		return;

	AABB2D local_bounds = GetBounds();
	Vector2 center_pos = local_bounds.CalcCenter();
	Vector2 extends = local_bounds.GetExtends();
	Rgba color = m_debug;
	g_simpleRenderer->Draw2DDebugQuad(center_pos, extends, color, color, *g_simpleRenderer->m_whiteTexture, 2.0f);
	g_simpleRenderer->DrawPoint(Vector2(0.0f, 0.0f), Rgba(255, 255, 255, 255), 25.0f);
}

void UIElement::RemoveSelf()
{
	if(m_parent)
		m_parent->RemoveChild(this);
}

void UIElement::Filicide()
{
	for (UIElement* child : m_children)
	{
		delete child;
	}

	m_children.clear();
}

void UIElement::SetDebugColor(const Rgba& new_color)
{
	m_debug = new_color;
}

UIElement* UIElement::GetParent() const
{
	return m_parent;
}

AABB2D UIElement::GetRelativeBounds()
{
	Vector2 parent_size;
	
	if (m_parent)
		parent_size = m_parent->GetSize();
	else
		return GetBounds();

	// Absolute Positioning
	Vector2 relative_size = parent_size * m_size.ratio;
	Vector2 my_size = relative_size + m_size.unit;

	Vector2 relative_position = parent_size * m_position.ratio;
	Vector2 my_position = relative_position + m_position.unit;

	AABB2D bounds = AABB2D(Vector2(0.0f, 0.0f), my_size);
	bounds.Translate(my_position - m_pivot * my_size);

	return bounds;
}

Vector2 UIElement::GetRelativePosition()
{
	AABB2D relative_bounds;
	if (m_parent)
		relative_bounds = m_parent->GetRelativeBounds();
	else
		relative_bounds = AABB2D();

	return relative_bounds.GetUV(m_pivot);
}

Matrix4 UIElement::GetLocalTransform() const
{
	Matrix4 transform;
	Vector2 pos = GetLocalPosition();
	transform.Translate(pos);
	return transform;
}

Matrix4 UIElement::GetWorldTransform() const
{
	Matrix4 myMatrix = GetLocalTransform();
	Matrix4 world_transform = myMatrix;

	for (UIElement* current_parent = m_parent; current_parent != nullptr; current_parent = current_parent->m_parent)
	{
		Matrix4 parent_matrix = current_parent->GetWorldTransform();
		world_transform = MatrixMultiplicationRowMajorAB(world_transform, parent_matrix);
	}

	return world_transform;
}

void UIElement::CalcBoundsAndChildBounds()
{
	m_bounds = GetBounds();

	for (UIElement* child : m_children)
	{
		child->m_bounds = child->GetBounds();
	}
}

void UIElement::SetTint(const Rgba& color)
{
	m_tint = color;
}

Vector2 UIElement::GetLocalPosition() const
{
	if (m_parent) 
	{
		AABB2D parent_relative = m_parent->GetBounds();
		return parent_relative.GetUV(m_position.ratio) + m_position.unit;
	}
	else
		return m_position.unit;
}

void UIElement::SetShader(ShaderProgram* shader)
{
	m_shader = shader;
}

void UIElement::SetDebug(bool debug)
{
	m_isDebug = debug;
}

void UIElement::ToggleDebug()
{
	SetDebug(!m_isDebug);
}

