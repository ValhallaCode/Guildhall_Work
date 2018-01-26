#pragma once
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2D.hpp"
#include "Engine/Math/Matrix4.hpp"
#include "Engine/Render/Rgba.hpp"
#include <vector>


class Sampler;
class ShaderProgram;

struct ui_metric_t
{
	Vector2 ratio;
	Vector2 unit;
};

class UIElement
{
public:
	UIElement();
	virtual ~UIElement();
	void SetPosition(Vector2 ratio, Vector2 unit);
	void SetSize(Vector2 size);
	void SetSizeRatio(Vector2 size_ratio);
	void SetPivot(Vector2 ratio_in_respect_to_my_bounds);
	UIElement* AddChild(UIElement* child);
	void RemoveChild(UIElement* child);
	Vector2 GetSize() const;
	AABB2D GetBounds() const;
	void DebugRender() const;
	virtual void Render() = 0;
	void RemoveSelf();
	void Filicide();
	void SetDebugColor(const Rgba& new_color);
	UIElement* GetParent() const;
	AABB2D GetRelativeBounds();
	Vector2 GetRelativePosition();
	Matrix4 GetLocalTransform() const;
	Matrix4 GetWorldTransform() const;
	void CalcBoundsAndChildBounds();
	void SetTint(const Rgba& color);
	Vector2 GetLocalPosition() const;
	void SetShader(ShaderProgram* shader);
	void SetDebug(bool debug);
	void ToggleDebug();

	template <typename T>
	T* CreateChild()
	{
		return (T*)AddChild(new T());
	}

public:
	ui_metric_t m_position;
	Vector2 m_pivot; // ratio of my bounds
	ui_metric_t m_size;
	AABB2D m_bounds;
	UIElement* m_parent;
	std::vector<UIElement*> m_children;

protected:
	Sampler* m_sampler;
	ShaderProgram* m_shader;
	Rgba m_debug;
	Rgba m_tint;
	bool m_isDebug;
};